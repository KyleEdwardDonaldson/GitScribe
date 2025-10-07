# GitScribe Status - Installation Guide

## What is GitScribe Status?

GitScribe Status adds visual Git status indicators (overlay icons) to Windows Explorer. See at a glance which files are modified, added, deleted, or ignored—all without opening any Git tools.

**100% Free Forever** | **2MB Install** | **No Account Required**

## System Requirements

- **Windows 10** (version 1809 or later) or **Windows 11**
- **Administrator access** (required for shell extension registration)
- **Git for Windows** (optional, but recommended)

## Installation

### Option 1: Using winget (Recommended)

Open PowerShell or Command Prompt and run:

```powershell
winget install GitScribe.Status
```

That's it! Restart Explorer or log out/in to see the icons.

### Option 2: Manual Installation

1. **Download** the installer from [gitscribe.dev](https://gitscribe.dev) or [GitHub Releases](https://github.com/KyleEdwardDonaldson/GitScribe/releases)
2. **Run** `GitScribe-Status-Setup.exe`
3. **Grant** administrator permission when prompted
4. **Wait** for installation to complete (10-15 seconds)
5. **Restart** Windows Explorer:
   - Press `Ctrl+Shift+Esc` to open Task Manager
   - Find "Windows Explorer" in the list
   - Right-click → "Restart"

### Option 3: Developer Build (From Source)

```bash
# Clone the repository
git clone https://github.com/KyleEdwardDonaldson/GitScribe.git
cd GitScribe

# Build the core library
cd gitscribe-core
cargo build --release

# Build the shell extension
cd ../gitscribe-shell
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release

# Register the extension (run as Administrator)
cd ..
register.cmd
```

## Verifying Installation

1. Open Windows Explorer
2. Navigate to any Git repository
3. You should see overlay icons on files:
   - 📝 **Modified** - File has uncommitted changes
   - ➕ **Added** - New file staged for commit
   - 🗑️ **Deleted** - File deleted or removed
   - ❓ **Untracked** - File not tracked by Git
   - ⚠️ **Conflicted** - File has merge conflicts
   - 🚫 **Ignored** - File ignored by .gitignore
   - ✓ **Clean** - File tracked with no changes

## Choosing Your Icon Theme

GitScribe Status includes 10 professionally designed icon themes:

| Theme | Style | Best For |
|-------|-------|----------|
| **Classic** (Default) | Traditional Git colors | All users |
| **Minimalist** | Clean, subtle indicators | Professional environments |
| **Bold** | High contrast, vibrant | High-visibility needs |
| **Monochrome** | Grayscale only | Accessibility |
| **Neon** | Bright, modern | Dark mode users |
| **Pastel** | Soft, gentle colors | Reduced eye strain |
| **Corporate** | Conservative blue/gray | Enterprise settings |
| **Emoji** | Full-color emoji style | Fun projects |
| **Flat** | Material Design style | Modern Windows 11 |
| **Glass** | Translucent effects | Aero glass fans |

**To change themes** (requires GitScribe Full - coming soon):
- Right-click on any Git folder → GitScribe → Settings → Icon Theme

## Uninstalling

### Via Settings (Windows 10/11)

1. Open **Settings** → **Apps** → **Installed apps**
2. Find **GitScribe Status**
3. Click **Uninstall**

### Via winget

```powershell
winget uninstall GitScribe.Status
```

### Manual Uninstall

Run as Administrator:

```cmd
cd "C:\Program Files\GitScribe"
unregister.cmd
```

Then delete the installation folder.

## Upgrading to GitScribe Full

Want more than just icons? **GitScribe Full** (coming soon) adds:

- ✅ Context menu operations (Commit, Push, Pull, Diff)
- ✅ Beautiful commit dialogs with diff viewer
- ✅ Branch management
- ✅ Conflict resolution tools
- ✅ Repository history viewer

**GitScribe Full is free for personal use!**

Visit [gitscribe.dev](https://gitscribe.dev) to join the waitlist.

## Next Steps

- **Troubleshooting?** See [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)
- **Report a bug?** [Open an issue](https://github.com/KyleEdwardDonaldson/GitScribe/issues)
- **Join the community** (Discord coming soon)

---

**Questions?** Email support@gitscribe.dev
