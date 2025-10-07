#pragma once

#include <windows.h>
#include <string>
#include <memory>
#include "../../gitscribe-core/include/gitscribe_core.h"
#include "GitScribeOverlay.h" // For GitStatus enum

// Repository state enum (must match Rust)
enum class RepoState {
    Clean = 0,
    Merging = 1,
    Rebasing = 2,
    CherryPicking = 3,
    Reverting = 4,
    Bisecting = 5
};

// Repository information
struct RepositoryInfo {
    RepoState state = RepoState::Clean;
    bool isClean = true;
    unsigned int modifiedCount = 0;
    unsigned int conflictedCount = 0;
    unsigned int aheadCount = 0;
    unsigned int behindCount = 0;
    std::wstring currentBranch;
};

// RAII wrapper for Git repository
class GitRepository {
public:
    explicit GitRepository(const std::wstring& path);
    ~GitRepository();

    // Delete copy constructor and assignment
    GitRepository(const GitRepository&) = delete;
    GitRepository& operator=(const GitRepository&) = delete;

    // Move constructor and assignment
    GitRepository(GitRepository&& other) noexcept;
    GitRepository& operator=(GitRepository&& other) noexcept;

    // Check if repository is valid
    bool IsValid() const { return m_repo != nullptr; }

    // Get repository information
    RepositoryInfo GetInfo();

    // Get file status
    GitStatus GetFileStatus(const std::wstring& path);

    // Get repository path
    const std::wstring& GetPath() const { return m_repoPath; }

private:
    GSRepository* m_repo;
    std::wstring m_repoPath;

    // Cache with 1-second TTL
    mutable RepositoryInfo m_cachedInfo;
    mutable DWORD m_cacheTime = 0;
    static const DWORD CACHE_TTL_MS = 1000; // 1 second

    // Helper to convert wstring to UTF-8
    static std::string WideToUtf8(const std::wstring& wide);
    static std::wstring Utf8ToWide(const std::string& utf8);
};

// Helper to find repository containing a path
std::unique_ptr<GitRepository> FindRepository(const std::wstring& path);
