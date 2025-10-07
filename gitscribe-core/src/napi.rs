//! N-API bindings for Node.js
//!
//! This module provides Node.js bindings for the GitScribe core library.
//! It exposes async functions that can be called from Electron/Node.js.

#![cfg(feature = "napi-bindings")]

use napi::bindgen_prelude::*;
use napi_derive::napi;

use crate::{Repository as CoreRepository, FileStatus as CoreFileStatus, FileStatusEntry, RepoState, StatusCache as CoreStatusCache};

/// File status information for JavaScript
#[napi(object)]
#[derive(Debug, Clone)]
pub struct FileStatusJS {
    pub path: String,
    /// 0=Clean, 1=Modified, 2=Added, 3=Deleted, 4=Ignored, 5=Conflicted, 6=Untracked, 7=Locked
    pub status: i32,
}

/// Repository status information
#[napi(object)]
#[derive(Debug, Clone)]
pub struct RepoStatusJS {
    pub files: Vec<FileStatusJS>,
    pub current_branch: String,
    pub ahead_count: i32,
    pub behind_count: i32,
    pub state: i32, // 0=Clean, 1=Merging, 2=Rebasing, etc.
    pub modified_count: i32,
    pub conflicted_count: i32,
}

/// Remote status information
#[napi(object)]
#[derive(Debug, Clone)]
pub struct RemoteStatusJS {
    pub ahead: i32,
    pub behind: i32,
    pub remote_name: String,
    pub remote_branch: String,
}

/// Repository handle for JavaScript
#[napi]
pub struct Repository {
    repo_path: String,
    cache_path: Option<String>,
}

impl From<CoreFileStatus> for i32 {
    fn from(status: CoreFileStatus) -> Self {
        match status {
            CoreFileStatus::Clean => 0,
            CoreFileStatus::Modified => 1,
            CoreFileStatus::Added => 2,
            CoreFileStatus::Deleted => 3,
            CoreFileStatus::Ignored => 4,
            CoreFileStatus::Conflicted => 5,
            CoreFileStatus::Untracked => 6,
            CoreFileStatus::Locked => 7,
        }
    }
}

impl From<FileStatusEntry> for FileStatusJS {
    fn from(entry: FileStatusEntry) -> Self {
        FileStatusJS {
            path: entry.path.to_string_lossy().to_string(),
            status: entry.status.into(),
        }
    }
}

impl From<RepoState> for i32 {
    fn from(state: RepoState) -> Self {
        match state {
            RepoState::Clean => 0,
            RepoState::Merging => 1,
            RepoState::Rebasing => 2,
            RepoState::CherryPicking => 3,
            RepoState::Reverting => 4,
            RepoState::Bisecting => 5,
        }
    }
}

#[napi]
impl Repository {
    /// Open a Git repository
    ///
    /// # Arguments
    /// * `path` - Path to the repository or any directory within it
    /// * `cache_path` - Optional path to SQLite cache database
    #[napi(constructor)]
    pub fn new(path: String, cache_path: Option<String>) -> Result<Self> {
        // Verify repository can be opened
        let repo = CoreRepository::open(&path)
            .map_err(|e| Error::from_reason(format!("Failed to open repository: {}", e)))?;

        let repo_path = repo.path().to_string_lossy().to_string();

        Ok(Repository {
            repo_path,
            cache_path,
        })
    }

    /// Get repository path
    #[napi]
    pub fn path(&self) -> String {
        self.repo_path.clone()
    }

    /// Get current branch name
    #[napi]
    pub async fn get_current_branch(&self) -> Result<String> {
        let repo_path = self.repo_path.clone();

        tokio::task::spawn_blocking(move || {
            let repo = CoreRepository::open(&repo_path)
                .map_err(|e| Error::from_reason(format!("Failed to open repository: {}", e)))?;

            repo.current_branch()
                .map_err(|e| Error::from_reason(format!("Failed to get current branch: {}", e)))
        })
        .await
        .map_err(|e| Error::from_reason(format!("Task failed: {}", e)))?
    }

