# gitscribe-shell - Development Guide

## Critical Principles

This is the most critical component of GitScribe. A crash here crashes Explorer.

**Rules:**
1. **Never crash**: Use exception handlers everywhere
2. **Never block**: All operations must be async or cached
3. **Minimal**: Shell extensions must be lightweight
4. **Stable**: Thorough testing before any release

## Two Build Configurations

### Lite (GITSCRIBE_LITE defined)
- **Purpose**: Gateway product, free forever
- **Size**: ~100KB DLL + minimal core
- **Features**: Overlay icons ONLY
- **No dependencies**: Doesn't need gitscribe-app
- **Strategy**: Get users hooked on visual status, upsell to full version

### Full (default)
- **Purpose**: Complete Git client
- **Size**: ~500KB DLL + full core + Electron app
- **Features**: Everything
- **Monetization**: Free for personal, paid for commercial/enterprise

## Architecture

Shell extension runs IN-PROCESS with Explorer.exe. This means:
- Your code runs in Microsoft's process
- Your crash is Explorer's crash
- Your slowdown is Explorer's slowdown
- Your memory leak affects all of Explorer

## COM Threading Model

Shell extensions use the Apartment threading model:
- Main thread: `STA` (Single-Threaded Apartment)
- Background threads: Use `CoInitializeEx` properly
- Always marshal interfaces across threads

## Performance Budget

Every operation has a budget:

| Operation | Target | Absolute Max |
|-----------|--------|--------------|
| `GetOverlayInfo` | <5ms | 50ms |
| `IsMemberOf` | <10ms | 100ms |
| `QueryContextMenu` | <20ms | 150ms |
| `InvokeCommand` | <50ms | 500ms |

**Exceeding these will make Explorer feel slow.**

## Caching Strategy

### Icon Overlays
- Cache in-memory for 1 second
- Query gitscribe-core in background thread
- Return cached result immediately
- Update cache async

### Context Menus
- Build static menu structure (fast)
- Enable/disable items based on cached state
- Update in background if needed

## Communication with Core

Shell extension links to `gitscribe-core.dll` via C API:

```cpp
#include "gitscribe_core.h"

// Always check return codes
gs_repository* repo = gs_repository_open(path);
if (!repo) {
    // Handle error - don't crash!
    return E_FAIL;
}

gs_status* status = gs_repository_status(repo);
// Use status...

gs_repository_free(repo);
```

## Error Handling

**Never let exceptions escape to Explorer:**

```cpp
STDMETHODIMP ContextMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu,
                                           UINT idCmdFirst, UINT idCmdLast,
                                           UINT uFlags)
{
    try {
        // Your code here
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(numAdded));
    }
    catch (const std::exception& e) {
        OutputDebugStringA(e.what());
        return E_FAIL;
    }
    catch (...) {
        OutputDebugStringA("Unknown exception in QueryContextMenu");
        return E_FAIL;
    }
}
```

## Logging

Use `OutputDebugString` for logging:

```cpp
void Log(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    OutputDebugStringA("[GitScribe] ");
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}
```

View logs with **DebugView** from Sysinternals.

## Conditional Compilation

Use preprocessor directives to separate Lite and Full features:

```cpp
// ShellExt.cpp
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    // Always register overlay icons
    if (rclsid == CLSID_GitScribeOverlayModified) {
        return CreateIconOverlay(ppv);
    }

#ifndef GITSCRIBE_LITE
    // Only in full version
    if (rclsid == CLSID_GitScribeContextMenu) {
        return CreateContextMenu(ppv);
    }
    if (rclsid == CLSID_GitScribePropertySheet) {
        return CreatePropertySheet(ppv);
    }
#endif

    return CLASS_E_CLASSNOTAVAILABLE;
}
```

### Lite Version Code Structure

```cpp
// Lite version ONLY includes:
// - IconOverlay.cpp (overlay providers)
// - Cache.cpp (SQLite status cache)
// - GitStatus.cpp (minimal Git status queries)
// - ShellExt.cpp (DLL entry points)

// Excluded in Lite:
// - ContextMenu.cpp
// - PropertySheet.cpp
// - DragDrop.cpp
// - IPC.cpp (communication with app)
```

## Icon Overlay Implementation

Windows has a 15-overlay limit system-wide. We use 8:

```cpp
class IconOverlayModified : public IShellIconOverlayIdentifier {
    STDMETHOD(GetOverlayInfo)(LPWSTR pwszIconFile, int cchMax,
                              int* pIndex, DWORD* pdwFlags) {
        // Return path to icon file and index
        wcscpy_s(pwszIconFile, cchMax, g_iconPath);
        *pIndex = ICON_MODIFIED;
        *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
        return S_OK;
    }

    STDMETHOD(IsMemberOf)(LPCWSTR pwszPath, DWORD dwAttrib) {
        // Fast check: is this file modified?
        auto status = GetCachedStatus(pwszPath);
        return (status == GS_STATUS_MODIFIED) ? S_OK : S_FALSE;
    }
};
```

## Context Menu Implementation

