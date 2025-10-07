//! Temporary Ignore Manager
//!
//! Manages files that are temporarily excluded from Git with the
//! intention to include them later. Supports auto-expiry and conditions.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};
use anyhow::{Result, Context};

/// A file that is temporarily ignored
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TemporaryIgnore {
    pub path: String,
    pub reason: Option<String>,
    pub include_when: IncludeCondition,
    pub added_by: String,
    pub added_date: u64,
    pub tags: Vec<String>,
}

/// Conditions for when to include the file
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum IncludeCondition {
    /// Manual inclusion only
    Manual,
    /// Include after a specific date
    AfterDate(u64),
    /// Include when a condition is met (free-form text)
    Condition(String),
}

/// Configuration for the temp ignore system
#[derive(Debug, Serialize, Deserialize)]
pub struct TempIgnoreConfig {
    pub version: String,
    pub ignores: Vec<TemporaryIgnore>,
    pub settings: TempIgnoreSettings,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct TempIgnoreSettings {
    pub auto_expire: bool,
    pub warn_before_expire_days: u32,
    pub suggest_review_after_days: u32,
}

impl Default for TempIgnoreSettings {
    fn default() -> Self {
        Self {
            auto_expire: true,
            warn_before_expire_days: 3,
            suggest_review_after_days: 30,
        }
    }
}

/// Manages temporary ignores for a repository
pub struct TempIgnoreManager {
    ignores: HashMap<String, TemporaryIgnore>,
    settings: TempIgnoreSettings,
    config_path: PathBuf,
    exclude_path: PathBuf,
    repo_path: PathBuf,
}

impl TempIgnoreManager {
    /// Create a new temp ignore manager for a repository
    pub fn new(repo_path: &Path) -> Result<Self> {
        let config_path = repo_path.join(".gitscribe").join("temp-ignore.json");
        let exclude_path = repo_path.join(".git").join("info").join("exclude");

        // Create directory if it doesn't exist
        if let Some(parent) = config_path.parent() {
            fs::create_dir_all(parent)?;
        }

        let mut manager = Self {
            ignores: HashMap::new(),
            settings: TempIgnoreSettings::default(),
            config_path,
            exclude_path,
            repo_path: repo_path.to_path_buf(),
        };

        // Load existing ignores
        manager.load()?;

        Ok(manager)
    }

    /// Add a temporary ignore
    pub fn add_temp_ignore(
        &mut self,
        path: String,
        reason: Option<String>,
        include_when: IncludeCondition,
        tags: Vec<String>,
    ) -> Result<()> {
        let username = std::env::var("USERNAME")
            .or_else(|_| std::env::var("USER"))
            .unwrap_or_else(|_| "unknown".to_string());

        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)?
            .as_secs();

        let ignore = TemporaryIgnore {
            path: path.clone(),
            reason,
            include_when,
            added_by: username,
            added_date: now,
            tags,
        };

        self.ignores.insert(path.clone(), ignore);
        self.update_exclude_file()?;
        self.save()?;

        Ok(())
    }

    /// Remove a temporary ignore (include the file)
    pub fn include_file(&mut self, path: &str) -> Result<()> {
        self.ignores.remove(path);
        self.update_exclude_file()?;
        self.save()?;
        Ok(())
    }

    /// Check if a file is temporarily ignored
    pub fn is_temp_ignored(&self, path: &str) -> bool {
        self.ignores.contains_key(path)
    }

    /// Get all temporary ignores
    pub fn all_ignores(&self) -> Vec<&TemporaryIgnore> {
        self.ignores.values().collect()
    }

    /// Get ignores that are expiring soon
    pub fn get_expiring_ignores(&self) -> Vec<&TemporaryIgnore> {
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();

        let warn_threshold = self.settings.warn_before_expire_days as u64 * 86400; // days to seconds

        self.ignores.values()
            .filter(|ignore| {
                if let IncludeCondition::AfterDate(expire_date) = ignore.include_when {
                    expire_date <= now + warn_threshold && expire_date > now
                } else {
                    false
                }
            })
            .collect()
    }

    /// Get ignores that should be reviewed
    pub fn get_review_candidates(&self) -> Vec<&TemporaryIgnore> {
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();

        let review_threshold = self.settings.suggest_review_after_days as u64 * 86400;

        self.ignores.values()
            .filter(|ignore| {
                now - ignore.added_date > review_threshold
            })
            .collect()
    }

    /// Get ignores by tag
    pub fn get_by_tag(&self, tag: &str) -> Vec<&TemporaryIgnore> {
        self.ignores.values()
            .filter(|ignore| ignore.tags.contains(&tag.to_string()))
            .collect()
    }

    /// Process expired ignores
    pub fn process_expired(&mut self) -> Result<Vec<String>> {
        if !self.settings.auto_expire {
            return Ok(vec![]);
        }

        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)?
            .as_secs();

        let expired: Vec<String> = self.ignores.iter()
            .filter(|(_, ignore)| {
                if let IncludeCondition::AfterDate(expire_date) = ignore.include_when {
                    expire_date <= now
                } else {
                    false
                }
            })
            .map(|(path, _)| path.clone())
            .collect();

