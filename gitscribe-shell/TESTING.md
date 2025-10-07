# GitScribe Shell Extension - Testing Guide

## Prerequisites

- Windows 10/11 (64-bit)
- Administrator privileges
- Git installed and in PATH
- A Git repository for testing

## Build the Extension

```cmd
cd C:\R\GitScribe\gitscribe-shell
cmake -B build
cmake --build build --config Debug
```

This will create:
- `build/bin/Debug/GitScribeShell.dll` - The shell extension
- `build/bin/Debug/gitscribe_core.dll` - The Rust core library
- `build/resources/icon-packs/` - Icon overlay resources

## Register the Extension

**IMPORTANT: Must run as Administrator**

```cmd
cd C:\R\GitScribe\gitscribe-shell
Right-click register.cmd → "Run as administrator"
```

This will:
1. Register the COM server with Windows
2. Kill and restart Explorer.exe to load the extension

## Verify Registration

Check that the extension is registered:

```cmd
reg query "HKCR\CLSID\{GUID}\InprocServer32"
```

Replace `{GUID}` with the actual CLSIDs from `src/resource.h`:
- `CLSID_ContextMenu` - Context menu handler
- `CLSID_GitScribeOverlayModified` - Modified file overlay
- `CLSID_GitScribeOverlayAdded` - Added file overlay
- etc.

## Test Overlay Icons

1. Open Windows Explorer
2. Navigate to a Git repository
3. You should see overlay icons on files:
   - **Modified files**: Different icon overlay
   - **Added files**: Plus icon overlay
   - **Untracked files**: Question mark overlay
   - **Conflicted files**: Warning icon overlay

### Troubleshooting Overlays

**Icons not showing?**

1. Check icon file paths exist:
   ```cmd
   dir build\resources\icon-packs\default\*.ico
   ```

2. Restart Explorer again:
   ```cmd
   taskkill /f /im explorer.exe && start explorer.exe
   ```

3. Check Windows overlay limit (max 15 system-wide):
   ```cmd
   reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers"
   ```

4. GitScribe overlays should appear near the top (alphabetically) to win priority

**Wrong icons showing?**

- Check `GetOverlayInfo()` in `GitScribeOverlay.cpp` returns correct paths
- Verify icon files are valid .ico format
- Use DebugView from Sysinternals to see debug output

## Test Context Menus

1. **Right-click on a modified file**
   - Should see "GitScribe" submenu
   - Menu should show:
     - 📝 Commit "filename"...
     - 📊 Diff with HEAD
     - ↩️ Revert changes...
     - Separator
     - 📜 Show History
     - 🏷️ Blame
     - Separator
     - GitScribe Settings...

2. **Right-click on an untracked file**
   - Should see:
     - ➕ Add "filename" to Git
     - 🚫 Ignore "filename"

3. **Right-click on repository folder**
   - Menu adapts based on repo state
   - Shows branch name and status

4. **Right-click on multiple selected files**
   - Shows multi-selection menu
   - Batch operations available

### Test Command Handlers

Click each menu item:

- **Commit**: Should either:
  - Launch GitScribe app (if installed), OR
  - Fall back to `git status` in terminal window

- **Diff**: Should either:
  - Show diff in GitScribe app, OR
  - Fall back to `git diff` in terminal

- **Push**: Should:
  - Show confirmation dialog with commit count
  - Execute `git push` in terminal

- **Pull**: Should:
  - Show confirmation dialog
  - Execute `git pull` in terminal

### Test Context Detection

The menu should adapt to these scenarios:

1. **FileModified**: Modified file selected
2. **FileUntracked**: Untracked file selected
3. **FileConflicted**: Conflicted file during merge
4. **RepoDirty**: Repository folder with uncommitted changes
5. **RepoAhead**: Repository with commits to push
6. **RepoBehind**: Repository with commits to pull
7. **RepoClean**: Clean repository
8. **MergeInProgress**: During active merge
9. **MultiSelection**: Multiple files selected

## Test IPC (App Communication)

If GitScribe app is installed:

