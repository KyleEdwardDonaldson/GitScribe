#include "AppLauncher.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <sstream>
#include <filesystem>

// Get named pipe name for IPC
std::wstring AppLauncher::GetPipeName() {
    // Use user SID for security
    // For now, use simple name - TODO: add user SID
    return L"\\\\.\\pipe\\GitScribe.IPC";
}

// Check if app is already running by trying to connect to pipe
bool AppLauncher::IsAppRunning() {
    HANDLE hPipe = CreateFileW(
        GetPipeName().c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
        return true;
    }

    return false;
}

// Send JSON message to running app via named pipe
bool AppLauncher::SendToApp(const std::string& jsonMessage) {
    HANDLE hPipe = CreateFileW(
        GetPipeName().c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD bytesWritten = 0;
    bool success = WriteFile(
        hPipe,
        jsonMessage.c_str(),
        static_cast<DWORD>(jsonMessage.length()),
        &bytesWritten,
        NULL
    );

    CloseHandle(hPipe);
    return success;
}

// Get path to GitScribe.exe
std::wstring AppLauncher::GetAppPath() {
    // Try to find app in several locations
    std::vector<std::wstring> searchPaths = {
        // Same directory as shell extension
        L"GitScribe.exe",
        // Program Files
        L"C:\\Program Files\\GitScribe\\GitScribe.exe",
        // Local AppData
        L"", // Will fill this with actual LocalAppData path
    };

    // Get LocalAppData path
    WCHAR localAppData[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData) == S_OK) {
        std::wstring appPath = localAppData;
        appPath += L"\\GitScribe\\GitScribe.exe";
        searchPaths.push_back(appPath);
    }

    // Check each path
    for (const auto& path : searchPaths) {
        if (path.empty()) continue;

        if (PathFileExistsW(path.c_str())) {
            return path;
        }
    }

    // Not found - return empty
    return L"";
}

// Convert action enum to string
std::wstring AppLauncher::ActionToString(AppAction action) {
    switch (action) {
        case AppAction::Commit:       return L"commit";
        case AppAction::Diff:         return L"diff";
        case AppAction::History:      return L"history";
        case AppAction::Blame:        return L"blame";
        case AppAction::Status:       return L"status";
        case AppAction::Resolve:      return L"resolve";
        case AppAction::Settings:     return L"settings";
        case AppAction::QuickActions: return L"quick";
        default:                      return L"status";
    }
}

// Build JSON-RPC message for app
std::string AppLauncher::BuildLaunchMessage(AppAction action, const AppContext& context) {
    // Simple JSON construction - for production, use a JSON library
    std::ostringstream json;
    json << "{\n";
    json << "  \"jsonrpc\": \"2.0\",\n";
    json << "  \"method\": \"launchApp\",\n";
    json << "  \"params\": {\n";
    json << "    \"action\": \"" << std::string(ActionToString(action).begin(), ActionToString(action).end()) << "\",\n";
    json << "    \"context\": {\n";

    // Convert wide string to UTF-8 for JSON
    std::string repoPathUtf8(context.repoPath.begin(), context.repoPath.end());
    json << "      \"repo\": \"" << repoPathUtf8 << "\",\n";

    if (!context.branch.empty()) {
        std::string branchUtf8(context.branch.begin(), context.branch.end());
        json << "      \"branch\": \"" << branchUtf8 << "\",\n";
    }

    if (!context.files.empty()) {
        json << "      \"files\": [";
        for (size_t i = 0; i < context.files.size(); i++) {
            std::string fileUtf8(context.files[i].begin(), context.files[i].end());
            json << "\"" << fileUtf8 << "\"";
            if (i < context.files.size() - 1) json << ", ";
        }
        json << "],\n";
    }

    if (context.line > 0) {
        json << "      \"line\": " << context.line << ",\n";
    }

    json << "      \"status\": 1\n";  // Dummy status
    json << "    }\n";
    json << "  },\n";
    json << "  \"id\": 1\n";
    json << "}\n";

    return json.str();
}

// Main launch function
bool AppLauncher::Launch(AppAction action, const AppContext& context) {
    // Build message
    std::string message = BuildLaunchMessage(action, context);

    // Try to send to existing app first
    if (IsAppRunning()) {
        return SendToApp(message);
    }

    // App not running - need to launch it
    std::wstring appPath = GetAppPath();

    if (appPath.empty()) {
        // App not found - fallback to showing message
        MessageBoxW(NULL,
            L"GitScribe app not found.\n\n"
            L"The full GitScribe application is not yet installed.\n"
            L"For now, the context menu will use basic Git commands.",
            L"GitScribe",
            MB_OK | MB_ICONINFORMATION);
        return false;
    }

    // Build command line arguments
    std::wstring cmdLine = L"\"" + appPath + L"\" --" + ActionToString(action);

    if (!context.repoPath.empty()) {
        cmdLine += L" --repo=\"" + context.repoPath + L"\"";
    }

    if (!context.files.empty()) {
        cmdLine += L" --file=\"" + context.files[0] + L"\"";
    }

    // Launch the app
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    bool success = CreateProcessW(
        NULL,
        const_cast<LPWSTR>(cmdLine.c_str()),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        context.repoPath.c_str(),
        &si,
        &pi
    );

    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    return false;
}
