/**
 * GitScribe Icon Pack Downloader - Implementation
 */

#include "IconPackDownloader.h"
#include <shlobj.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <wincrypt.h>
#include <nlohmann/json.hpp>

#pragma comment(lib, "crypt32.lib")

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace GitScribe {

const std::string IconPackDownloader::MARKETPLACE_URL = "https://gitscribe.dev";
const std::string IconPackDownloader::API_BASE = "https://gitscribe.dev/api/marketplace";

std::vector<IconPackInfo> IconPackDownloader::FetchIconPacks(const std::string& style) {
    std::string url = API_BASE + "/icon-packs";
    if (!style.empty()) {
        url += "?style=" + style;
    }

    std::string response = HttpGet(url);
    return ParseIconPacksJson(response);
}

IconPackInfo IconPackDownloader::FetchIconPack(const std::string& slug) {
    std::string url = API_BASE + "/icon-packs/" + slug;
    std::string response = HttpGet(url);
    return ParseIconPackJson(response);
}

DownloadResult IconPackDownloader::Download(
    const std::string& slug,
    const std::wstring& destDir,
    ProgressCallback onProgress
) {
    DownloadResult result = { false, L"", "", "" };

    try {
        // Fetch metadata
        IconPackInfo info = FetchIconPack(slug);

        // Determine destination path
        std::wstring targetDir = destDir.empty() ? fs::temp_directory_path().wstring() : destDir;
        std::wstring filename = std::wstring(slug.begin(), slug.end()) + L".zip";
        std::wstring destPath = (fs::path(targetDir) / filename).wstring();

        // Download file
        auto downloadResult = DownloadFromUrl(info.downloadUrl, destPath, onProgress);
        if (!downloadResult.success) {
            return downloadResult;
        }

        // Verify checksum
        if (!VerifyChecksum(destPath, info.checksum)) {
            result.error = "Checksum verification failed";
            fs::remove(destPath);
            return result;
        }

        // Track download
        TrackDownload(info.id, info.version);

        result.success = true;
        result.filePath = destPath;
        result.checksum = info.checksum;

    } catch (const std::exception& ex) {
        result.error = ex.what();
    }

    return result;
}

DownloadResult IconPackDownloader::DownloadFromUrl(
    const std::string& url,
    const std::wstring& destPath,
    ProgressCallback onProgress
) {
    DownloadResult result = { false, L"", "", "" };

    HINTERNET hInternet = InternetOpenA(
        "GitScribe/1.0",
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0
    );

    if (!hInternet) {
        result.error = "Failed to initialize WinINet";
        return result;
    }

    HINTERNET hUrl = InternetOpenUrlA(
        hInternet,
        url.c_str(),
        NULL,
        0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
        0
    );

    if (!hUrl) {
        InternetCloseHandle(hInternet);
        result.error = "Failed to open URL";
        return result;
    }

    // Get content length
    DWORD contentLength = 0;
    DWORD bufferLength = sizeof(contentLength);
    DWORD index = 0;
    HttpQueryInfoA(
        hUrl,
        HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
        &contentLength,
        &bufferLength,
        &index
    );

    // Download to file
    std::ofstream outFile(destPath, std::ios::binary);
    if (!outFile) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        result.error = "Failed to create destination file";
        return result;
    }

    const DWORD BUFFER_SIZE = 8192;
    BYTE buffer[BUFFER_SIZE];
    DWORD bytesRead = 0;
    DWORD totalRead = 0;

    while (InternetReadFile(hUrl, buffer, BUFFER_SIZE, &bytesRead) && bytesRead > 0) {
        outFile.write(reinterpret_cast<char*>(buffer), bytesRead);
        totalRead += bytesRead;

        if (onProgress) {
            onProgress(totalRead, contentLength);
        }
    }

    outFile.close();
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    result.success = true;
    result.filePath = destPath;
    result.checksum = CalculateChecksum(destPath);

    return result;
}

