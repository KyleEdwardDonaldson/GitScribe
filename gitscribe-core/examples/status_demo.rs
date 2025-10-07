//! Manual demo of gitscribe-core functionality
//!
//! Run with: cargo run --example status_demo

use gitscribe_core::{Repository, FileStatus};
use std::env;

fn main() -> anyhow::Result<()> {
    // Get path from command line or use current directory
    let path = env::args().nth(1).unwrap_or_else(|| ".".to_string());

    println!("GitScribe Core - Status Demo");
    println!("=============================\n");

    // Try to open repository
    println!("Opening repository at: {}", path);
    let repo = match Repository::open(&path) {
        Ok(r) => {
            println!("✓ Repository found at: {}\n", r.path().display());
            r
        }
        Err(e) => {
            println!("✗ Failed to open repository: {}", e);
            println!("\nTip: Run this from inside a Git repository, or pass a path:");
            println!("  cargo run --example status_demo -- /path/to/repo");
            return Ok(());
        }
    };

    // Get repository status
    println!("Repository Status:");
    println!("------------------");

    let statuses = repo.status()?;

    if statuses.is_empty() {
        println!("✓ Working directory clean - no changes\n");
    } else {
        println!("Found {} files with changes:\n", statuses.len());

        // Group by status
        let mut by_status: std::collections::HashMap<FileStatus, Vec<_>> =
            std::collections::HashMap::new();

        for entry in &statuses {
            by_status.entry(entry.status)
                .or_insert_with(Vec::new)
                .push(&entry.path);
        }

        // Print each group
        for (status, paths) in by_status.iter() {
            let (icon, label) = match status {
                FileStatus::Modified => ("Modified", "Modified"),
                FileStatus::Added => ("Added", "Added"),
                FileStatus::Deleted => ("Deleted", "Deleted"),
                FileStatus::Untracked => ("Untracked", "Untracked"),
                FileStatus::Conflicted => ("Conflicted", "Conflicted"),
                FileStatus::Ignored => ("Ignored", "Ignored"),
                FileStatus::Clean => ("Clean", "Clean"),
                FileStatus::Locked => ("Locked", "Locked"),
            };

            println!("{} ({}):", label, paths.len());
            for path in paths {
                println!("  {}", path.display());
            }
            println!();
        }
    }

    // Demonstrate file-specific status check
    if let Some(first_file) = statuses.first() {
        println!("Individual file status check:");
        println!("  File: {}", first_file.path.display());
        let status = repo.file_status(&first_file.path)?;
        println!("  Status: {:?}\n", status);
    }

    println!("Demo complete!");

    Ok(())
}