1. Right-click file → Commit
2. Check that named pipe exists:
   ```cmd
   powershell "Get-ChildItem \\.\pipe\ | Where-Object {$_.Name -like '*GitScribe*'}"
   ```

3. Verify app receives JSON-RPC message
4. App should show commit dialog with selected file

## Performance Testing

### Small Repository (< 1,000 files)

```cmd
cd C:\R\GitScribe\gitscribe-shell
cmake --build build --config Release
```

1. Right-click file
2. Menu should appear in < 50ms
3. No Explorer lag or freezing

### Large Repository (> 10,000 files)

Test with Linux kernel or Chromium:

```cmd
git clone --depth 1 https://github.com/torvalds/linux.git
cd linux
```

1. Right-click file
2. Menu should still appear in < 150ms
3. Check memory usage in Task Manager
4. Shell extension should use < 50MB

### Stress Test

1. Open 10 Explorer windows in same repository
2. Right-click files rapidly
3. No crashes, no deadlocks
4. Overlay icons update correctly

## Memory Leak Testing

1. Install DebugDiag or UMDH
2. Attach to `explorer.exe`
3. Right-click files 1000+ times
4. Check for memory growth
5. Acceptable: < 1MB per 1000 operations

## Debugging

### Enable Debug Output

Build in Debug configuration (already default).

### View Debug Messages

1. Download [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview)
2. Run as Administrator
3. Capture → Capture Global Win32
4. Look for "[GitScribe]" messages

### Attach Debugger

1. Build in Debug: `cmake --build build --config Debug`
2. Visual Studio → Debug → Attach to Process
3. Find all `explorer.exe` processes (may be multiple)
4. Attach to all
5. Set breakpoints in shell extension code
6. Right-click file in Explorer to hit breakpoint

### Common Issues

**Issue**: "The module 'GitScribeShell.dll' failed to load"
- **Cause**: Missing gitscribe_core.dll dependency
- **Fix**: Copy gitscribe_core.dll to same directory as GitScribeShell.dll

**Issue**: Context menu shows but clicks do nothing
- **Cause**: `InvokeCommand()` not handling command IDs correctly
- **Fix**: Check `ContextMenu::InvokeCommand()` switch statement

**Issue**: Icons show on wrong files
- **Cause**: `IsMemberOf()` returning incorrect status
- **Fix**: Debug `GitScribeOverlay::IsFileStatus()` logic

**Issue**: Explorer crashes when right-clicking
- **Cause**: Unhandled exception in shell extension
- **Fix**: Add try-catch in all COM methods, check for null pointers

## Unregister (Cleanup)

To remove the shell extension:

```cmd
Right-click unregister.cmd → "Run as administrator"
```

This will:
1. Unregister all COM components
2. Restart Explorer
3. Remove all registry entries

## Test Checklist

Use this checklist for release testing:

- [ ] Build completes without errors
- [ ] Registration succeeds
- [ ] Overlay icons appear on modified files
- [ ] Overlay icons appear on added files
- [ ] Overlay icons appear on untracked files
- [ ] Context menu appears on files
- [ ] Context menu appears on folders
- [ ] Context menu shows correct items for file state
- [ ] Commit command works (app or fallback)
- [ ] Diff command works (app or fallback)
- [ ] Push command shows confirmation
- [ ] Pull command shows confirmation
- [ ] No Explorer crashes during normal use
- [ ] No Explorer lag with large repositories
- [ ] Memory usage stays stable over time
- [ ] Unregistration removes all components
- [ ] No registry keys left after uninstall

## Next Steps

Once basic testing passes:

1. **Test on clean Windows VM** - Ensure no dev dependencies
2. **Test with different Git configs** - SSH vs HTTPS, different remotes
3. **Test edge cases** - Submodules, worktrees, bare repos
4. **Performance profiling** - Identify bottlenecks
5. **Build Release configuration** - Test optimized build
6. **Create installer** - WiX or NSIS package
7. **Beta release** - Get real user feedback

## Reporting Issues

If you find bugs:

1. Capture DebugView output
2. Note exact steps to reproduce
3. Include repository state (branch, status)
4. Check Event Viewer for crashes
5. Report to: https://github.com/[org]/gitscribe/issues