        for path in &expired {
            self.ignores.remove(path);
        }

        if !expired.is_empty() {
            self.update_exclude_file()?;
            self.save()?;
        }

        Ok(expired)
    }

    /// Update the .git/info/exclude file with current temp ignores
    fn update_exclude_file(&self) -> Result<()> {
        // Read existing exclude file
        let existing_content = if self.exclude_path.exists() {
            fs::read_to_string(&self.exclude_path)?
        } else {
            String::new()
        };

        // Remove old GitScribe section
        let marker_start = "# === GITSCRIBE TEMPORARY IGNORES ===";
        let marker_end = "# === END GITSCRIBE TEMPORARY IGNORES ===";

        let mut lines: Vec<String> = existing_content
            .lines()
            .map(String::from)
            .collect();

        // Remove old section
        if let Some(start_idx) = lines.iter().position(|l| l.contains(marker_start)) {
            if let Some(end_idx) = lines.iter().position(|l| l.contains(marker_end)) {
                lines.drain(start_idx..=end_idx);
            }
        }

        // Add new section if there are temp ignores
        if !self.ignores.is_empty() {
            lines.push(String::new());
            lines.push(marker_start.to_string());

            for (path, ignore) in &self.ignores {
                let mut comment_parts = vec!["Temp:".to_string()];

                if let Some(ref reason) = ignore.reason {
                    comment_parts.push(reason.clone());
                }

                match &ignore.include_when {
                    IncludeCondition::AfterDate(timestamp) => {
                        comment_parts.push(format!("Expires: {}", timestamp));
                    }
                    IncludeCondition::Condition(cond) => {
                        comment_parts.push(format!("When: {}", cond));
                    }
                    IncludeCondition::Manual => {}
                }

                let comment = comment_parts.join(" | ");
                lines.push(format!("{}  # {}", path, comment));
            }

            lines.push(marker_end.to_string());
        }

        // Write back
        fs::write(&self.exclude_path, lines.join("\n"))?;

        Ok(())
    }

    /// Save config to disk
    fn save(&self) -> Result<()> {
        let config = TempIgnoreConfig {
            version: "1.0.0".to_string(),
            ignores: self.ignores.values().cloned().collect(),
            settings: TempIgnoreSettings {
                auto_expire: self.settings.auto_expire,
                warn_before_expire_days: self.settings.warn_before_expire_days,
                suggest_review_after_days: self.settings.suggest_review_after_days,
            },
        };

        let json = serde_json::to_string_pretty(&config)?;
        fs::write(&self.config_path, json)?;
        Ok(())
    }

    /// Load config from disk
    fn load(&mut self) -> Result<()> {
        if self.config_path.exists() {
            let json = fs::read_to_string(&self.config_path)?;
            let config: TempIgnoreConfig = serde_json::from_str(&json)?;

            self.settings = config.settings;
            self.ignores = config.ignores.into_iter()
                .map(|ignore| (ignore.path.clone(), ignore))
                .collect();
        }
        Ok(())
    }

    /// Update settings
    pub fn update_settings(&mut self, settings: TempIgnoreSettings) -> Result<()> {
        self.settings = settings;
        self.save()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::env;

    #[test]
    fn test_add_temp_ignore() {
        let temp_dir = env::temp_dir().join("gitscribe_test_tempignore");
        fs::create_dir_all(&temp_dir.join(".git").join("info")).unwrap();

        let mut manager = TempIgnoreManager::new(&temp_dir).unwrap();

        manager.add_temp_ignore(
            ".env.local".to_string(),
            Some("Contains test secrets".to_string()),
            IncludeCondition::Manual,
            vec!["secrets".to_string()],
        ).unwrap();

        assert!(manager.is_temp_ignored(".env.local"));
        assert_eq!(manager.all_ignores().len(), 1);

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn test_include_file() {
        let temp_dir = env::temp_dir().join("gitscribe_test_include");
        fs::create_dir_all(&temp_dir.join(".git").join("info")).unwrap();

        let mut manager = TempIgnoreManager::new(&temp_dir).unwrap();

        manager.add_temp_ignore(
            "test.txt".to_string(),
            None,
            IncludeCondition::Manual,
            vec![],
        ).unwrap();

        assert!(manager.is_temp_ignored("test.txt"));

        manager.include_file("test.txt").unwrap();
        assert!(!manager.is_temp_ignored("test.txt"));

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn test_expiry() {
        let temp_dir = env::temp_dir().join("gitscribe_test_expiry");
        fs::create_dir_all(&temp_dir.join(".git").join("info")).unwrap();

        let mut manager = TempIgnoreManager::new(&temp_dir).unwrap();

        // Add file that expired 1 day ago
        let yesterday = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs() - 86400;

        manager.add_temp_ignore(
            "expired.txt".to_string(),
            None,
            IncludeCondition::AfterDate(yesterday),
            vec![],
        ).unwrap();

        let expired = manager.process_expired().unwrap();
        assert_eq!(expired.len(), 1);
        assert_eq!(expired[0], "expired.txt");
        assert!(!manager.is_temp_ignored("expired.txt"));

        fs::remove_dir_all(&temp_dir).unwrap();
    }
}
