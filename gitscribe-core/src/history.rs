//! Git History Operations
//!
//! Provides commit log, branch listing, and search functionality using git2.

use anyhow::{Result, Context};
use git2::{Repository as Git2Repo, BranchType, Oid, DiffOptions};
use serde::{Deserialize, Serialize};
use std::path::Path;

/// A single commit in the history
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Commit {
    pub oid: String,
    pub short_oid: String,
    pub message: String,
    pub summary: String,
    pub author_name: String,
    pub author_email: String,
    pub committer_name: String,
    pub committer_email: String,
    pub timestamp: i64,
    pub parent_oids: Vec<String>,
}

/// A Git branch
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Branch {
    pub name: String,
    pub is_head: bool,
    pub is_remote: bool,
    pub upstream: Option<String>,
    pub target_oid: Option<String>,
}

/// Diff statistics for a commit
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DiffStats {
    pub files_changed: usize,
    pub insertions: usize,
    pub deletions: usize,
}

/// A file change in a commit
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FileChange {
    pub path: String,
    pub old_path: Option<String>,
    pub status: String, // "added", "modified", "deleted", "renamed"
    pub insertions: usize,
    pub deletions: usize,
}

/// Manages Git history operations
pub struct GitHistory {
    repo: Git2Repo,
}

impl GitHistory {
    /// Open a repository for history operations
    pub fn open(repo_path: &Path) -> Result<Self> {
        let repo = Git2Repo::open(repo_path)
            .context("Failed to open repository")?;

        Ok(Self { repo })
    }

    /// Get commit log with optional branch filter
    pub fn get_commits(&self, branch_name: Option<&str>, limit: usize, skip: usize) -> Result<Vec<Commit>> {
        let mut revwalk = self.repo.revwalk()?;

        // Set up the revwalk based on branch or HEAD
        if let Some(branch) = branch_name {
            let reference = self.repo.find_reference(&format!("refs/heads/{}", branch))?;
            revwalk.push(reference.target().context("Branch has no target")?)?;
        } else {
            revwalk.push_head()?;
        }

        revwalk.set_sorting(git2::Sort::TIME)?;

        let mut commits = Vec::new();

        for (i, oid_result) in revwalk.enumerate() {
            if i < skip {
                continue;
            }

            if commits.len() >= limit {
                break;
            }

            let oid = oid_result?;
            let commit = self.repo.find_commit(oid)?;

            commits.push(self.commit_to_struct(&commit)?);
        }

        Ok(commits)
    }

    /// Get all branches
    pub fn get_branches(&self) -> Result<Vec<Branch>> {
        let mut branches = Vec::new();

        // Get local branches
        for branch_result in self.repo.branches(Some(BranchType::Local))? {
            let (branch, _) = branch_result?;
            branches.push(self.branch_to_struct(&branch, false)?);
        }

        // Get remote branches
        for branch_result in self.repo.branches(Some(BranchType::Remote))? {
            let (branch, _) = branch_result?;
            branches.push(self.branch_to_struct(&branch, true)?);
        }

        Ok(branches)
    }

    /// Search commits by message, author, or hash
    pub fn search_commits(&self, query: &str, limit: usize) -> Result<Vec<Commit>> {
        let mut revwalk = self.repo.revwalk()?;
        revwalk.push_head()?;
        revwalk.set_sorting(git2::Sort::TIME)?;

        let query_lower = query.to_lowercase();
        let mut matches = Vec::new();

        for oid_result in revwalk {
            if matches.len() >= limit {
                break;
            }

            let oid = oid_result?;
            let commit = self.repo.find_commit(oid)?;

            // Check if query matches
            let oid_str = oid.to_string();
            let message = commit.message().unwrap_or("").to_string();
            let author_sig = commit.author();
            let author = author_sig.name().unwrap_or("").to_string();

            if oid_str.starts_with(&query_lower)
                || message.to_lowercase().contains(&query_lower)
                || author.to_lowercase().contains(&query_lower)
            {
                matches.push(self.commit_to_struct(&commit)?);
            }
        }

        Ok(matches)
    }

