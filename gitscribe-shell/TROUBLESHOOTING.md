# GitScribe Status - Troubleshooting Guide

## Overlay Icons Not Showing

### Problem: No icons appear at all

**Likely cause**: Windows has a limit of 15 overlay icon handlers, and other applications may have used them all.

**Solution 1: Check overlay registration**

1. Press `Win+R` and type: `regedit`
2. Navigate to: `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers`
3. Look for entries starting with **" GitScribe"** (note the leading space)
4. If missing, reinstall GitScribe Status

**Solution 2: Increase overlay priority**

Windows loads overlay handlers alphabetically. GitScribe uses a leading space to appear first, but other apps (like OneDrive, Dropbox) may conflict.

1. Open Registry Editor (as above)
2. Rename unwanted overlay handlers by adding `zzz` prefix:
   - `OneDrive1` → `zzzOneDrive1`
   - `DropboxExt1` → `zzzDropboxExt1`
3. Restart Explorer: `taskkill /f /im explorer.exe && start explorer.exe`

**Solution 3: Restart Explorer**

Sometimes Explorer needs a full restart:

```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

Or log out and back in.

### Problem: Icons show for some files but not others

**Likely cause**: Windows caches icon overlays aggressively.

**Solution: Clear icon cache**

```cmd
taskkill /f /im explorer.exe
del /f /s /q /a %LocalAppData%\IconCache.db
start explorer.exe
```

Or reboot your computer.

### Problem: Icons disappeared after Windows update

**Likely cause**: Windows updates sometimes unregister shell extensions.

**Solution: Re-register the extension**

Run as Administrator:

```cmd
cd "C:\Program Files\GitScribe"
register.cmd
```

Then restart Explorer.

## Performance Issues

### Problem: Explorer feels slow after installing

**Likely cause**: Large repositories with many files.

**Solution 1: Check repository size**

GitScribe Status works best with repos under 50,000 files. For larger repos:

1. Use `.gitignore` to exclude large directories (`node_modules`, `build`, etc.)
2. Consider using Git sparse-checkout
3. Disable GitScribe in that specific repo (coming in GitScribe Full)

**Solution 2: Check for conflicts with other shell extensions**

Disable other Git tools temporarily to test:

- TortoiseGit
- GitHub Desktop
- GitKraken
- SourceTree

**Solution 3: Enable performance mode** (coming soon)

This will cache status for longer (5 minutes instead of 1 second).

### Problem: High CPU usage

**Likely cause**: Filesystem watcher conflicts.

**Solution: Disable conflicting watchers**

1. Check if you have multiple Git tools running
2. Disable OneDrive/Dropbox in your Git repos (they conflict with `.git` folder monitoring)
3. Close IDEs like VS Code that also watch files

## Installation Issues

### Problem: "Access denied" during installation

**Solution**: Right-click installer → "Run as administrator"

### Problem: "DLL registration failed"

**Likely cause**: Antivirus blocking shell extension registration.

**Solution**:

1. Temporarily disable antivirus
2. Reinstall GitScribe Status
3. Re-enable antivirus
4. Add `C:\Program Files\GitScribe` to antivirus exclusions

### Problem: "VCRUNTIME140.dll not found"

**Solution**: Install Microsoft Visual C++ Redistributable

Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe

### Problem: Installation succeeds but nothing happens

**Solution**: Restart Explorer or reboot

```cmd
shutdown /r /t 0
```

## Repository Detection Issues

### Problem: Icons don't show in my Git repository

**Verify it's a Git repository**:

```cmd
cd C:\path\to\repo
git status
```

If you get `fatal: not a git repository`, you need to initialize Git:

```cmd
git init
```

### Problem: Submodules not detected

**Known limitation**: GitScribe Status v0.1.0 has basic submodule support. Full support coming in v0.2.0.

**Workaround**: Navigate directly into submodule folders.

### Problem: Network drives not working

**Likely cause**: Windows doesn't allow shell extensions on network drives by default.

**Solution**: Enable remote shell extensions (use caution, security risk)

1. Open Registry Editor
2. Navigate to: `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System`
3. Create DWORD: `EnableLinkedConnections` = `1`
4. Reboot

**Warning**: Only enable this if you trust the network drive's security.

## Conflicts with Other Git Tools

### Problem: TortoiseGit and GitScribe both installed

**Solution**: Both can coexist, but you may hit the 15-overlay limit.

**Option 1**: Disable TortoiseGit overlays in TortoiseGit Settings

**Option 2**: Uninstall TortoiseGit (GitScribe is better 😉)

### Problem: GitHub Desktop conflicts

**GitHub Desktop doesn't use shell overlays**, so there's no conflict. You can use both together.

### Problem: GitKraken conflicts

**GitKraken doesn't use shell overlays**, so there's no conflict.

## Icon Theme Issues

### Problem: Can't change icon theme

**Known limitation**: Icon theme selection requires **GitScribe Full** (coming soon).

**Current workaround**: Manually replace icon files in `C:\Program Files\GitScribe\icons\` (requires Administrator).

**Better solution**: Wait for GitScribe Full release 😊

## Debugging

### Enable Verbose Logging

1. Set environment variable:
   ```cmd
   setx GITSCRIBE_DEBUG 1
   ```
2. Restart Explorer
3. Download [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview)
4. Run DebugView as Administrator
5. Look for `[GitScribe]` entries

### Check DLL Registration

```cmd
reg query HKCR\CLSID\{F8C3F5A0-1234-4567-89AB-123456789ABC}\InprocServer32
```

(Replace GUID with actual GitScribe CLSID from `register.cmd`)

### Verify Files Installed

Check that these exist:

- `C:\Program Files\GitScribe\GitScribeShell.dll`
- `C:\Program Files\GitScribe\gitscribe_core.dll`
- `C:\Program Files\GitScribe\icons\classic\*.ico`

## Still Not Working?

### Before reporting a bug, please collect:

1. **Windows version**: `winver`
2. **GitScribe version**: Check in "Programs and Features"
3. **Git version**: `git --version`
4. **DebugView output**: (if you enabled verbose logging)
5. **Screenshot** of the issue

### Report the bug:

- **GitHub Issues**: https://github.com/KyleEdwardDonaldson/GitScribe/issues
- **Email**: support@gitscribe.dev

Include "GitScribe Status v0.1.0" in the subject line.

## Known Limitations (To Be Fixed)

- ❌ No icon theme selector (requires GitScribe Full)
- ❌ Basic submodule support (full support in v0.2.0)
- ❌ No performance tuning UI (coming in v0.2.0)
- ❌ No per-repo disable (coming in GitScribe Full)
- ❌ Limited large-repo optimization (>100k files may be slow)

## Workarounds for Large Repositories

If you work with massive repos (e.g., Windows source, Chromium):

1. Use `git sparse-checkout` to reduce working tree size
2. Exclude build directories in `.gitignore`
3. Consider using GitScribe only on smaller projects
4. Wait for **GitScribe Pro** which includes advanced caching

## FAQ

### Q: Can I use this with WSL (Windows Subsystem for Linux)?

**A**: Not yet. GitScribe Status requires a Windows-native Git repository. WSL support planned for v1.0.

### Q: Does this work with Git LFS?

**A**: Yes! Git LFS files show the same status as regular files.

### Q: Does this slow down my computer?

**A**: GitScribe Status uses <50MB of RAM and has minimal CPU impact. It only activates when you open Explorer in a Git repo.

### Q: Is my data private?

**A**: **100% private.** GitScribe Status runs entirely locally. No telemetry, no phone home, no analytics. Open source, so you can verify.

### Q: Can I use this commercially?

**A**: **Yes, 100% free for commercial use.** GitScribe Status is MPL-2.0 licensed. No restrictions, no license fees, forever.

---

**Still stuck?** Email support@gitscribe.dev or join our Discord (coming soon).
