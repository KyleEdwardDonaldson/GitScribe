#include "PerformanceCache.h"
#include "resource.h"
#include <filesystem>

extern HINSTANCE g_hInstance;

PerformanceCache& PerformanceCache::Instance() {
    static PerformanceCache instance;
    return instance;
}

PerformanceCache::PerformanceCache() {
    OutputDebugStringA("[GitScribe] PerformanceCache initialized\n");
}

PerformanceCache::~PerformanceCache() {
    ReleaseIcons();
}

void PerformanceCache::PreloadIcons() {
    std::lock_guard<std::mutex> lock(m_iconMutex);

    if (m_menuIcon) {
        return; // Already loaded
    }

    OutputDebugStringA("[GitScribe] Preloading menu icon...\n");

    // Load menu icon
    HICON hIcon = (HICON)LoadImageW(
        g_hInstance,
        MAKEINTRESOURCEW(IDI_MENU),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        0
    );

    if (hIcon) {
        // Convert to HBITMAP for menu use
        ICONINFO iconInfo;
        if (GetIconInfo(hIcon, &iconInfo)) {
            m_menuIcon = iconInfo.hbmColor;
            DeleteObject(iconInfo.hbmMask);
        }
        DestroyIcon(hIcon);
        OutputDebugStringA("[GitScribe] Menu icon preloaded successfully\n");
    } else {
        OutputDebugStringA("[GitScribe] WARNING: Failed to load menu icon\n");
    }
}

void PerformanceCache::ReleaseIcons() {
    std::lock_guard<std::mutex> lock(m_iconMutex);

    if (m_menuIcon) {
        DeleteObject(m_menuIcon);
        m_menuIcon = nullptr;
    }
}

HBITMAP PerformanceCache::GetMenuIcon() {
    std::lock_guard<std::mutex> lock(m_iconMutex);
    return m_menuIcon;
}

bool PerformanceCache::HasDotGitDirectory(const std::wstring& path) {
    namespace fs = std::filesystem;

    try {
        fs::path currentPath(path);

        // Handle files vs directories
        if (fs::is_regular_file(currentPath)) {
            currentPath = currentPath.parent_path();
        }

        // Walk up directory tree looking for .git
        // Limit to 10 levels to prevent infinite loops
        for (int i = 0; i < 10 && !currentPath.empty(); i++) {
            fs::path gitPath = currentPath / ".git";

            if (fs::exists(gitPath)) {
                return true;
            }

            // Move to parent
            fs::path parent = currentPath.parent_path();
            if (parent == currentPath) {
                break; // Reached root
            }
            currentPath = parent;
        }
    }
    catch (...) {
        // Ignore filesystem errors
    }

    return false;
}

bool PerformanceCache::IsLikelyRepository(const std::wstring& path) {
    DWORD now = GetTickCount();

    {
        std::lock_guard<std::mutex> lock(m_repoCacheMutex);

        // Check cache first
        auto it = m_repoCache.find(path);
        if (it != m_repoCache.end()) {
            if (now < it->second.expiry) {
                // Cache hit
                return it->second.isRepo;
            } else {
                // Expired, remove
                m_repoCache.erase(it);
            }
        }
    }

    // Fast check: look for .git directory (no libgit2)
    bool isRepo = HasDotGitDirectory(path);

    {
        std::lock_guard<std::mutex> lock(m_repoCacheMutex);

        // Store in cache
        CacheEntry entry;
        entry.isRepo = isRepo;
        entry.expiry = now + CACHE_TTL_MS;
        m_repoCache[path] = entry;

        // Limit cache size to prevent memory bloat
        if (m_repoCache.size() > 1000) {
            // Remove oldest entries
            auto oldest = m_repoCache.begin();
            m_repoCache.erase(oldest);
        }
    }

    return isRepo;
}
