#include "GitRepository.h"
#include <filesystem>
#include <windows.h>

// Convert wide string to UTF-8
std::string GitRepository::WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &str[0], size_needed, nullptr, nullptr);
    return str;
}

// Convert UTF-8 to wide string
std::wstring GitRepository::Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring wstr(size_needed - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wstr[0], size_needed);
    return wstr;
}

GitRepository::GitRepository(const std::wstring& path)
    : m_repo(nullptr)
    , m_repoPath(path)
{
    std::string utf8Path = WideToUtf8(path);
    m_repo = gs_repository_open(utf8Path.c_str());
}

GitRepository::~GitRepository() {
    if (m_repo) {
        gs_repository_free(m_repo);
    }
}

GitRepository::GitRepository(GitRepository&& other) noexcept
    : m_repo(other.m_repo)
    , m_repoPath(std::move(other.m_repoPath))
{
    other.m_repo = nullptr;
}

GitRepository& GitRepository::operator=(GitRepository&& other) noexcept {
    if (this != &other) {
        if (m_repo) {
            gs_repository_free(m_repo);
        }
        m_repo = other.m_repo;
        m_repoPath = std::move(other.m_repoPath);
        other.m_repo = nullptr;
    }
    return *this;
}

RepositoryInfo GitRepository::GetInfo() {
    RepositoryInfo info;

    if (!IsValid()) {
        return info;
    }

    // Check cache (1-second TTL)
    DWORD now = GetTickCount();
    if (m_cacheTime != 0 && (now - m_cacheTime) < CACHE_TTL_MS) {
        OutputDebugStringA("[GitScribe] Using cached repository info\n");
        return m_cachedInfo;
    }

    OutputDebugStringA("[GitScribe] Querying fresh repository info\n");

    // Get repository info from Rust
    GSRepoInfo gsInfo = {0};
    if (gs_repository_info(m_repo, &gsInfo) == 0) {
        info.state = static_cast<RepoState>(gsInfo.state);
        info.isClean = (gsInfo.is_clean != 0);
        info.modifiedCount = gsInfo.modified_count;
        info.conflictedCount = gsInfo.conflicted_count;
        info.aheadCount = gsInfo.ahead_count;
        info.behindCount = gsInfo.behind_count;
    }

    // Get current branch
    char* branch = gs_repository_current_branch(m_repo);
    if (branch) {
        info.currentBranch = Utf8ToWide(branch);
        gs_string_free(branch);
    }

    // Update cache
    m_cachedInfo = info;
    m_cacheTime = now;

    return info;
}

GitStatus GitRepository::GetFileStatus(const std::wstring& path) {
    if (!IsValid()) {
        return GitStatus::Clean;
    }

    std::string utf8Path = WideToUtf8(path);
    int status = gs_file_status(m_repo, utf8Path.c_str());

    if (status < 0) {
        return GitStatus::Clean;
    }

    return static_cast<GitStatus>(status);
}

std::unique_ptr<GitRepository> FindRepository(const std::wstring& path) {
    namespace fs = std::filesystem;

    try {
        fs::path fsPath(path);

        // If it's a file, start from its directory
        if (fs::is_regular_file(fsPath)) {
            fsPath = fsPath.parent_path();
        }

        // Try to open repository at this path
        // git2 will search upward for .git directory
        auto repo = std::make_unique<GitRepository>(fsPath.wstring());

        if (repo->IsValid()) {
            return repo;
        }
    }
    catch (...) {
        // Ignore errors
    }

    return nullptr;
}