    /// Get repository status with optional caching
    ///
    /// # Arguments
    /// * `ttl_ms` - Cache time-to-live in milliseconds (0 = no cache)
    #[napi]
    pub async fn get_status(&self, ttl_ms: Option<i32>) -> Result<RepoStatusJS> {
        let repo_path = self.repo_path.clone();
        let cache_path = self.cache_path.clone();
        let ttl = ttl_ms.unwrap_or(1000) as u64;

        tokio::task::spawn_blocking(move || {
            let repo = CoreRepository::open(&repo_path)
                .map_err(|e| Error::from_reason(format!("Failed to open repository: {}", e)))?;

            // Get file status
            let files = if let Some(cache_path) = cache_path {
                if ttl > 0 {
                    // Use cache
                    let cache = CoreStatusCache::new(&cache_path)
                        .map_err(|e| Error::from_reason(format!("Cache error: {}", e)))?;

                    cache.get_cached_status(&repo, ttl)
                        .map_err(|e| Error::from_reason(format!("Status error: {}", e)))?
                } else {
                    // No cache
                    repo.status()
                        .map_err(|e| Error::from_reason(format!("Status error: {}", e)))?
                }
            } else {
                repo.status()
                    .map_err(|e| Error::from_reason(format!("Status error: {}", e)))?
            };

            // Convert to JS types
            let files_js: Vec<FileStatusJS> = files.into_iter()
                .map(|f| f.into())
                .collect();

            // Get branch
            let current_branch = repo.current_branch()
                .unwrap_or_else(|_| "HEAD".to_string());

            // Get remote status
            let remote_status = repo.remote_status().ok().flatten();
            let (ahead_count, behind_count) = remote_status
                .map(|r| (r.ahead as i32, r.behind as i32))
                .unwrap_or((0, 0));

            // Get state
            let state: i32 = repo.state().into();

            // Get counts
            let modified_count = repo.count_modified().unwrap_or(0) as i32;
            let conflicted_count = repo.count_conflicted().unwrap_or(0) as i32;

            Ok(RepoStatusJS {
                files: files_js,
                current_branch,
                ahead_count,
                behind_count,
                state,
                modified_count,
                conflicted_count,
            })
        })
        .await
        .map_err(|e| Error::from_reason(format!("Task failed: {}", e)))?
    }

    /// Get remote status (commits ahead/behind)
    #[napi]
    pub async fn get_remote_status(&self) -> Result<Option<RemoteStatusJS>> {
        let repo_path = self.repo_path.clone();

        tokio::task::spawn_blocking(move || {
            let repo = CoreRepository::open(&repo_path)
                .map_err(|e| Error::from_reason(format!("Failed to open repository: {}", e)))?;

            match repo.remote_status() {
                Ok(Some(status)) => Ok(Some(RemoteStatusJS {
                    ahead: status.ahead as i32,
                    behind: status.behind as i32,
                    remote_name: status.remote_name,
                    remote_branch: status.remote_branch,
                })),
                Ok(None) => Ok(None),
                Err(e) => Err(Error::from_reason(format!("Failed to get remote status: {}", e))),
            }
        })
        .await
        .map_err(|e| Error::from_reason(format!("Task failed: {}", e)))?
    }

    /// Check if working tree is clean
    #[napi]
    pub async fn is_clean(&self) -> Result<bool> {
        let repo_path = self.repo_path.clone();

        tokio::task::spawn_blocking(move || {
            let repo = CoreRepository::open(&repo_path)
                .map_err(|e| Error::from_reason(format!("Failed to open repository: {}", e)))?;

            repo.is_clean()
                .map_err(|e| Error::from_reason(format!("Failed to check if clean: {}", e)))
        })
        .await
        .map_err(|e| Error::from_reason(format!("Task failed: {}", e)))?
    }
}

/// Initialize the N-API module
#[napi]
pub fn init_gitscribe() -> String {
    format!("GitScribe Core v{}", crate::VERSION)
}
