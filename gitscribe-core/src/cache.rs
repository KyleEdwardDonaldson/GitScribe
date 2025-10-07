//! SQLite-based status caching for fast overlay icon queries

use anyhow::{Context, Result};
use rusqlite::{Connection, params};
use std::path::Path;
use std::time::{SystemTime, UNIX_EPOCH};
use serde::{Deserialize, Serialize};

use crate::status::FileStatus;
use crate::{Repository, FileStatusEntry};

/// Shared SQLite cache version - coordinate changes across all components
const CACHE_VERSION: &str = "1.0.0";

/// Operation status in queue
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum OperationStatus {
    Pending = 0,
    Running = 1,
    Complete = 2,
    Failed = 3,
}

/// Cache for file status queries
///
/// Uses SQLite to store file status and avoid repeated Git queries.
/// Implements shared schema v1.0.0 for cross-component compatibility.
pub struct StatusCache {
    conn: Connection,
    ttl_seconds: i64,
}

impl StatusCache {
    /// Create or open a status cache database with shared schema v1.0.0
    pub fn new<P: AsRef<Path>>(path: P) -> Result<Self> {
        let conn = Connection::open(path)
            .context("Failed to open cache database")?;

        // Enable WAL mode for better concurrency
        conn.execute("PRAGMA journal_mode=WAL", [])?;
        conn.execute("PRAGMA synchronous=NORMAL", [])?;

        Self::create_schema(&conn)?;

        Ok(StatusCache {
            conn,
            ttl_seconds: 1, // 1 second default for UI responsiveness
        })
    }

    /// Create the complete shared schema (version 1.0.0)
    fn create_schema(conn: &Connection) -> Result<()> {
        // Repositories table
        conn.execute(
            "CREATE TABLE IF NOT EXISTS repositories (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                path TEXT UNIQUE NOT NULL,
                last_accessed INTEGER NOT NULL,
                is_valid INTEGER DEFAULT 1
            )",
            [],
        )?;

