#include "GitScribeOverlay.h"
#include "resource.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <unordered_map>
#include <mutex>
#include <atomic>

// Forward declare C API from gitscribe-core
extern "C" {
    typedef struct GSRepository GSRepository;

    typedef struct GSFileStatus {
        char* path;
        int status;
    } GSFileStatus;

    typedef struct GSStatusList {
        GSFileStatus* entries;
        size_t count;
    } GSStatusList;

    GSRepository* gs_repository_open(const char* path);
    int gs_file_status(GSRepository* repo, const char* path);
    void gs_repository_free(GSRepository* repo);
    GSStatusList* gs_repository_all_statuses(GSRepository* repo);
    void gs_status_list_free(GSStatusList* list);
}

#pragma comment(lib, "shlwapi.lib")

// Repository-level cache - stores all file statuses for a repository
struct RepoStatusCache {
    std::unordered_map<std::wstring, int> fileStatuses;  // path -> status
    std::unordered_map<std::wstring, int> folderStatuses; // folder path -> status (Modified if contains changes)
    std::wstring repoPath;                                // root path of repository
    DWORD timestamp;                                       // when cache was populated
};

// Path-to-repo mapping cache for fast lookups
struct PathRepoMapping {
    std::wstring repoRoot;
    DWORD timestamp;
};

// Single-path fast cache (like TortoiseSVN) - for repeated queries
struct FastPathCache {
    std::wstring path;
    int status;
    DWORD timestamp;
    std::mutex mutex;
};

static std::unordered_map<std::wstring, RepoStatusCache> g_repoCache;  // repoPath -> cache
static std::unordered_map<std::wstring, PathRepoMapping> g_pathToRepo; // path -> repo root mapping
static FastPathCache g_fastCache;                                       // Ultra-fast single entry cache
static std::mutex g_cacheMutex;
static const DWORD CACHE_TTL_MS = 30000;  // 30 second TTL (longer to avoid re-queries)
static const DWORD PATH_MAPPING_TTL_MS = 60000; // 60 second TTL for path->repo mappings
static const DWORD FAST_CACHE_TTL_MS = 200;    // 200ms TTL for fast cache

// Fast mode: Skip all overlay checks for a brief period after right-click
static std::atomic<DWORD> g_lastContextMenuTime(0);
static const DWORD CONTEXT_MENU_SKIP_MS = 500;  // Skip overlay checks for 500ms after context menu

// Convert wide string to UTF-8
std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], size, nullptr, nullptr);
    return utf8;
}

// Convert UTF-8 to wide string
std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();

    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring wide(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], size);
    return wide;
}

// Helper function to check if path is on network drive
bool IsNetworkPath(const std::wstring& path) {
    if (path.empty()) return false;

    // UNC paths (\\server\share)
    if (path.length() >= 2 && path[0] == L'\\' && path[1] == L'\\') {
        return true;
    }

    // Check drive type for mapped network drives
    if (path.length() >= 3 && path[1] == L':') {
        std::wstring root = path.substr(0, 3);  // e.g., "C:\"
        UINT driveType = GetDriveTypeW(root.c_str());
        return (driveType == DRIVE_REMOTE);
    }

    return false;
}

// Helper function to check fast cache
bool CheckFastCache(const std::wstring& path, int& status) {
    std::lock_guard<std::mutex> lock(g_fastCache.mutex);

    if (g_fastCache.path == path) {
        DWORD now = GetTickCount();
        if (now - g_fastCache.timestamp < FAST_CACHE_TTL_MS) {
            status = g_fastCache.status;
            return true;
        }
    }
    return false;
}

// Helper function to update fast cache
void UpdateFastCache(const std::wstring& path, int status) {
    std::lock_guard<std::mutex> lock(g_fastCache.mutex);
    g_fastCache.path = path;
    g_fastCache.status = status;
    g_fastCache.timestamp = GetTickCount();
}

