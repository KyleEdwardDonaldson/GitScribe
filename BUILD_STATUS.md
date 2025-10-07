# Building GitScribe Status v0.1

This guide explains how to build **GitScribe Status** - the lightweight version with overlay icons and simple context menu.

## What is GitScribe Status?

**GitScribe Status** is a free, minimal release that includes:
- ✅ Overlay icons showing file status (modified, added, deleted, ignored, clean)
- ✅ Context menu displaying "GitScribe | Clean" (or other statuses)
- ✅ Clicking the menu item copies the file path to clipboard
- ❌ No submenu operations (commit, push, pull, etc.)
- ❌ No property sheet tab
- ❌ No dependency on gitscribe-app

**Perfect for:**
- Developers who want visual Git indicators without a heavy app
- Quick status checks at a glance
- Minimal resource usage (~2MB installer)

## Build Requirements

- **Windows 10/11**
- **Visual Studio 2022** (or 2019 with C++ Desktop Development workload)
- **Rust 1.70+** (install from https://rustup.rs)
- **CMake 3.20+** (install from https://cmake.org or via Visual Studio)

## Building

### Option 1: Using PowerShell Script (Recommended)

```powershell
cd gitscribe-shell
.\build-status.ps1
```

This will:
1. Build `gitscribe-core` (Rust library)
2. Configure CMake with `-DGITSCRIBE_STATUS=ON`
3. Build the shell extension DLL
4. Output to `build-status/bin/Release/`

### Option 2: Manual Build

```powershell
# 1. Build Rust core
cd gitscribe-core
cargo build --release
cd ..

# 2. Configure CMake for Status version
cd gitscribe-shell
mkdir build-status
cd build-status
cmake .. -G "Visual Studio 17 2022" -A x64 -DGITSCRIBE_STATUS=ON

# 3. Build with MSBuild
cmake --build . --config Release
```

## Installation

After building:

```powershell
cd build-status/bin/Release

# Register the shell extension (requires admin)
regsvr32 GitScribeShell.dll

# Restart Windows Explorer
taskkill /f /im explorer.exe
start explorer.exe
```

## Uninstallation

```powershell
# Unregister (requires admin)
regsvr32 /u GitScribeShell.dll

# Restart Explorer
taskkill /f /im explorer.exe
start explorer.exe
```

## What's Different from Full Version?

| Feature | GitScribe Status | GitScribe Full |
|---------|-----------------|----------------|
| Overlay icons | ✅ Yes | ✅ Yes |
| Context menu | ✅ Single item (shows status) | ✅ Full submenu |
| Click action | Copies file path | Opens operations menu |
| Property sheet tab | ❌ No | ✅ Yes |
| Requires app | ❌ No | ✅ Yes (gitscribe-app) |
| Size | ~2MB | ~50MB+ |

## Technical Details

### Preprocessor Flags

The Status version is built with the `GITSCRIBE_STATUS` preprocessor flag defined.

**In Code:**
```cpp
#ifdef GITSCRIBE_STATUS
    // Status version: Single menu item
    InsertMenuItemW(hMenu, insertPos, TRUE, &mii);
#else
    // Full version: Submenu with operations
    CreatePopupMenu();
    // ...
#endif
```

### Excluded Components

When building with `-DGITSCRIBE_STATUS=ON`:
- `AppLauncher.cpp` is excluded (no IPC with app)
- `PropertySheet.cpp` is excluded (no property sheet)
- Property sheet registration is skipped in `DllRegisterServer()`

### Registry Keys

GitScribe Status registers:
- **Overlay handlers:** `HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\`
  - ` GitScribeModified`
  - ` GitScribeClean`
  - ` GitScribeAdded`
  - ` GitScribeUntracked`
  - ` GitScribeConflicted`
  - ` GitScribeIgnored`
- **Context menu handler:** `HKCR\*\shellex\ContextMenuHandlers\  GitScribe`

## Troubleshooting

### Overlays not showing?

1. Windows has a 15-overlay limit. Check existing overlays:
   ```powershell
   reg query "HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers"
   ```

2. GitScribe uses space prefix (` GitScribe*`) for alphabetical priority.

3. Restart Explorer: `taskkill /f /im explorer.exe && start explorer.exe`

### Context menu not appearing?

1. Verify registration:
   ```powershell
   reg query "HKCR\*\shellex\ContextMenuHandlers\  GitScribe"
   ```

2. Check DLL is in PATH or same directory as registry points to

3. Restart Explorer

### Build errors?

1. Ensure Rust is in PATH: `cargo --version`
2. Ensure CMake is in PATH: `cmake --version`
3. Ensure Visual Studio 2022 is installed with C++ tools
4. Try building gitscribe-core first: `cd gitscribe-core && cargo build --release`

## Next Steps

After building GitScribe Status:
1. Test in a Git repository
2. Verify overlay icons appear on files
3. Right-click a file and see "GitScribe | Clean" (or other status)
4. Click the menu item - file path should be copied to clipboard
5. Consider packaging into an installer (see `installer/` directory)

## Upgrading to GitScribe Full

To build the full version with all features:
```powershell
cd gitscribe-shell
.\build-full.ps1
```

This builds without the `-DGITSCRIBE_STATUS` flag, including:
- Full context menu with submenu operations
- Property sheet tab in Properties dialog
- IPC integration with gitscribe-app
- AppLauncher for launching the Electron/Tauri app

---

**Need help?** Open an issue at https://github.com/KyleEdwardDonaldson/GitScribe/issues
