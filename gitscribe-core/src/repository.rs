//! Git repository operations

use anyhow::{Context, Result};
use std::path::{Path, PathBuf};

/// Repository state (special operations in progress)
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(C)]
pub enum RepoState {
    Clean = 0,
    Merging = 1,
    Rebasing = 2,
    CherryPicking = 3,
    Reverting = 4,
    Bisecting = 5,
}

/// Remote sync status
#[derive(Debug, Clone)]
pub struct RemoteStatus {
    pub ahead: usize,
    pub behind: usize,
    pub remote_name: String,
    pub remote_branch: String,
}

/// Represents an open Git repository
pub struct Repository {
    path: PathBuf,
    inner: git2::Repository,
}

impl Repository {
    /// Open a Git repository at the given path
    ///
    /// # Arguments
    /// * `path` - Path to the repository (can be inside the repo, will search upwards)
    ///
    /// # Returns
    /// * `Ok(Repository)` if repository found
    /// * `Err` if not a Git repository or other error
    pub fn open<P: AsRef<Path>>(path: P) -> Result<Self> {
        let path = path.as_ref();

        let inner = git2::Repository::discover(path)
            .context("Failed to open Git repository")?;

        let repo_path = inner.path().parent()
            .unwrap_or(inner.path())
            .to_path_buf();

        Ok(Repository {
            path: repo_path,
            inner,
        })
    }

    /// Get the repository root path
    pub fn path(&self) -> &Path {
        &self.path
    }

    /// Get reference to underlying git2::Repository
    pub(crate) fn inner(&self) -> &git2::Repository {
        &self.inner
    }

    /// Get current repository state (clean, merging, rebasing, etc.)
    pub fn state(&self) -> RepoState {
        match self.inner.state() {
            git2::RepositoryState::Merge => RepoState::Merging,
            git2::RepositoryState::Rebase
            | git2::RepositoryState::RebaseInteractive
            | git2::RepositoryState::RebaseMerge => RepoState::Rebasing,
            git2::RepositoryState::CherryPick
            | git2::RepositoryState::CherryPickSequence => RepoState::CherryPicking,
            git2::RepositoryState::Revert
            | git2::RepositoryState::RevertSequence => RepoState::Reverting,
            git2::RepositoryState::Bisect => RepoState::Bisecting,
            _ => RepoState::Clean,
        }
    }

    /// Get the current branch name
    pub fn current_branch(&self) -> Result<String> {
        let head = self.inner.head()?;

        if head.is_branch() {
            let branch_name = head.shorthand().unwrap_or("HEAD");
            Ok(branch_name.to_string())
        } else {
            Ok("HEAD".to_string()) // Detached HEAD
        }
    }

    /// Get remote status (commits ahead/behind)
    pub fn remote_status(&self) -> Result<Option<RemoteStatus>> {
        let head = self.inner.head()?;

        if !head.is_branch() {
            return Ok(None); // Detached HEAD
        }

        let local_oid = head.target().context("No local OID")?;

        let branch = git2::Branch::wrap(head);
        let upstream = match branch.upstream() {
            Ok(u) => u,
            Err(_) => return Ok(None), // No upstream configured
        };

        let upstream_oid = upstream.get().target().context("No upstream OID")?;

        let (ahead, behind) = self.inner.graph_ahead_behind(local_oid, upstream_oid)?;

        let upstream_name = upstream.name()?.unwrap_or("origin");
        let parts: Vec<&str> = upstream_name.split('/').collect();
        let remote_name = parts.get(0).unwrap_or(&"origin").to_string();
        let remote_branch = parts.get(1).unwrap_or(&"main").to_string();

        Ok(Some(RemoteStatus {
            ahead,
            behind,
            remote_name,
            remote_branch,
        }))
    }

    /// Count modified files in working tree
    pub fn count_modified(&self) -> Result<usize> {
        let statuses = self.status()?;
        Ok(statuses.iter().filter(|e|
            matches!(e.status, crate::status::FileStatus::Modified |
                               crate::status::FileStatus::Added |
                               crate::status::FileStatus::Deleted)
        ).count())
    }