```cpp
STDMETHODIMP ContextMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu,
                                           UINT idCmdFirst, UINT idCmdLast,
                                           UINT uFlags) {
    // Don't add menu if not a Git repo
    if (!IsGitRepository(m_path)) {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
    }

    // Create submenu
    HMENU hSubmenu = CreatePopupMenu();
    InsertMenu(hSubmenu, 0, MF_STRING, idCmdFirst + CMD_COMMIT, L"Commit...");
    InsertMenu(hSubmenu, 1, MF_STRING, idCmdFirst + CMD_PUSH, L"Push");
    InsertMenu(hSubmenu, 2, MF_STRING, idCmdFirst + CMD_PULL, L"Pull");

    // Insert into main menu
    MENUITEMINFO mii = { sizeof(mii) };
    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
    mii.wID = idCmdFirst + CMD_MENU;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = L"GitScribe";
    InsertMenuItem(hMenu, indexMenu, TRUE, &mii);

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, CMD_MAX);
}
```

## IPC with Main App

When user clicks "Commit", we launch the Electron app:

```cpp
void LaunchCommitDialog(const wchar_t* repoPath) {
    // Create named pipe message
    std::wstring pipeName = L"\\\\.\\pipe\\gitscribe";
    HANDLE hPipe = CreateFile(pipeName.c_str(), GENERIC_WRITE, ...);

    if (hPipe == INVALID_HANDLE_VALUE) {
        // App not running, launch it
        ShellExecute(NULL, L"open", L"gitscribe.exe",
                     L"--commit", repoPath, SW_SHOW);
    } else {
        // App running, send message
        WriteFile(hPipe, message, size, &written, NULL);
        CloseHandle(hPipe);
    }
}
```

## Registry Keys

Shell extension registers these keys:

```
HKCR\CLSID\{GUID}\
├── InprocServer32\
│   ├── (Default) = "C:\Program Files\GitScribe\gitscribe-shell.dll"
│   └── ThreadingModel = "Apartment"
└── ...

HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\
└── GitScribeModified = {GUID}
```

## Debugging Tips

### Attach to Explorer

1. Build in Debug configuration
2. F5 → Attach to Process
3. Find `explorer.exe` (may be multiple, attach to all)
4. Set breakpoints
5. Trigger action in Explorer

### Force Reload

```cmd
regsvr32 /u /s gitscribe-shell.dll
regsvr32 /s gitscribe-shell.dll
taskkill /f /im explorer.exe
start explorer.exe
```

### Check Registration

```cmd
reg query HKCR\CLSID\{YOUR-GUID}\InprocServer32
```

## Testing Checklist

Before any release:

- [ ] Test on clean Windows install
- [ ] Test with large repos (>10k files)
- [ ] Test with network drives
- [ ] Test with OneDrive/Dropbox
- [ ] Stress test: open/close Explorer repeatedly
- [ ] Memory leak test: let run overnight
- [ ] Crash test: attach debugger, let run for hours

## Common Pitfalls

1. **Don't allocate on heap in hot path** - Use stack or cache
2. **Don't query filesystem in UI thread** - Use cache
3. **Don't assume paths are valid** - Always validate
4. **Don't leak COM objects** - Use smart pointers
5. **Don't ignore HRESULTs** - Check every return value

## Resource Management

```cpp
class ComPtr<T> {
    T* ptr = nullptr;
public:
    ~ComPtr() { if (ptr) ptr->Release(); }
    T** operator&() { return &ptr; }
    T* operator->() { return ptr; }
};

// Usage
ComPtr<IShellFolder> folder;
SHGetDesktopFolder(&folder);
// Auto-released on scope exit
```

## Upgrade Path (Lite → Full)

### Detection
Lite version can show "Upgrade" tooltip when user hovers on folder:

```cpp
#ifdef GITSCRIBE_LITE
STDMETHOD(GetInfoTip)(DWORD dwFlags, LPWSTR* ppwszTip) {
    static int tipCount = 0;
    tipCount++;

    // Show upgrade hint every 20th hover
    if (tipCount % 20 == 0) {
        *ppwszTip = L"💡 Want to commit from Explorer? Upgrade to GitScribe Full";
    }

    return S_OK;
}
#endif
```

### Upgrade Process
1. User downloads GitScribe-Setup.exe
2. Installer detects Lite is installed (registry check)
3. Installer:
   - Unregisters gitscribe-shell-lite.dll
   - Installs gitscribe-shell.dll (same CLSID)
   - Migrates cache database
   - Registers full version
   - Restarts Explorer
4. User now has full version, cache preserved

### No Nagging
**Important**: Lite version should NEVER nag. Only subtle hints:
- Occasional tooltip (1 in 20)
- Small link in property page (Lite only has minimal property sheet)
- No popups, no interruptions

## Future Improvements

- Use Windows Runtime (WinRT) APIs instead of Win32
- Implement background queue for heavy operations
- Add telemetry for performance monitoring (opt-in only)
- Port to C++ modules (C++20)
- Cross-platform shell integration (if GitScribe goes cross-platform)