// Get repository root path for a given file path
std::wstring GetRepoRoot(const std::wstring& path) {
    std::string utf8Path = WideToUtf8(path);
    GSRepository* repo = gs_repository_open(utf8Path.c_str());
    if (!repo) {
        return L"";  // Not in a repository
    }

    // We don't have a direct way to get repo path from C API, so we'll discover it
    // by walking up the directory tree looking for .git
    std::wstring testPath = path;
    if (PathFileExistsW((testPath + L"\\.git").c_str())) {
        gs_repository_free(repo);
        return testPath;
    }

    // If it's a file, start from parent directory
    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        wchar_t parent[MAX_PATH];
        wcscpy_s(parent, path.c_str());
        PathRemoveFileSpecW(parent);
        testPath = parent;
    }

    // Walk up looking for .git directory
    for (int i = 0; i < 20; i++) {  // Max 20 levels up
        if (PathFileExistsW((testPath + L"\\.git").c_str())) {
            gs_repository_free(repo);
            return testPath;
        }

        wchar_t parent[MAX_PATH];
        wcscpy_s(parent, testPath.c_str());
        if (!PathRemoveFileSpecW(parent) || wcslen(parent) <= 3) {  // Reached root
            break;
        }
        testPath = parent;
    }

    gs_repository_free(repo);
    return L"";  // Couldn't find .git
}

GitScribeOverlay::GitScribeOverlay(GitStatus status, int iconResourceId)
    : m_status(status)
    , m_iconResourceId(iconResourceId)
    , m_refCount(1) {
}

GitScribeOverlay::~GitScribeOverlay() {
}

// Static method to notify when context menu is shown
void GitScribeOverlay::NotifyContextMenu() {
    g_lastContextMenuTime.store(GetTickCount());
}

// IUnknown implementation
STDMETHODIMP GitScribeOverlay::QueryInterface(REFIID riid, void** ppv) {
    static const QITAB qit[] = {
        QITABENT(GitScribeOverlay, IShellIconOverlayIdentifier),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) GitScribeOverlay::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) GitScribeOverlay::Release() {
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

// IShellIconOverlayIdentifier implementation
STDMETHODIMP GitScribeOverlay::GetOverlayInfo(
    PWSTR pwszIconFile,
    int cchMax,
    int* pIndex,
    DWORD* pdwFlags) {

    // Map resource ID to icon filename
    const wchar_t* iconName = L"modified.ico";
    switch (m_iconResourceId) {
        case IDI_MODIFIED:   iconName = L"modified.ico"; break;
        case IDI_CLEAN:      iconName = L"clean.ico"; break;
        case IDI_ADDED:      iconName = L"added.ico"; break;
        case IDI_UNTRACKED:  iconName = L"untracked.ico"; break;
        case IDI_CONFLICTED: iconName = L"conflicted.ico"; break;
        case IDI_IGNORED:    iconName = L"ignored.ico"; break;
    }

    // Get DLL's directory path
    extern HINSTANCE g_hInstance;
    WCHAR dllPath[MAX_PATH];
    GetModuleFileNameW(g_hInstance, dllPath, MAX_PATH);
    PathRemoveFileSpecW(dllPath);  // Remove DLL filename, leaving directory

    // Build path to icon file - icons are in ../../../resources/icon-packs/default/
    StringCchPrintfW(pwszIconFile, cchMax,
        L"%s\\..\\..\\..\\resources\\icon-packs\\default\\%s", dllPath, iconName);

    *pIndex = 0;
    *pdwFlags = ISIOI_ICONFILE;

    return S_OK;
}

STDMETHODIMP GitScribeOverlay::GetPriority(int* pPriority) {
    // Higher priority = shown first
    // Modified files are most important
    switch (m_status) {
        case GitStatus::Conflicted: *pPriority = 0; break;  // Highest
        case GitStatus::Modified:   *pPriority = 1; break;
        case GitStatus::Added:      *pPriority = 2; break;
        case GitStatus::Untracked:  *pPriority = 3; break;
        case GitStatus::Clean:      *pPriority = 4; break;
        case GitStatus::Ignored:    *pPriority = 5; break;  // Lowest
        default:                    *pPriority = 10; break;
    }
    return S_OK;
}

STDMETHODIMP GitScribeOverlay::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib) {
    // FAST MODE: Skip all overlay checks during context menu (500ms grace period)
    DWORD now = GetTickCount();
    DWORD contextMenuTime = g_lastContextMenuTime.load();
    if (contextMenuTime > 0 && (now - contextMenuTime) < CONTEXT_MENU_SKIP_MS) {
        return S_FALSE;  // Don't show any overlays during right-click
    }

    // EARLY EXIT 1: Validate input
    if (!pwszPath || wcslen(pwszPath) < 3) {
        return S_FALSE;
    }

    std::wstring path(pwszPath);

    // EARLY EXIT 2: Skip network paths (too slow)
    if (IsNetworkPath(path)) {
        return S_FALSE;
    }

    // EARLY EXIT 3: Skip system folders
    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_SYSTEM)) {
        return S_FALSE;
    }

    // FAST CACHE: Check single-path cache first
    int cachedStatus;
    if (CheckFastCache(path, cachedStatus)) {
        GitStatus status = static_cast<GitStatus>(cachedStatus);
        if (status == GitStatus::Clean && m_status == GitStatus::Clean) {
            return S_FALSE;  // Don't show clean overlay
        }
        return (status == m_status) ? S_OK : S_FALSE;
    }

    // Handle both files and directories
    bool isDirectory = (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0;

    if (isDirectory) {
        // For directories, only show "Modified" overlay if contains changes
        // (simplified: just check if the directory itself has status)
        if (m_status != GitStatus::Modified) {
            UpdateFastCache(path, 0);  // Cache as clean
            return S_FALSE;
        }
    }

    // Check Git status (now uses cache)
    if (!IsFileStatus(path, m_status)) {
        return S_FALSE;
    }

    return S_OK;
}