    /// Count conflicted files
    pub fn count_conflicted(&self) -> Result<usize> {
        let statuses = self.status()?;
        Ok(statuses.iter().filter(|e|
            e.status == crate::status::FileStatus::Conflicted
        ).count())
    }

    /// Check if working tree is clean
    pub fn is_clean(&self) -> Result<bool> {
        Ok(self.count_modified()? == 0)
    }

    /// Stage files for commit
    ///
    /// # Arguments
    /// * `paths` - File paths to stage (relative to repo root)
    pub fn stage<I, S>(&self, paths: I) -> Result<()>
    where
        I: IntoIterator<Item = S>,
        S: AsRef<Path>,
    {
        let mut index = self.inner.index()?;

        for path in paths {
            index.add_path(path.as_ref())
                .context(format!("Failed to stage file: {:?}", path.as_ref()))?;
        }

        index.write()?;
        Ok(())
    }

    /// Unstage files
    ///
    /// # Arguments
    /// * `paths` - File paths to unstage (relative to repo root)
    pub fn unstage<I, S>(&self, paths: I) -> Result<()>
    where
        I: IntoIterator<Item = S>,
        S: AsRef<Path>,
    {
        let head_commit = self.inner.head()?.peel_to_commit()?;
        let head_object = head_commit.as_object();

        for path in paths {
            let path_ref = path.as_ref();
            // Reset index entry to match HEAD
            self.inner.reset_default(Some(head_object), [path_ref].iter())?;
        }

        Ok(())
    }

    /// Create a commit
    ///
    /// # Arguments
    /// * `message` - Commit message
    ///
    /// # Returns
    /// * Commit OID as string
    pub fn commit(&self, message: &str) -> Result<String> {
        let mut index = self.inner.index()?;
        let tree_id = index.write_tree()?;
        let tree = self.inner.find_tree(tree_id)?;

        let signature = self.inner.signature()
            .or_else(|_| {
                // Fallback to default signature if git config not set
                git2::Signature::now("GitScribe User", "gitscribe@localhost")
            })?;

        // Get current HEAD as parent
        let parent_commit = match self.inner.head() {
            Ok(head) => {
                let oid = head.target().context("HEAD has no target")?;
                Some(self.inner.find_commit(oid)?)
            }
            Err(_) => None, // First commit (no parent)
        };

        let parents: Vec<&git2::Commit> = if let Some(ref p) = parent_commit {
            vec![p]
        } else {
            vec![]
        };

        let commit_oid = self.inner.commit(
            Some("HEAD"),
            &signature,
            &signature,
            message,
            &tree,
            &parents,
        )?;

        Ok(commit_oid.to_string())
    }

    /// Push to remote
    ///
    /// # Arguments
    /// * `remote_name` - Name of the remote (e.g., "origin")
    /// * `refspec` - Optional refspec (e.g., "refs/heads/main:refs/heads/main")
    ///               If None, pushes current branch to upstream
    pub fn push(&self, remote_name: &str, refspec: Option<&str>) -> Result<()> {
        let mut remote = self.inner.find_remote(remote_name)?;

        let refspec = if let Some(spec) = refspec {
            spec.to_string()
        } else {
            // Use current branch
            let branch = self.current_branch()?;
            format!("refs/heads/{0}:refs/heads/{0}", branch)
        };

        let mut push_options = git2::PushOptions::new();

        // Set up callbacks for credentials
        let mut callbacks = git2::RemoteCallbacks::new();
        callbacks.credentials(|_url, username_from_url, _allowed_types| {
            git2::Cred::ssh_key_from_agent(username_from_url.unwrap_or("git"))
        });
        push_options.remote_callbacks(callbacks);

        remote.push(&[refspec.as_str()], Some(&mut push_options))?;
        Ok(())
    }

