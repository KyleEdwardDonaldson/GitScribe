//! GitScribe Core - High-performance Git operations library
//!
//! This library provides fast, cached Git status queries for the GitScribe
//! shell extension and application.

pub mod repository;
pub mod status;
pub mod cache;
pub mod ffi;
pub mod oplog;
pub mod stash;
pub mod temp_ignore;
pub mod history;

// N-API bindings for Node.js (optional)
#[cfg(feature = "napi-bindings")]
pub mod napi;

// Re-export main types
pub use repository::{Repository, RepoState, RemoteStatus};
pub use status::FileStatusEntry;
// Export status FileStatus with a different name to avoid conflicts
pub use status::FileStatus as StatusFileStatus;
pub use cache::StatusCache;
pub use oplog::{OperationLog, Operation, OperationType};
pub use stash::{
    VisualStashManager, VisualStash, StashedFile,
    BranchContext, CommitInfo
};
// Export stash FileStatus separately with explicit alias
pub use stash::FileStatus as StashFileStatus;
pub use temp_ignore::{TempIgnoreManager, TemporaryIgnore, IncludeCondition, TempIgnoreSettings};
pub use history::{GitHistory, Commit, Branch, DiffStats, FileChange};

/// Library version
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_version() {
        assert_eq!(VERSION, "0.1.0");
    }
}