bool GitScribeOverlay::IsFileStatus(const std::wstring& path, GitStatus expectedStatus) {
    DWORD now = GetTickCount();
    std::wstring repoRoot;

    // FAST PATH: Check cached path->repo mapping first
    {
        std::lock_guard<std::mutex> lock(g_cacheMutex);
        auto mapping = g_pathToRepo.find(path);
        if (mapping != g_pathToRepo.end()) {
            if (now - mapping->second.timestamp < PATH_MAPPING_TTL_MS) {
                repoRoot = mapping->second.repoRoot;
            } else {
                g_pathToRepo.erase(mapping);  // Expired
            }
        }
    }

    // SLOW PATH: Need to find repo root
    if (repoRoot.empty()) {
        repoRoot = GetRepoRoot(path);
        if (repoRoot.empty()) {
            return false;  // Not in a repository
        }

        // Cache the mapping for next time
        {
            std::lock_guard<std::mutex> lock(g_cacheMutex);
            PathRepoMapping mapping;
            mapping.repoRoot = repoRoot;
            mapping.timestamp = now;
            g_pathToRepo[path] = mapping;

            // Limit cache size
            if (g_pathToRepo.size() > 1000) {
                g_pathToRepo.clear();  // Simple eviction
            }
        }
    }

    // Check if we have a cached repository status
    RepoStatusCache* cache = nullptr;
    bool needsRefresh = false;

    {
        std::lock_guard<std::mutex> lock(g_cacheMutex);
        auto it = g_repoCache.find(repoRoot);

        if (it != g_repoCache.end()) {
            if (now - it->second.timestamp < CACHE_TTL_MS) {
                cache = &it->second;
            } else {
                needsRefresh = true;
            }
        } else {
            needsRefresh = true;
        }
    }

    // If we need to refresh, do bulk query
    if (needsRefresh) {
        std::string utf8RepoPath = WideToUtf8(repoRoot);
        GSRepository* repo = gs_repository_open(utf8RepoPath.c_str());
        if (!repo) {
            return false;
        }

        // BULK QUERY: Get all file statuses at once
        GSStatusList* list = gs_repository_all_statuses(repo);
        gs_repository_free(repo);

        if (!list) {
            return false;
        }

        // Populate cache with all file statuses + folder statuses
        {
            std::lock_guard<std::mutex> lock(g_cacheMutex);

            RepoStatusCache newCache;
            newCache.repoPath = repoRoot;
            newCache.timestamp = now;

            // Store all file statuses and build folder status map
            for (size_t i = 0; i < list->count; i++) {
                std::wstring filePath = Utf8ToWide(list->entries[i].path);

                // Make absolute path
                std::wstring absPath;
                if (PathIsRelativeW(filePath.c_str())) {
                    absPath = repoRoot + L"\\" + filePath;
                } else {
                    absPath = filePath;
                }

                // Store file status
                newCache.fileStatuses[absPath] = list->entries[i].status;

                // Mark all parent folders as Modified if file is not clean
                if (list->entries[i].status != 0) {  // 0 = Clean
                    wchar_t folderPath[MAX_PATH];
                    wcscpy_s(folderPath, absPath.c_str());

                    while (PathRemoveFileSpecW(folderPath) && wcslen(folderPath) > wcslen(repoRoot.c_str())) {
                        std::wstring folder(folderPath);
                        newCache.folderStatuses[folder] = 1;  // 1 = Modified
                    }
                }
            }

            g_repoCache[repoRoot] = std::move(newCache);
            cache = &g_repoCache[repoRoot];

            // Limit number of cached repos
            if (g_repoCache.size() > 10) {
                auto oldest = g_repoCache.begin();
                DWORD oldestTime = oldest->second.timestamp;
                for (auto it = g_repoCache.begin(); it != g_repoCache.end(); ++it) {
                    if (it->second.timestamp < oldestTime) {
                        oldest = it;
                        oldestTime = it->second.timestamp;
                    }
                }
                g_repoCache.erase(oldest);
            }
        }

        gs_status_list_free(list);
    }

    // Now look up status in cache (instant hash map lookup!)
    if (cache) {
        // Try file status first
        auto fileIt = cache->fileStatuses.find(path);
        if (fileIt != cache->fileStatuses.end()) {
            GitStatus fileStatus = static_cast<GitStatus>(fileIt->second);

            // Update fast cache for next time
            UpdateFastCache(path, fileIt->second);

            if (fileStatus == GitStatus::Clean && expectedStatus == GitStatus::Clean) {
                return false;  // Don't show clean overlay
            }

            return fileStatus == expectedStatus;
        }

        // Try folder status
        auto folderIt = cache->folderStatuses.find(path);
        if (folderIt != cache->folderStatuses.end()) {
            // Update fast cache
            UpdateFastCache(path, 1);  // 1 = Modified for folders

            // Folders only show Modified overlay
            return expectedStatus == GitStatus::Modified;
        }

        // Not in cache = clean
        UpdateFastCache(path, 0);  // 0 = Clean
        return false;
    }

    return false;
}

// Specific overlay implementations
ModifiedOverlay::ModifiedOverlay()
    : GitScribeOverlay(GitStatus::Modified, IDI_MODIFIED) {
}

CleanOverlay::CleanOverlay()
    : GitScribeOverlay(GitStatus::Clean, IDI_CLEAN) {
}

AddedOverlay::AddedOverlay()
    : GitScribeOverlay(GitStatus::Added, IDI_ADDED) {
}

UntrackedOverlay::UntrackedOverlay()
    : GitScribeOverlay(GitStatus::Untracked, IDI_UNTRACKED) {
}

ConflictedOverlay::ConflictedOverlay()
    : GitScribeOverlay(GitStatus::Conflicted, IDI_CONFLICTED) {
}

IgnoredOverlay::IgnoredOverlay()
    : GitScribeOverlay(GitStatus::Ignored, IDI_IGNORED) {
}