bool IconPackDownloader::VerifyChecksum(const std::wstring& filePath, const std::string& expectedChecksum) {
    std::string actualChecksum = CalculateChecksum(filePath);
    return actualChecksum == expectedChecksum;
}

std::string IconPackDownloader::CalculateChecksum(const std::wstring& filePath) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        return "";
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return "";
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    const DWORD BUFFER_SIZE = 8192;
    BYTE buffer[BUFFER_SIZE];

    while (file.read(reinterpret_cast<char*>(buffer), BUFFER_SIZE) || file.gcount() > 0) {
        DWORD bytesRead = static_cast<DWORD>(file.gcount());
        CryptHashData(hHash, buffer, bytesRead, 0);
    }

    file.close();

    DWORD hashLen = 0;
    DWORD hashLenSize = sizeof(DWORD);
    CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashLen, &hashLenSize, 0);

    BYTE* hashBytes = new BYTE[hashLen];
    CryptGetHashParam(hHash, HP_HASHVAL, hashBytes, &hashLen, 0);

    // Convert to hex string
    std::ostringstream oss;
    for (DWORD i = 0; i < hashLen; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hashBytes[i];
    }

    delete[] hashBytes;
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return oss.str();
}

bool IconPackDownloader::Extract(const std::wstring& zipPath, const std::wstring& destDir) {
    // Create destination directory
    fs::create_directories(destDir);

    // Use Windows Shell API to extract ZIP
    IShellDispatch* pISD;
    Folder* pZipFolder = nullptr;
    Folder* pDestFolder = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void**)&pISD);
    if (FAILED(hr)) return false;

    VARIANT vZipPath;
    VariantInit(&vZipPath);
    vZipPath.vt = VT_BSTR;
    vZipPath.bstrVal = SysAllocString(zipPath.c_str());

    hr = pISD->NameSpace(vZipPath, &pZipFolder);
    VariantClear(&vZipPath);

    if (FAILED(hr) || !pZipFolder) {
        pISD->Release();
        return false;
    }

    VARIANT vDestPath;
    VariantInit(&vDestPath);
    vDestPath.vt = VT_BSTR;
    vDestPath.bstrVal = SysAllocString(destDir.c_str());

    hr = pISD->NameSpace(vDestPath, &pDestFolder);
    VariantClear(&vDestPath);

    if (FAILED(hr) || !pDestFolder) {
        pZipFolder->Release();
        pISD->Release();
        return false;
    }

    FolderItems* pFolderItems = nullptr;
    hr = pZipFolder->Items(&pFolderItems);

    if (SUCCEEDED(hr) && pFolderItems) {
        VARIANT vItem;
        VariantInit(&vItem);
        vItem.vt = VT_DISPATCH;
        vItem.pdispVal = pFolderItems;

        VARIANT vOptions;
        VariantInit(&vOptions);
        vOptions.vt = VT_I4;
        vOptions.lVal = FOF_NO_UI; // No progress dialog

        hr = pDestFolder->CopyHere(vItem, vOptions);

        VariantClear(&vItem);
        VariantClear(&vOptions);
        pFolderItems->Release();
    }

    pDestFolder->Release();
    pZipFolder->Release();
    pISD->Release();

    return SUCCEEDED(hr);
}

bool IconPackDownloader::Install(
    const std::string& slug,
    const std::wstring& installDir,
    ProgressCallback onProgress
) {
    // Download to temp directory
    auto downloadResult = Download(slug, L"", onProgress);
    if (!downloadResult.success) {
        return false;
    }

    // Extract to install directory
    std::wstring packDir = (fs::path(installDir) / slug).wstring();
    bool extracted = Extract(downloadResult.filePath, packDir);

    // Clean up temp file
    fs::remove(downloadResult.filePath);

    return extracted;
}