    /// Pull from remote (fetch + merge)
    ///
    /// # Arguments
    /// * `remote_name` - Name of the remote (e.g., "origin")
    pub fn pull(&self, remote_name: &str) -> Result<()> {
        // First, fetch
        let mut remote = self.inner.find_remote(remote_name)?;

        let mut fetch_options = git2::FetchOptions::new();
        let mut callbacks = git2::RemoteCallbacks::new();
        callbacks.credentials(|_url, username_from_url, _allowed_types| {
            git2::Cred::ssh_key_from_agent(username_from_url.unwrap_or("git"))
        });
        fetch_options.remote_callbacks(callbacks);

        remote.fetch(&[self.current_branch()?], Some(&mut fetch_options), None)?;

        // Then merge
        let fetch_head = self.inner.find_reference("FETCH_HEAD")?;
        let fetch_commit = self.inner.reference_to_annotated_commit(&fetch_head)?;

        // Perform merge analysis
        let analysis = self.inner.merge_analysis(&[&fetch_commit])?;

        if analysis.0.is_up_to_date() {
            return Ok(()); // Already up to date
        }

        if analysis.0.is_fast_forward() {
            // Fast-forward merge
            let refname = format!("refs/heads/{}", self.current_branch()?);
            let mut reference = self.inner.find_reference(&refname)?;
            reference.set_target(fetch_commit.id(), "Fast-forward merge")?;
            self.inner.set_head(&refname)?;
            self.inner.checkout_head(Some(git2::build::CheckoutBuilder::default().force()))?;
        } else {
            // Normal merge (might have conflicts)
            self.inner.merge(&[&fetch_commit], None, None)?;

            // Check for conflicts
            let mut index = self.inner.index()?;
            if index.has_conflicts() {
                return Err(anyhow::anyhow!("Merge conflicts detected"));
            }

            // Create merge commit
            let signature = self.inner.signature()?;
            let tree_id = index.write_tree()?;
            let tree = self.inner.find_tree(tree_id)?;

            let head_commit = self.inner.head()?.peel_to_commit()?;
            let fetch_commit_obj = self.inner.find_commit(fetch_commit.id())?;

            self.inner.commit(
                Some("HEAD"),
                &signature,
                &signature,
                &format!("Merge branch '{}' of {}", self.current_branch()?, remote_name),
                &tree,
                &[&head_commit, &fetch_commit_obj],
            )?;

            self.inner.cleanup_state()?;
        }

        Ok(())
    }

    /// Fetch from remote
    ///
    /// # Arguments
    /// * `remote_name` - Name of the remote (e.g., "origin")
    pub fn fetch(&self, remote_name: &str) -> Result<()> {
        let mut remote = self.inner.find_remote(remote_name)?;

        let mut fetch_options = git2::FetchOptions::new();
        let mut callbacks = git2::RemoteCallbacks::new();
        callbacks.credentials(|_url, username_from_url, _allowed_types| {
            git2::Cred::ssh_key_from_agent(username_from_url.unwrap_or("git"))
        });
        fetch_options.remote_callbacks(callbacks);

        remote.fetch(&[self.current_branch()?], Some(&mut fetch_options), None)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;
    use std::fs;

    #[test]
    fn test_open_repository() {
        // Create a temporary Git repository
        let temp_dir = TempDir::new().unwrap();
        let repo_path = temp_dir.path();

        // Initialize Git repo
        git2::Repository::init(repo_path).unwrap();

        // Open it with our wrapper
        let repo = Repository::open(repo_path).unwrap();
        assert_eq!(repo.path(), repo_path);
    }

    #[test]
    fn test_open_non_repository() {
        let temp_dir = TempDir::new().unwrap();
        let result = Repository::open(temp_dir.path());
        assert!(result.is_err());
    }

    #[test]
    fn test_discover_from_subdirectory() {
        // Create repo with subdirectory
        let temp_dir = TempDir::new().unwrap();
        let repo_path = temp_dir.path();
        git2::Repository::init(repo_path).unwrap();

        let subdir = repo_path.join("subdir");
        fs::create_dir(&subdir).unwrap();

        // Should find repo from subdirectory
        let repo = Repository::open(&subdir).unwrap();
        assert_eq!(repo.path(), repo_path);
    }
}
