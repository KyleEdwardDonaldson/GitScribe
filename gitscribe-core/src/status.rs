//! File status queries

use anyhow::Result;
use serde::{Deserialize, Serialize};
use std::path::PathBuf;

use crate::Repository;

/// File status in Git
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[repr(C)]
pub enum FileStatus {
    /// File is unmodified
    Clean = 0,
    /// File has been modified in working tree
    Modified = 1,
    /// File is newly added to index
    Added = 2,
    /// File has been deleted
    Deleted = 3,
    /// File is ignored by .gitignore
    Ignored = 4,
    /// File has conflicts
    Conflicted = 5,
    /// File is not tracked by Git
    Untracked = 6,
    /// Repository is locked (operation in progress)
    Locked = 7,
}

/// A file with its Git status
#[derive(Debug, Clone)]
pub struct FileStatusEntry {
    pub path: PathBuf,
    pub status: FileStatus,
}

impl Repository {
    /// Get status of all files in the repository
    ///
    /// Note: This is relatively expensive for large repos.
    /// Consider using `status_cached()` with a StatusCache instead.
    pub fn status(&self) -> Result<Vec<FileStatusEntry>> {
        let mut entries = Vec::new();
        let mut opts = git2::StatusOptions::new();
        opts.include_untracked(true)
            .include_ignored(false)  // Don't show ignored files by default
            .recurse_untracked_dirs(true);

        let statuses = self.inner().statuses(Some(&mut opts))?;

        for entry in statuses.iter() {
            let path = match entry.path() {
                Some(p) => PathBuf::from(p),
                None => continue, // Skip entries with invalid UTF-8 paths
            };

            let status = Self::convert_status(entry.status());

            entries.push(FileStatusEntry { path, status });
        }

        Ok(entries)
    }

    /// Get status of a specific file or directory
    pub fn file_status<P: AsRef<std::path::Path>>(&self, path: P) -> Result<FileStatus> {
        let file_path = path.as_ref();

        // Convert to path relative to repository root
        // status_file expects a relative path, not absolute
        let relative_path = if file_path.is_absolute() {
            file_path.strip_prefix(self.path()).ok()
        } else {
            Some(file_path)
        };

        let rel_path = match relative_path {
            Some(p) => p,
            None => return Ok(FileStatus::Clean), // Not in this repo
        };

        // Check if this is a directory
        if file_path.is_dir() {
            // For directories, check if any files inside have changes
            return self.directory_status(rel_path);
        }

        let status = self.inner().status_file(rel_path)?;
        Ok(Self::convert_status(status))
    }

    /// Get status of a directory (checks if it contains any changes)
    fn directory_status<P: AsRef<std::path::Path>>(&self, rel_path: P) -> Result<FileStatus> {
        let dir_path = rel_path.as_ref();

        // Get all statuses
        let all_statuses = self.status()?;

        // Check if any file in this directory has changes
        for entry in all_statuses {
            if let Ok(_entry_path) = entry.path.strip_prefix(dir_path) {
                // File is in this directory
                if entry.status != FileStatus::Clean && entry.status != FileStatus::Ignored {
                    return Ok(FileStatus::Modified); // Directory contains changes
                }
            }
        }

        Ok(FileStatus::Clean)
    }

    fn convert_status(status: git2::Status) -> FileStatus {
        // Check in priority order
        if status.is_conflicted() {
            FileStatus::Conflicted
        } else if status.is_wt_modified() || status.is_index_modified() {
            FileStatus::Modified
        } else if status.is_wt_new() || status.is_index_new() {
            if status.is_index_new() {
                FileStatus::Added
            } else {
                FileStatus::Untracked
            }
        } else if status.is_wt_deleted() || status.is_index_deleted() {
            FileStatus::Deleted
        } else if status.is_ignored() {
            FileStatus::Ignored
        } else {
            FileStatus::Clean
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;
    use std::fs;

    #[test]
    fn test_empty_repository() {
        let temp_dir = TempDir::new().unwrap();
        git2::Repository::init(temp_dir.path()).unwrap();

        let repo = Repository::open(temp_dir.path()).unwrap();
        let status = repo.status().unwrap();

        // Empty repo should have no files
        assert_eq!(status.len(), 0);
    }

    #[test]
    fn test_untracked_file() {
        let temp_dir = TempDir::new().unwrap();
        let repo_path = temp_dir.path();
        git2::Repository::init(repo_path).unwrap();

        // Create a file
        let file_path = repo_path.join("test.txt");
        fs::write(&file_path, "hello").unwrap();

        let repo = Repository::open(repo_path).unwrap();
        let status = repo.status().unwrap();

        assert_eq!(status.len(), 1);
        assert_eq!(status[0].status, FileStatus::Untracked);
    }

    #[test]
    fn test_modified_file() {
        let temp_dir = TempDir::new().unwrap();
        let repo_path = temp_dir.path();
        let git_repo = git2::Repository::init(repo_path).unwrap();

        // Create and commit a file
        let file_path = repo_path.join("test.txt");
        fs::write(&file_path, "initial").unwrap();

        let mut index = git_repo.index().unwrap();
        index.add_path(std::path::Path::new("test.txt")).unwrap();
        index.write().unwrap();

        let tree_id = index.write_tree().unwrap();
        let tree = git_repo.find_tree(tree_id).unwrap();
        let sig = git2::Signature::now("Test", "test@example.com").unwrap();
        git_repo.commit(Some("HEAD"), &sig, &sig, "Initial commit", &tree, &[]).unwrap();

        // Modify the file
        fs::write(&file_path, "modified").unwrap();

        let repo = Repository::open(repo_path).unwrap();
        let status = repo.status().unwrap();

        assert_eq!(status.len(), 1);
        assert_eq!(status[0].status, FileStatus::Modified);
    }
}
