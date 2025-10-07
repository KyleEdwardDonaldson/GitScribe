#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include "GitRepository.h"

// Context types (what user right-clicked on)
enum class ContextType {
    None,               // Not in a Git repository
    FileModified,       // Modified file
    FileUntracked,      // Untracked file
    FileConflicted,     // Conflicted file
    FileClean,          // Clean tracked file
    MultiSelection,     // Multiple files selected
    RepoDirty,          // Repository with changes
    RepoAhead,          // Clean repo, commits ahead
    RepoBehind,         // Clean repo, commits behind
    RepoClean,          // Clean repo, synced
    MergeInProgress,    // Merge/rebase in progress
};

// Context information for menu generation
class MenuContext {
public:
    MenuContext(const std::vector<std::wstring>& selectedPaths);
    ~MenuContext() = default;

    // Get context type
    ContextType GetType() const { return m_type; }

    // Check if in a repository
    bool InRepository() const { return m_repo != nullptr; }

    // Get repository info
    const RepositoryInfo& GetRepoInfo() const { return m_repoInfo; }

    // Get repository
    GitRepository* GetRepository() const { return m_repo.get(); }

    // Get selected paths
    const std::vector<std::wstring>& GetSelectedPaths() const { return m_selectedPaths; }

    // Get primary file (first selected file)
    const std::wstring& GetPrimaryFile() const;

    // Get file name only (no path)
    std::wstring GetPrimaryFileName() const;

private:
    ContextType m_type;
    std::vector<std::wstring> m_selectedPaths;
    std::unique_ptr<GitRepository> m_repo;
    RepositoryInfo m_repoInfo;

    // Detect context based on selection
    void DetectContext();

    // Get file status for single file
    GitStatus GetPrimaryFileStatus() const;
};