        // File status table (shared schema)
        conn.execute(
            "CREATE TABLE IF NOT EXISTS file_status (
                repo_id INTEGER NOT NULL,
                file_path TEXT NOT NULL,
                work_tree_status INTEGER NOT NULL,
                index_status INTEGER NOT NULL,
                cache_time INTEGER NOT NULL,
                file_mtime INTEGER NOT NULL,
                PRIMARY KEY (repo_id, file_path),
                FOREIGN KEY (repo_id) REFERENCES repositories(id) ON DELETE CASCADE
            )",
            [],
        )?;

        // Repository status table
        conn.execute(
            "CREATE TABLE IF NOT EXISTS repo_status (
                repo_id INTEGER PRIMARY KEY,
                current_branch TEXT,
                upstream_branch TEXT,
                ahead_count INTEGER DEFAULT 0,
                behind_count INTEGER DEFAULT 0,
                merge_head TEXT,
                rebase_head TEXT,
                cache_time INTEGER NOT NULL,
                FOREIGN KEY (repo_id) REFERENCES repositories(id) ON DELETE CASCADE
            )",
            [],
        )?;

        // Operation queue table
        conn.execute(
            "CREATE TABLE IF NOT EXISTS operation_queue (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                repo_id INTEGER NOT NULL,
                operation_type TEXT NOT NULL,
                operation_data TEXT NOT NULL,
                status INTEGER DEFAULT 0,
                created_at INTEGER NOT NULL,
                started_at INTEGER,
                completed_at INTEGER,
                error_message TEXT,
                FOREIGN KEY (repo_id) REFERENCES repositories(id) ON DELETE CASCADE
            )",
            [],
        )?;

        // Performance indexes
        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_file_status_cache ON file_status(cache_time)",
            [],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_repo_status_cache ON repo_status(cache_time)",
            [],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_operation_queue_status ON operation_queue(status, created_at)",
            [],
        )?;

        Ok(())
    }

    /// Get or create repository ID
    fn get_repo_id(&self, repo_path: &str) -> Result<i64> {
        let now = Self::now_timestamp();

        // Try to get existing
        let mut stmt = self.conn.prepare_cached(
            "SELECT id FROM repositories WHERE path = ?"
        )?;

        if let Ok(id) = stmt.query_row(params![repo_path], |row| row.get::<_, i64>(0)) {
            // Update last accessed
            self.conn.execute(
                "UPDATE repositories SET last_accessed = ? WHERE id = ?",
                params![now, id],
            )?;
            return Ok(id);
        }

        // Create new
        self.conn.execute(
            "INSERT INTO repositories (path, last_accessed) VALUES (?, ?)",
            params![repo_path, now],
        )?;

        Ok(self.conn.last_insert_rowid())
    }

    /// Get cached status from repository with TTL check
    pub fn get_cached_status(&self, repo: &Repository, ttl_ms: u64) -> Result<Vec<FileStatusEntry>> {
        let repo_path = repo.path().to_string_lossy().to_string();
        let repo_id = self.get_repo_id(&repo_path)?;

        let now = Self::now_timestamp();
        let ttl_seconds = (ttl_ms / 1000) as i64;
        let min_time = now - ttl_seconds;

        let mut stmt = self.conn.prepare_cached(
            "SELECT file_path, work_tree_status FROM file_status
             WHERE repo_id = ? AND cache_time > ?"
        )?;

        let mut rows = stmt.query(params![repo_id, min_time])?;
        let mut entries = Vec::new();

        while let Some(row) = rows.next()? {
            let path: String = row.get(0)?;
            let status: i32 = row.get(1)?;

            if let Some(file_status) = Self::int_to_status(status) {
                entries.push(FileStatusEntry {
                    path: path.into(),
                    status: file_status,
                });
            }
        }

        // If cache is empty or expired, get fresh status and cache it
        if entries.is_empty() {
            let fresh = repo.status()?;
            self.cache_status(repo_id, &fresh)?;
            return Ok(fresh);
        }

        Ok(entries)
    }

    /// Cache repository status
    fn cache_status(&self, repo_id: i64, entries: &[FileStatusEntry]) -> Result<()> {
        let now = Self::now_timestamp();

        for entry in entries {
            let path = entry.path.to_string_lossy();
            self.conn.execute(
                "INSERT OR REPLACE INTO file_status
                 (repo_id, file_path, work_tree_status, index_status, cache_time, file_mtime)
                 VALUES (?, ?, ?, ?, ?, ?)",
                params![repo_id, path.as_ref(), entry.status as i32, 0, now, now],
            )?;
        }

        Ok(())
    }

    /// Create an in-memory cache (for testing)
    pub fn in_memory() -> Result<Self> {
        Self::new(":memory:")
    }

    /// Queue an operation for background processing
    pub fn queue_operation(
        &self,
        repo_path: &str,
        operation_type: &str,
        operation_data: &str,
    ) -> Result<i64> {
        let repo_id = self.get_repo_id(repo_path)?;
        let now = Self::now_timestamp();

        self.conn.execute(
            "INSERT INTO operation_queue
             (repo_id, operation_type, operation_data, status, created_at)
             VALUES (?, ?, ?, ?, ?)",
            params![repo_id, operation_type, operation_data, OperationStatus::Pending as i32, now],
        )?;

        Ok(self.conn.last_insert_rowid())
    }

    /// Get pending operations for a repository
    pub fn get_pending_operations(&self, repo_path: &str) -> Result<Vec<(i64, String, String)>> {
        let repo_id = self.get_repo_id(repo_path)?;

        let mut stmt = self.conn.prepare_cached(
            "SELECT id, operation_type, operation_data FROM operation_queue
             WHERE repo_id = ? AND status = ?
             ORDER BY created_at ASC"
        )?;

        let mut rows = stmt.query(params![repo_id, OperationStatus::Pending as i32])?;
        let mut operations = Vec::new();

        while let Some(row) = rows.next()? {
            operations.push((
                row.get(0)?,
                row.get(1)?,
                row.get(2)?,
            ));
        }

        Ok(operations)
    }

    /// Update operation status
    pub fn update_operation_status(
        &self,
        operation_id: i64,
        status: OperationStatus,
        error_message: Option<&str>,
    ) -> Result<()> {
        let now = Self::now_timestamp();

        match status {
            OperationStatus::Running => {
                self.conn.execute(
                    "UPDATE operation_queue SET status = ?, started_at = ? WHERE id = ?",
                    params![status as i32, now, operation_id],
                )?;
            }
            OperationStatus::Complete | OperationStatus::Failed => {
                self.conn.execute(
                    "UPDATE operation_queue
                     SET status = ?, completed_at = ?, error_message = ?
                     WHERE id = ?",
                    params![status as i32, now, error_message, operation_id],
                )?;
            }
            _ => {}
        }

        Ok(())
    }

    /// Invalidate cache for specific repository
    pub fn invalidate_repo(&self, repo_path: &str) -> Result<()> {
        let repo_id = self.get_repo_id(repo_path)?;

        self.conn.execute(
            "DELETE FROM file_status WHERE repo_id = ?",
            params![repo_id],
        )?;

        self.conn.execute(
            "DELETE FROM repo_status WHERE repo_id = ?",
            params![repo_id],
        )?;

        Ok(())
    }

    /// Cleanup old operations from queue
    pub fn cleanup_operations(&self, max_age_seconds: i64) -> Result<usize> {
        let now = Self::now_timestamp();
        let min_time = now - max_age_seconds;

        let deleted = self.conn.execute(
            "DELETE FROM operation_queue
             WHERE (status = ? OR status = ?) AND completed_at < ?",
            params![OperationStatus::Complete as i32, OperationStatus::Failed as i32, min_time],
        )?;

        Ok(deleted)
    }

    fn now_timestamp() -> i64 {
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs() as i64
    }

    fn int_to_status(i: i32) -> Option<FileStatus> {
        match i {
            0 => Some(FileStatus::Clean),
            1 => Some(FileStatus::Modified),
            2 => Some(FileStatus::Added),
            3 => Some(FileStatus::Deleted),
            4 => Some(FileStatus::Ignored),
            5 => Some(FileStatus::Conflicted),
            6 => Some(FileStatus::Untracked),
            7 => Some(FileStatus::Locked),
            _ => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_operation_queue() {
        let cache = StatusCache::in_memory().unwrap();

        // Queue an operation
        let op_id = cache.queue_operation(
            "/test/repo",
            "commit",
            r#"{"message": "test commit"}"#
        ).unwrap();

        // Get pending operations
        let pending = cache.get_pending_operations("/test/repo").unwrap();
        assert_eq!(pending.len(), 1);
        assert_eq!(pending[0].0, op_id);
        assert_eq!(pending[0].1, "commit");

        // Mark as running
        cache.update_operation_status(op_id, OperationStatus::Running, None).unwrap();

        // Mark as complete
        cache.update_operation_status(op_id, OperationStatus::Complete, None).unwrap();

        // Should not appear in pending anymore
        let pending = cache.get_pending_operations("/test/repo").unwrap();
        assert_eq!(pending.len(), 0);
    }

    #[test]
    fn test_repo_invalidation() {
        let cache = StatusCache::in_memory().unwrap();

        // This will create repo entries
        cache.queue_operation("/test/repo", "test", "{}").unwrap();

        // Invalidate should clear cache
        cache.invalidate_repo("/test/repo").unwrap();

        // Verify operations still exist (invalidate only clears status caches)
        let pending = cache.get_pending_operations("/test/repo").unwrap();
        assert_eq!(pending.len(), 1);
    }

    #[test]
    fn test_cleanup_operations() {
        let cache = StatusCache::in_memory().unwrap();

        let op_id = cache.queue_operation("/test/repo", "test", "{}").unwrap();
        cache.update_operation_status(op_id, OperationStatus::Complete, None).unwrap();

        // Cleanup operations older than 0 seconds (all completed)
        let deleted = cache.cleanup_operations(0).unwrap();
        assert_eq!(deleted, 1);
    }
}
