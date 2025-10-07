# Changelog

All notable changes to GitScribe products will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Product Versioning

GitScribe uses independent version numbers for each product:

- **GitScribe Status** - Lightweight shell extension (status display + overlays)
- **GitScribe Full** - Complete GUI client (all features)
- **gitscribe-core** - Rust library (used by both products)

Each product has its own release cycle and changelog section below.

---

## GitScribe Status

### [0.1.0] - 2025-10-07

#### Added
- **Overlay icons** for all Git file states (modified, added, deleted, ignored, clean, conflicted)
- **Context menu** showing repository status: "GitScribe | Clean", "GitScribe | Modified", etc.
- **Status detection** for repository states: Clean, Modified, Merging, Rebasing, Cherry-Picking, Reverting, Bisecting
- **Click-to-copy** file path functionality when clicking context menu item
- **Fast caching** using SQLite for <50ms status updates
- **Standalone operation** - no app dependency required

#### Technical Details
- Built with Rust core (`gitscribe-core`) + C++ shell extension
- Uses libgit2 for Git operations
- ~2MB installer with minimal dependencies
- Windows 10/11 x64 support
- CMake build system with `GITSCRIBE_STATUS` flag for Status-only builds

#### Known Limitations
- No operations menu (commit, push, pull) - status display only
- No property sheet tab
- Context menu is a single item (not a submenu)

#### Build
```powershell
cd gitscribe-shell
.\build-status.ps1
```

---

## GitScribe Full

### [Unreleased]

The full GitScribe client with GUI operations is currently in development.

**Planned Features:**
- Full context menu with operations (commit, push, pull, diff, etc.)
- Beautiful Tauri/Electron app with diff viewer
- Advanced Git operations (rebase, cherry-pick, stash)
- Property sheet tab in Windows Explorer
- Repository management features
- Team collaboration features

---

## gitscribe-core (Rust Library)

### [0.1.0] - 2025-10-07

#### Added
- Repository status queries via libgit2
- File status detection (modified, added, deleted, ignored, conflicted, clean)
- Repository state detection (merging, rebasing, cherry-picking, reverting, bisecting)
- Branch and remote tracking (ahead/behind counts)
- C FFI for shell extension integration
- N-API bindings for Node.js/Electron integration (for future Full version)
- SQLite caching layer for performance
- Async operations support

#### Technical
- Built with Rust 1.70+
- Uses libgit2-sys for Git operations
- Exposes C API via cbindgen
- DLL output for Windows shell extension
- Operation queue management to prevent locks

---

## Links

- **Website:** https://gitscri.be
- **Changelog (Web):** https://gitscri.be/changelog
- **GitHub Releases:** https://github.com/KyleEdwardDonaldson/GitScribe/releases
- **Source Code:** https://github.com/KyleEdwardDonaldson/GitScribe

---

## Release Workflow

When releasing a new version:

1. Update this CHANGELOG.md in the main monorepo
2. Update `gitscribe-landing/src/pages/changelog.astro` with web changelog
3. Create GitHub release with tag format:
   - GitScribe Status: `status-v0.1.0`
   - GitScribe Full: `full-v1.0.0`
   - gitscribe-core: `core-v0.1.0`
4. Build installer and attach to GitHub release
5. Commit all changes: `git add -A && git commit -m "Release: GitScribe Status v0.1.0"`
6. Push all repos: main monorepo, gitscribe-landing, gitscribe-app (if changes)

---

**Maintained by:** Kyle Donaldson (kyle@gitscri.be)
