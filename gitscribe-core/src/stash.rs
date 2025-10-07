//! Visual Stash Manager
//!
//! Provides drag-and-drop stash functionality with named stashes,
//! file-level granularity, and persistent storage.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};
use anyhow::{Result, Context};

/// A single file stashed with its content and metadata
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StashedFile {
    pub path: String,
    pub patch: String,              // Git diff patch
    pub original_content: String,   // Full file content at stash time
    pub status: FileStatus,
    pub size: u64,
    pub last_modified: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum FileStatus {
    Modified,
    New,
    Deleted,
}

/// A named stash containing multiple files
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct VisualStash {
    pub id: String,
    pub name: String,
    pub created: u64,
    pub modified: u64,
    pub files: Vec<StashedFile>,
    pub color: Option<String>,
    pub tags: Vec<String>,
    pub description: Option<String>,
    pub branch_context: Option<BranchContext>,
}

/// Branch context for branch-level stashes
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BranchContext {
    pub branch_name: String,
    pub base_commit: String,          // Base commit SHA
    pub head_commit: String,          // Current HEAD SHA
    pub ahead_count: u32,             // Commits ahead of upstream
    pub behind_count: u32,            // Commits behind upstream
    pub upstream_branch: Option<String>, // Tracking branch (e.g., "origin/main")
    pub unpushed_commits: Vec<CommitInfo>, // Commits not pushed
    pub staged_files: Vec<String>,    // Separately track staged vs unstaged
    pub unstaged_files: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CommitInfo {
    pub sha: String,
    pub message: String,
    pub author: String,
    pub timestamp: u64,
}

impl VisualStash {
    pub fn new(name: String) -> Self {
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();

        Self {
            id: uuid::Uuid::new_v4().to_string(),
            name,
            created: now,
            modified: now,
            files: Vec::new(),
            color: None,
            tags: Vec::new(),
            description: None,
            branch_context: None,
        }
    }

    pub fn new_branch_stash(name: String, branch_context: BranchContext) -> Self {
        let mut stash = Self::new(name);
        stash.branch_context = Some(branch_context);
        stash.tags.push("branch".to_string());
        stash
    }

    pub fn is_branch_stash(&self) -> bool {
        self.branch_context.is_some()
    }

    pub fn add_file(&mut self, file: StashedFile) {
        self.files.push(file);
        self.update_modified();
    }

    pub fn remove_file(&mut self, path: &str) -> Option<StashedFile> {
        if let Some(index) = self.files.iter().position(|f| f.path == path) {
            self.update_modified();
            Some(self.files.remove(index))
        } else {
            None
        }
    }

    fn update_modified(&mut self) {
        self.modified = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();
    }
}

/// Manages all visual stashes for a repository
#[derive(Debug)]
pub struct VisualStashManager {
    stashes: HashMap<String, VisualStash>,
    storage_path: PathBuf,
    repo_path: PathBuf,
}

impl VisualStashManager {
    /// Create a new stash manager for a repository
    pub fn new(repo_path: &Path) -> Result<Self> {
        let storage_path = repo_path.join(".git").join("gitscribe").join("stashes.json");

        // Create directory if it doesn't exist
        if let Some(parent) = storage_path.parent() {
            fs::create_dir_all(parent)?;
        }

        let mut manager = Self {
            stashes: HashMap::new(),
            storage_path,
            repo_path: repo_path.to_path_buf(),
        };

        // Load existing stashes
        manager.load()?;

        Ok(manager)
    }

    /// Create a new named stash
    pub fn create_stash(&mut self, name: String, color: Option<String>, tags: Vec<String>) -> String {
        let mut stash = VisualStash::new(name);
        stash.color = color;
        stash.tags = tags;

        let id = stash.id.clone();
        self.stashes.insert(id.clone(), stash);
        let _ = self.save();

        id
    }

    /// Add a file to a stash
    pub fn add_file_to_stash(
        &mut self,
        stash_id: &str,
        file_path: &str,
        content: &str,
        patch: &str,
        status: FileStatus,
    ) -> Result<()> {
        let stash = self.stashes.get_mut(stash_id)
            .context("Stash not found")?;

        let metadata = fs::metadata(self.repo_path.join(file_path))?;

        let stashed_file = StashedFile {
            path: file_path.to_string(),
            patch: patch.to_string(),
            original_content: content.to_string(),
            status,
            size: metadata.len(),
            last_modified: metadata.modified()?
                .duration_since(UNIX_EPOCH)?
                .as_secs(),
        };

        stash.add_file(stashed_file);
        self.save()?;

        Ok(())
    }

    /// Remove a file from a stash
    pub fn remove_file_from_stash(&mut self, stash_id: &str, file_path: &str) -> Result<StashedFile> {
        let stash = self.stashes.get_mut(stash_id)
            .context("Stash not found")?;

        let file = stash.remove_file(file_path)
            .context("File not found in stash")?;

        self.save()?;
        Ok(file)
    }

    /// Delete an entire stash
    pub fn delete_stash(&mut self, stash_id: &str) -> Result<VisualStash> {
        let stash = self.stashes.remove(stash_id)
            .context("Stash not found")?;

        self.save()?;
        Ok(stash)
    }

    /// Rename a stash
    pub fn rename_stash(&mut self, stash_id: &str, new_name: String) -> Result<()> {
        let stash = self.stashes.get_mut(stash_id)
            .context("Stash not found")?;

        stash.name = new_name;
        stash.update_modified();
        self.save()?;

        Ok(())
    }

    /// Get a stash by ID
    pub fn get_stash(&self, stash_id: &str) -> Option<&VisualStash> {
        self.stashes.get(stash_id)
    }

    /// Get all stashes
    pub fn all_stashes(&self) -> Vec<&VisualStash> {
        let mut stashes: Vec<&VisualStash> = self.stashes.values().collect();
        stashes.sort_by(|a, b| b.modified.cmp(&a.modified)); // Most recent first
        stashes
    }

    /// Apply a file from stash (restore it to working directory)
    pub fn apply_file(&self, stash_id: &str, file_path: &str) -> Result<String> {
        let stash = self.stashes.get(stash_id)
            .context("Stash not found")?;

        let file = stash.files.iter()
            .find(|f| f.path == file_path)
            .context("File not found in stash")?;

        // Return the original content to be written by the caller
        Ok(file.original_content.clone())
    }

    /// Apply entire stash (all files)
    pub fn apply_stash(&self, stash_id: &str) -> Result<Vec<(String, String)>> {
        let stash = self.stashes.get(stash_id)
            .context("Stash not found")?;

        let files: Vec<(String, String)> = stash.files.iter()
            .map(|f| (f.path.clone(), f.original_content.clone()))
            .collect();

        Ok(files)
    }

    /// Pop a stash (apply and delete)
    pub fn pop_stash(&mut self, stash_id: &str) -> Result<Vec<(String, String)>> {
        let files = self.apply_stash(stash_id)?;
        self.delete_stash(stash_id)?;
        Ok(files)
    }

    /// Get stashes by tag
    pub fn get_stashes_by_tag(&self, tag: &str) -> Vec<&VisualStash> {
        self.stashes.values()
            .filter(|s| s.tags.contains(&tag.to_string()))
            .collect()
    }

    /// Stash entire branch with all uncommitted work
    /// Returns stash ID
    pub fn stash_current_branch(
        &mut self,
        branch_name: String,
        branch_context: BranchContext,
        files: Vec<StashedFile>,
    ) -> String {
        let name = format!("Branch: {}", branch_name);
        let mut stash = VisualStash::new_branch_stash(name, branch_context);
        stash.files = files;
        stash.color = Some("#4A90E2".to_string()); // Blue for branch stashes

        let id = stash.id.clone();
        self.stashes.insert(id.clone(), stash);
        let _ = self.save();

        id
    }

    /// Restore a branch stash
    /// Returns (files to restore, branch to checkout, staged files)
    pub fn restore_branch_stash(
        &self,
        stash_id: &str,
    ) -> Result<(Vec<(String, String)>, BranchContext), String> {
        let stash = self.stashes.get(stash_id)
            .ok_or("Stash not found")?;

        if !stash.is_branch_stash() {
            return Err("Not a branch stash".to_string());
        }

        let branch_context = stash.branch_context.as_ref()
            .ok_or("Branch context missing")?
            .clone();

        let files: Vec<(String, String)> = stash.files.iter()
            .map(|f| (f.path.clone(), f.original_content.clone()))
            .collect();

        Ok((files, branch_context))
    }

    /// Get all branch stashes
    pub fn get_branch_stashes(&self) -> Vec<&VisualStash> {
        self.stashes.values()
            .filter(|s| s.is_branch_stash())
            .collect()
    }

    /// Stash and switch workflow: stash current branch, then checkout another
    /// This is a helper that returns what branch to switch to after stashing
    pub fn stash_and_switch(
        &mut self,
        current_branch: String,
        target_branch: String,
        branch_context: BranchContext,
        files: Vec<StashedFile>,
    ) -> Result<(String, String), String> {
        // Stash current branch
        let stash_id = self.stash_current_branch(current_branch.clone(), branch_context, files);

        // Return (stash_id, target_branch) for caller to execute switch
        Ok((stash_id, target_branch))
    }

    /// Save stashes to disk
    fn save(&self) -> Result<()> {
        let json = serde_json::to_string_pretty(&self.stashes)?;
        fs::write(&self.storage_path, json)?;
        Ok(())
    }

    /// Load stashes from disk
    fn load(&mut self) -> Result<()> {
        if self.storage_path.exists() {
            let json = fs::read_to_string(&self.storage_path)?;
            self.stashes = serde_json::from_str(&json)?;
        }
        Ok(())
    }

    /// Export stash to .patch file
    pub fn export_stash(&self, stash_id: &str, output_path: &Path) -> Result<()> {
        let stash = self.stashes.get(stash_id)
            .context("Stash not found")?;

        let mut patch_content = format!("# GitScribe Stash: {}\n", stash.name);
        patch_content.push_str(&format!("# Created: {}\n", stash.created));
        patch_content.push_str(&format!("# Files: {}\n\n", stash.files.len()));

        for file in &stash.files {
            patch_content.push_str(&format!("# File: {}\n", file.path));
            patch_content.push_str(&file.patch);
            patch_content.push_str("\n\n");
        }

        fs::write(output_path, patch_content)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::env;

    #[test]
    fn test_create_stash() {
        let temp_dir = env::temp_dir().join("gitscribe_test_stash");
        fs::create_dir_all(&temp_dir).unwrap();

        let mut manager = VisualStashManager::new(&temp_dir).unwrap();

        let stash_id = manager.create_stash(
            "Test Stash".to_string(),
            Some("#FF5733".to_string()),
            vec!["test".to_string()],
        );

        assert!(manager.get_stash(&stash_id).is_some());
        let stash = manager.get_stash(&stash_id).unwrap();
        assert_eq!(stash.name, "Test Stash");
        assert_eq!(stash.color, Some("#FF5733".to_string()));
        assert_eq!(stash.tags, vec!["test".to_string()]);

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn test_add_file_to_stash() {
        let temp_dir = env::temp_dir().join("gitscribe_test_stash_file");
        fs::create_dir_all(&temp_dir).unwrap();
        fs::write(temp_dir.join("test.txt"), "test content").unwrap();

        let mut manager = VisualStashManager::new(&temp_dir).unwrap();
        let stash_id = manager.create_stash("Test".to_string(), None, vec![]);

        manager.add_file_to_stash(
            &stash_id,
            "test.txt",
            "test content",
            "diff --git a/test.txt...",
            FileStatus::Modified,
        ).unwrap();

        let stash = manager.get_stash(&stash_id).unwrap();
        assert_eq!(stash.files.len(), 1);
        assert_eq!(stash.files[0].path, "test.txt");

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn test_persistence() {
        let temp_dir = env::temp_dir().join("gitscribe_test_persist");
        fs::create_dir_all(&temp_dir).unwrap();

        let stash_id = {
            let mut manager = VisualStashManager::new(&temp_dir).unwrap();
            manager.create_stash("Persistent".to_string(), None, vec![])
        };

        // Create new manager - should load existing stashes
        let manager2 = VisualStashManager::new(&temp_dir).unwrap();
        assert!(manager2.get_stash(&stash_id).is_some());

        fs::remove_dir_all(&temp_dir).unwrap();
    }
}
