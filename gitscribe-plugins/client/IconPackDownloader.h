/**
 * GitScribe Icon Pack Downloader
 *
 * C++ client for downloading and installing icon packs from the GitScribe Marketplace.
 * Used by GitScribe Status shell extension.
 */

#pragma once

#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <functional>

#pragma comment(lib, "wininet.lib")

namespace GitScribe {

/**
 * Icon pack metadata from marketplace
 */
struct IconPackInfo {
    std::string id;
    std::string slug;
    std::string name;
    std::string description;
    std::string version;
    std::string downloadUrl;
    size_t packageSize;
    std::string checksum;  // SHA-256
    std::string style;
    std::string previewUrl;
    int downloads;
    float rating;
};

/**
 * Download progress callback
 */
using ProgressCallback = std::function<void(size_t downloaded, size_t total)>;

/**
 * Download result
 */
struct DownloadResult {
    bool success;
    std::wstring filePath;
    std::string error;
    std::string checksum;
};

/**
 * Icon Pack Downloader
 */
class IconPackDownloader {
public:
    /**
     * Fetch icon pack list from marketplace
     *
     * @param style Optional style filter (e.g., "neon", "minimal")
     * @return Vector of icon pack metadata
     */
    static std::vector<IconPackInfo> FetchIconPacks(const std::string& style = "");

    /**
     * Fetch single icon pack by slug
     *
     * @param slug Icon pack slug (e.g., "neon-city")
     * @return Icon pack metadata
     */
    static IconPackInfo FetchIconPack(const std::string& slug);

    /**
     * Download icon pack from marketplace
     *
     * @param slug Icon pack slug
     * @param destDir Destination directory (defaults to temp directory)
     * @param onProgress Optional progress callback
     * @return Download result with file path or error
     */
    static DownloadResult Download(
        const std::string& slug,
        const std::wstring& destDir = L"",
        ProgressCallback onProgress = nullptr
    );

    /**
     * Download icon pack from direct URL
     *
     * @param url Download URL
     * @param destPath Destination file path
     * @param onProgress Optional progress callback
     * @return Download result
     */
    static DownloadResult DownloadFromUrl(
        const std::string& url,
        const std::wstring& destPath,
        ProgressCallback onProgress = nullptr
    );

    /**
     * Verify downloaded file checksum
     *
     * @param filePath Path to downloaded file
     * @param expectedChecksum Expected SHA-256 hash
     * @return True if checksum matches
     */
    static bool VerifyChecksum(const std::wstring& filePath, const std::string& expectedChecksum);

    /**
     * Calculate SHA-256 hash of file
     *
     * @param filePath Path to file
     * @return SHA-256 hash as hex string
     */
    static std::string CalculateChecksum(const std::wstring& filePath);

    /**
     * Extract icon pack ZIP to destination directory
     *
     * @param zipPath Path to ZIP file
     * @param destDir Destination directory (e.g., %APPDATA%/GitScribe/icon-packs/neon-city)
     * @return True if extraction successful
     */
    static bool Extract(const std::wstring& zipPath, const std::wstring& destDir);

    /**
     * Install icon pack (download + verify + extract)
     *
     * @param slug Icon pack slug
     * @param installDir Base install directory (e.g., %APPDATA%/GitScribe/icon-packs)
     * @param onProgress Optional progress callback
     * @return True if installation successful
     */
    static bool Install(
        const std::string& slug,
        const std::wstring& installDir,
        ProgressCallback onProgress = nullptr
    );

    /**
     * Track download in analytics
     *
     * @param itemId Icon pack UUID
     * @param version Version string
     */
    static void TrackDownload(const std::string& itemId, const std::string& version);

    /**
     * Get icon packs directory
     *
     * @return Path to %APPDATA%/GitScribe/icon-packs
     */
    static std::wstring GetIconPacksDirectory();

    /**
     * List installed icon packs
     *
     * @return Vector of installed pack slugs
     */
    static std::vector<std::string> GetInstalledPacks();

private:
    static const std::string MARKETPLACE_URL;
    static const std::string API_BASE;

    /**
     * HTTP GET request helper
     *
     * @param url Request URL
     * @return Response body
     */
    static std::string HttpGet(const std::string& url);

    /**
     * Parse JSON response from marketplace API
     *
     * @param json JSON string
     * @return Icon pack info
     */
    static IconPackInfo ParseIconPackJson(const std::string& json);

    /**
     * Parse JSON array of icon packs
     *
     * @param json JSON string
     * @return Vector of icon pack info
     */
    static std::vector<IconPackInfo> ParseIconPacksJson(const std::string& json);
};

} // namespace GitScribe