    /// Get diff for a specific commit
    pub fn get_commit_diff(&self, oid_str: &str) -> Result<(DiffStats, Vec<FileChange>)> {
        let oid = Oid::from_str(oid_str)?;
        let commit = self.repo.find_commit(oid)?;

        let tree = commit.tree()?;
        let parent_tree = if commit.parent_count() > 0 {
            Some(commit.parent(0)?.tree()?)
        } else {
            None
        };

        let mut diff_opts = DiffOptions::new();
        let diff = self.repo.diff_tree_to_tree(
            parent_tree.as_ref(),
            Some(&tree),
            Some(&mut diff_opts),
        )?;

        let stats = diff.stats()?;
        let diff_stats = DiffStats {
            files_changed: stats.files_changed(),
            insertions: stats.insertions(),
            deletions: stats.deletions(),
        };

        let mut file_changes = Vec::new();

        diff.foreach(
            &mut |delta, _progress| {
                let status = match delta.status() {
                    git2::Delta::Added => "added",
                    git2::Delta::Deleted => "deleted",
                    git2::Delta::Modified => "modified",
                    git2::Delta::Renamed => "renamed",
                    _ => "unknown",
                };

                let path = delta.new_file().path()
                    .and_then(|p| p.to_str())
                    .unwrap_or("")
                    .to_string();

                let old_path = if status == "renamed" {
                    delta.old_file().path()
                        .and_then(|p| p.to_str())
                        .map(|s| s.to_string())
                } else {
                    None
                };

                file_changes.push(FileChange {
                    path,
                    old_path,
                    status: status.to_string(),
                    insertions: 0, // Will be filled by line callback
                    deletions: 0,
                });

                true
            },
            None,
            None,
            None,
        )?;

        Ok((diff_stats, file_changes))
    }

    /// Get the current branch name
    pub fn get_current_branch(&self) -> Result<String> {
        let head = self.repo.head()?;

        if let Some(shorthand) = head.shorthand() {
            Ok(shorthand.to_string())
        } else {
            Ok("HEAD".to_string())
        }
    }

    /// Helper to convert git2::Commit to our Commit struct
    fn commit_to_struct(&self, commit: &git2::Commit) -> Result<Commit> {
        let oid = commit.id();
        let message = commit.message().unwrap_or("").to_string();
        let summary = commit.summary().unwrap_or("").to_string();

        let author = commit.author();
        let committer = commit.committer();

        let parent_oids: Vec<String> = commit.parent_ids()
            .map(|id| id.to_string())
            .collect();

        Ok(Commit {
            oid: oid.to_string(),
            short_oid: format!("{:.7}", oid),
            message,
            summary,
            author_name: author.name().unwrap_or("").to_string(),
            author_email: author.email().unwrap_or("").to_string(),
            committer_name: committer.name().unwrap_or("").to_string(),
            committer_email: committer.email().unwrap_or("").to_string(),
            timestamp: author.when().seconds(),
            parent_oids,
        })
    }

    /// Helper to convert git2::Branch to our Branch struct
    fn branch_to_struct(&self, branch: &git2::Branch, is_remote: bool) -> Result<Branch> {
        let name = branch.name()?
            .context("Branch name is not valid UTF-8")?
            .to_string();

        let is_head = branch.is_head();

        let upstream = if !is_remote {
            branch.upstream()
                .ok()
                .and_then(|u| {
                    u.name()
                        .ok()
                        .flatten()
                        .map(|s| s.to_string())
                })
        } else {
            None
        };

        let target_oid = branch.get().target().map(|oid| oid.to_string());

        Ok(Branch {
            name,
            is_head,
            is_remote,
            upstream,
            target_oid,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::env;

    #[test]
    fn test_get_commits() {
        // This test assumes you're running it in a git repository
        let current_dir = env::current_dir().unwrap();

        if let Ok(history) = GitHistory::open(&current_dir) {
            let commits = history.get_commits(None, 10, 0).unwrap();
            assert!(commits.len() > 0);

            // Check first commit has required fields
            let first = &commits[0];
            assert!(!first.oid.is_empty());
            assert!(!first.author_name.is_empty());
        }
    }
}
