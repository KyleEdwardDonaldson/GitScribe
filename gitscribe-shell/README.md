# GitScribe Status - Windows Git Overlay Icons

**See your Git status at a glance** — Visual indicators for modified, added, deleted, and ignored files right in Windows Explorer.

![License](https://img.shields.io/badge/license-MPL--2.0-blue)
![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue)
![Status](https://img.shields.io/badge/status-v0.1.0%20Ready-green)

## What Is This?

GitScribe Status adds **overlay icons** to Windows Explorer for Git repositories. No context menus, no dialogs, no apps—just visual status indicators.

**Perfect for users who:**
- Just want to see file status without opening Git tools
- Use command-line Git but want visual feedback
- Have lightweight performance requirements
- Don't need full Git GUI features

## Features

### Overlay Icons

- 📝 **Modified** - File has uncommitted changes
- ➕ **Added** - New file staged for commit
- 🗑️ **Deleted** - File deleted or removed
- ❓ **Untracked** - File not tracked by Git
- ⚠️ **Conflicted** - File has merge conflicts
- 🚫 **Ignored** - File ignored by .gitignore
- ✓ **Clean** - File tracked with no changes

### Performance

- ⚡ **<50ms** icon updates (cached)
- 🪶 **<50MB** RAM usage
- 🚀 **SQLite caching** for speed
- 🔄 **Background updates** — never blocks Explorer

### Icon Themes

10 professional themes included:
- Classic, Minimalist, Bold, Monochrome, Neon
- Pastel, Corporate, Emoji, Flat, Glass

*(Theme selection requires GitScribe Full - coming soon)*

## Installation

### Using winget (Recommended)

```powershell
winget install GitScribe.Status
```

### Manual Install

1. Download from [gitscribe.dev](https://gitscribe.dev) or [GitHub Releases](https://github.com/KyleEdwardDonaldson/GitScribe/releases)
2. Run `GitScribe-Status-Setup.exe`
3. Restart Windows Explorer

**Full guide:** See [INSTALL.md](./INSTALL.md)

## Documentation

- 📘 **[Installation Guide](./INSTALL.md)** - Detailed setup instructions
- 🔧 **[Troubleshooting](./TROUBLESHOOTING.md)** - Fix common issues
- 🧪 **[Testing Guide](./TESTING.md)** - For developers/contributors
- 🏗️ **[Architecture](./CLAUDE.md)** - Technical details

## Building from Source

### Prerequisites

- Windows 10/11
- Visual Studio 2022 (with C++ desktop development)
- CMake 3.20+
- Rust 1.70+ (for core library)

### Build Steps

```bash
# Build core library
cd gitscribe-core
cargo build --release

# Build shell extension
cd ../gitscribe-shell
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release

# Register (run as Administrator)
cd ..
register.cmd
```

### Unregister

```cmd
unregister.cmd  (Run as Administrator)
```

## Want More Features?

**GitScribe Full** (coming soon) adds:
- Context menu operations (Commit, Push, Pull, Diff)
- Beautiful commit dialogs with Monaco editor
- Branch management and merge tools
- Repository history viewer
- Conflict resolution UI

**Still 100% free for personal use!**

Join the waitlist: [gitscribe.dev](https://gitscribe.dev)

## Compatibility

### Works With
- ✅ Git for Windows
- ✅ GitHub Desktop
- ✅ GitKraken
- ✅ VS Code, Visual Studio
- ✅ Git LFS
- ✅ Multiple Git tools simultaneously

### Limitations
- ❌ WSL repositories (planned for v1.0)
- ❌ Network drives (requires registry edit, not recommended)
- ⚠️ Large repos (>100k files) may be slow

## Troubleshooting

**Icons not showing?**
1. Restart Explorer: `taskkill /f /im explorer.exe && start explorer.exe`
2. Check registry: `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers`
3. See [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)

**Performance issues?**
- Exclude large directories in `.gitignore` (e.g., `node_modules`, `build`)
- Check for conflicts with TortoiseGit overlays
- See [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)

## Contributing

We welcome contributions!

**Areas we need help:**
- Icon themes (submit your designs!)
- Performance testing on large repos
- Windows 11 testing
- Documentation improvements
- Bug reports

See the main [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## License

**Mozilla Public License 2.0 (MPL-2.0)**

- ✅ Free for commercial use
- ✅ Modify and distribute
- ✅ No attribution required (but appreciated!)
- ℹ️ Must share modifications to MPL-2.0 code

## Support

- 🐛 **Bug reports**: [GitHub Issues](https://github.com/KyleEdwardDonaldson/GitScribe/issues)
- 💬 **Questions**: support@gitscribe.dev
- 📚 **Docs**: [gitscribe.dev/docs](https://gitscribe.dev/docs)

---

**Made with ❤️ for the Windows developer community**
