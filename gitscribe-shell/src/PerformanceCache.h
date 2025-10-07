#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>
#include <mutex>

// Performance cache for shell extension
class PerformanceCache {
public:
    static PerformanceCache& Instance();

    // Icon cache
    HBITMAP GetMenuIcon();
    void PreloadIcons();
    void ReleaseIcons();

    // Repository detection cache (very fast, no libgit2)
    bool IsLikelyRepository(const std::wstring& path);

private:
    PerformanceCache();
    ~PerformanceCache();

    // Prevent copying
    PerformanceCache(const PerformanceCache&) = delete;
    PerformanceCache& operator=(const PerformanceCache&) = delete;

    // Icon cache
    HBITMAP m_menuIcon = nullptr;
    std::mutex m_iconMutex;

    // Repository cache (path -> expiry time)
    struct CacheEntry {
        bool isRepo;
        DWORD expiry;
    };
    std::unordered_map<std::wstring, CacheEntry> m_repoCache;
    std::mutex m_repoCacheMutex;
    static const DWORD CACHE_TTL_MS = 5000; // 5 seconds

    // Fast .git detection without libgit2
    bool HasDotGitDirectory(const std::wstring& path);
};

// Global accessor
inline PerformanceCache& GetCache() {
    return PerformanceCache::Instance();
}