void IconPackDownloader::TrackDownload(const std::string& itemId, const std::string& version) {
    // Build JSON payload
    json payload = {
        {"itemId", itemId},
        {"itemType", "icon-pack"},
        {"version", version}
    };

    std::string jsonStr = payload.dump();

    // Send POST request (fire and forget)
    HINTERNET hInternet = InternetOpenA("GitScribe/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return;

    HINTERNET hConnect = InternetConnectA(
        hInternet,
        "gitscribe.dev",
        INTERNET_DEFAULT_HTTPS_PORT,
        NULL, NULL,
        INTERNET_SERVICE_HTTP,
        0, 0
    );

    if (hConnect) {
        HINTERNET hRequest = HttpOpenRequestA(
            hConnect,
            "POST",
            "/api/marketplace/downloads",
            NULL, NULL, NULL,
            INTERNET_FLAG_SECURE,
            0
        );

        if (hRequest) {
            const char* headers = "Content-Type: application/json\r\n";
            HttpSendRequestA(hRequest, headers, -1, (LPVOID)jsonStr.c_str(), jsonStr.length());
            InternetCloseHandle(hRequest);
        }

        InternetCloseHandle(hConnect);
    }

    InternetCloseHandle(hInternet);
}

std::wstring IconPackDownloader::GetIconPacksDirectory() {
    WCHAR appDataPath[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath);

    fs::path iconPacksDir = fs::path(appDataPath) / L"GitScribe" / L"icon-packs";
    fs::create_directories(iconPacksDir);

    return iconPacksDir.wstring();
}

std::vector<std::string> IconPackDownloader::GetInstalledPacks() {
    std::vector<std::string> packs;
    std::wstring packsDir = GetIconPacksDirectory();

    for (const auto& entry : fs::directory_iterator(packsDir)) {
        if (entry.is_directory()) {
            std::string packName = entry.path().filename().string();
            packs.push_back(packName);
        }
    }

    return packs;
}

std::string IconPackDownloader::HttpGet(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("GitScribe/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) throw std::runtime_error("Failed to initialize WinINet");

    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        throw std::runtime_error("Failed to open URL");
    }

    std::ostringstream oss;
    const DWORD BUFFER_SIZE = 4096;
    BYTE buffer[BUFFER_SIZE];
    DWORD bytesRead = 0;

    while (InternetReadFile(hUrl, buffer, BUFFER_SIZE, &bytesRead) && bytesRead > 0) {
        oss.write(reinterpret_cast<char*>(buffer), bytesRead);
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    return oss.str();
}

IconPackInfo IconPackDownloader::ParseIconPackJson(const std::string& jsonStr) {
    json j = json::parse(jsonStr);

    IconPackInfo info;
    info.id = j["id"];
    info.slug = j["slug"];
    info.name = j["name"];
    info.description = j["description"];
    info.version = j["version"];
    info.downloadUrl = j["downloadUrl"];
    info.packageSize = j["packageSize"];
    info.checksum = j["checksum"];
    info.style = j["style"];
    info.previewUrl = j["previewUrl"];
    info.downloads = j["downloads"];
    info.rating = j["rating"];

    return info;
}

std::vector<IconPackInfo> IconPackDownloader::ParseIconPacksJson(const std::string& jsonStr) {
    json j = json::parse(jsonStr);
    std::vector<IconPackInfo> packs;

    for (const auto& item : j["items"]) {
        IconPackInfo info;
        info.id = item["id"];
        info.slug = item["slug"];
        info.name = item["name"];
        info.description = item["description"];
        info.version = item["version"];
        info.downloadUrl = item["downloadUrl"];
        info.packageSize = item["packageSize"];
        info.checksum = item["checksum"];
        info.style = item["style"];
        info.previewUrl = item["previewUrl"];
        info.downloads = item["downloads"];
        info.rating = item["rating"];

        packs.push_back(info);
    }

    return packs;
}

} // namespace GitScribe
