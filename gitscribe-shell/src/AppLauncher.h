#pragma once

#include <windows.h>
#include <string>
#include <vector>

// App launch actions
enum class AppAction {
    Commit,
    Diff,
    History,
    Blame,
    Status,
    Resolve,
    Settings,
    QuickActions
};

// Context data to pass to app
struct AppContext {
    std::wstring repoPath;
    std::vector<std::wstring> files;
    std::wstring branch;
    int line = 0;  // For blame/diff at specific line
};

// Helper class for launching and communicating with GitScribe app
class AppLauncher {
public:
    // Launch app with specific action and context
    static bool Launch(AppAction action, const AppContext& context);

    // Check if app is running
    static bool IsAppRunning();

    // Send message to running app via named pipe
    static bool SendToApp(const std::string& jsonMessage);

    // Get app executable path
    static std::wstring GetAppPath();

private:
    static std::wstring ActionToString(AppAction action);
    static std::string BuildLaunchMessage(AppAction action, const AppContext& context);
    static std::wstring GetPipeName();
};
