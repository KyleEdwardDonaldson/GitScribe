//! Operation Log System
//!
//! Tracks all Git operations for undo/redo functionality
//! Inspired by GitButler's operation timeline

use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use std::time::{SystemTime, UNIX_EPOCH};

/// Type of Git operation
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum OperationType {
    Commit { message: String, hash: String },
    Stage { files: Vec<String> },
    Unstage { files: Vec<String> },
    Discard { files: Vec<String> },
    CreateBranch { name: String },
    SwitchBranch { from: String, to: String },
    Merge { branch: String },
    Pull { remote: String, branch: String },
    Push { remote: String, branch: String },
    Stash { message: Option<String> },
    StashPop { index: usize },
    Rebase { onto: String },
    CherryPick { commit: String },
    Revert { commit: String },
    Reset { mode: ResetMode, target: String },
    AmendCommit { message: String },
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ResetMode {
    Soft,
    Mixed,
    Hard,
}

/// Snapshot of repository state before operation
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RepositorySnapshot {
    pub head: String,
    pub branch: String,
    pub staged_files: Vec<String>,
    pub modified_files: Vec<String>,
    pub untracked_files: Vec<String>,
    pub stashes: Vec<String>,
}

/// A single operation in the log
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Operation {
    pub id: String,
    pub timestamp: u64,
    pub operation_type: OperationType,
    pub snapshot_before: RepositorySnapshot,
    pub snapshot_after: Option<RepositorySnapshot>,
    pub success: bool,
    pub error_message: Option<String>,
}

impl Operation {
    pub fn new(operation_type: OperationType, snapshot_before: RepositorySnapshot) -> Self {
        let timestamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();

        Self {
            id: uuid::Uuid::new_v4().to_string(),
            timestamp,
            operation_type,
            snapshot_before,
            snapshot_after: None,
            success: true,
            error_message: None,
        }
    }

    pub fn with_result(
        mut self,
        snapshot_after: RepositorySnapshot,
        success: bool,
        error: Option<String>,
    ) -> Self {
        self.snapshot_after = Some(snapshot_after);
        self.success = success;
        self.error_message = error;
        self
    }
}

/// Operation log for tracking and undo/redo
#[derive(Debug)]
pub struct OperationLog {
    operations: Vec<Operation>,
    current_index: Option<usize>,
    repo_path: PathBuf,
    max_operations: usize,
}

impl OperationLog {
    pub fn new(repo_path: PathBuf) -> Self {
        Self {
            operations: Vec::new(),
            current_index: None,
            repo_path,
            max_operations: 1000, // Keep last 1000 operations
        }
    }

    /// Add a new operation to the log
    pub fn push(&mut self, operation: Operation) {
        // If we're not at the end of the log, truncate future operations
        if let Some(index) = self.current_index {
            self.operations.truncate(index + 1);
        }

        self.operations.push(operation);
        self.current_index = Some(self.operations.len() - 1);

        // Enforce max operations limit
        if self.operations.len() > self.max_operations {
            let to_remove = self.operations.len() - self.max_operations;
            self.operations.drain(0..to_remove);
            if let Some(ref mut index) = self.current_index {
                *index = index.saturating_sub(to_remove);
            }
        }
    }

    /// Get the current operation
    pub fn current(&self) -> Option<&Operation> {
        self.current_index
            .and_then(|idx| self.operations.get(idx))
    }

    /// Can we undo?
    pub fn can_undo(&self) -> bool {
        self.current_index.is_some() && self.current_index.unwrap() > 0
    }

    /// Can we redo?
    pub fn can_redo(&self) -> bool {
        if let Some(index) = self.current_index {
            index < self.operations.len() - 1
        } else {
            !self.operations.is_empty()
        }
    }

    /// Undo the last operation
    pub fn undo(&mut self) -> Option<&Operation> {
        if self.can_undo() {
            if let Some(ref mut index) = self.current_index {
                *index -= 1;
                return self.operations.get(*index);
            }
        }
        None
    }

    /// Redo the next operation
    pub fn redo(&mut self) -> Option<&Operation> {
        if self.can_redo() {
            match self.current_index {
                Some(ref mut index) => {
                    *index += 1;
                    self.operations.get(*index)
                }
                None => {
                    self.current_index = Some(0);
                    self.operations.get(0)
                }
            }
        } else {
            None
        }
    }

    /// Get all operations
    pub fn all_operations(&self) -> &[Operation] {
        &self.operations
    }

    /// Get operations in reverse chronological order
    pub fn recent_operations(&self, count: usize) -> Vec<&Operation> {
        self.operations
            .iter()
            .rev()
            .take(count)
            .collect()
    }

    /// Search operations by type
    pub fn find_by_type(&self, operation_type: &str) -> Vec<&Operation> {
        self.operations
            .iter()
            .filter(|op| {
                match &op.operation_type {
                    OperationType::Commit { .. } => operation_type == "commit",
                    OperationType::Stage { .. } => operation_type == "stage",
                    OperationType::Push { .. } => operation_type == "push",
                    OperationType::Pull { .. } => operation_type == "pull",
                    _ => false,
                }
            })
            .collect()
    }

    /// Get operations within a time range
    pub fn operations_in_range(&self, start: u64, end: u64) -> Vec<&Operation> {
        self.operations
            .iter()
            .filter(|op| op.timestamp >= start && op.timestamp <= end)
            .collect()
    }

    /// Clear the entire log
    pub fn clear(&mut self) {
        self.operations.clear();
        self.current_index = None;
    }

    /// Export log to JSON
    pub fn to_json(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string_pretty(&self.operations)
    }

    /// Import log from JSON
    pub fn from_json(json: &str) -> Result<Vec<Operation>, serde_json::Error> {
        serde_json::from_str(json)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn create_test_snapshot() -> RepositorySnapshot {
        RepositorySnapshot {
            head: "abc123".to_string(),
            branch: "main".to_string(),
            staged_files: vec![],
            modified_files: vec![],
            untracked_files: vec![],
            stashes: vec![],
        }
    }

    #[test]
    fn test_push_and_current() {
        let mut log = OperationLog::new(PathBuf::from("/test"));
        let op = Operation::new(
            OperationType::Commit {
                message: "Test".to_string(),
                hash: "abc".to_string(),
            },
            create_test_snapshot(),
        );

        log.push(op.clone());
        assert!(log.current().is_some());
        assert_eq!(log.current().unwrap().id, op.id);
    }

    #[test]
    fn test_undo_redo() {
        let mut log = OperationLog::new(PathBuf::from("/test"));

        let op1 = Operation::new(
            OperationType::Commit {
                message: "First".to_string(),
                hash: "abc".to_string(),
            },
            create_test_snapshot(),
        );
        let op2 = Operation::new(
            OperationType::Commit {
                message: "Second".to_string(),
                hash: "def".to_string(),
            },
            create_test_snapshot(),
        );

        log.push(op1.clone());
        log.push(op2.clone());

        assert_eq!(log.current().unwrap().id, op2.id);

        log.undo();
        assert_eq!(log.current().unwrap().id, op1.id);

        log.redo();
        assert_eq!(log.current().unwrap().id, op2.id);
    }

    #[test]
    fn test_max_operations() {
        let mut log = OperationLog::new(PathBuf::from("/test"));
        log.max_operations = 3;

        for i in 0..5 {
            let op = Operation::new(
                OperationType::Commit {
                    message: format!("Commit {}", i),
                    hash: format!("{}", i),
                },
                create_test_snapshot(),
            );
            log.push(op);
        }

        assert_eq!(log.operations.len(), 3);
    }
}
