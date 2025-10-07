# GitScribe Development Roadmap

## Current Status: Week 1 (Planning Complete)

✅ Repository structure created
✅ Documentation written
✅ Licensing strategy finalized
✅ Landing page reframed
⏭️ Ready to start development

## Phase 0: GitScribe Lite MVP (Weeks 1-4)

**North Star**: Ship a 2MB installer that shows Git status overlay icons in Windows Explorer

### Week 1: Foundation (Current Week)

**Objective**: Get development environment ready and core library skeleton working

#### Day 1-2: Environment Setup
- [ ] Install Rust 1.70+ (stable)
- [ ] Install Visual Studio 2022 with C++ tools
- [ ] Install WiX Toolset 3.14+
- [ ] Install Git for Windows
- [ ] Clone all repositories
- [ ] Verify build tools: `cargo --version`, `msbuild`, `candle`

#### Day 3-4: gitscribe-core Skeleton
```bash
cd gitscribe-core
cargo init --lib
```

**Cargo.toml setup:**
```toml
[package]
name = "gitscribe-core"
version = "0.1.0"
edition = "2021"

[dependencies]
git2 = "0.18"        # libgit2 bindings
rusqlite = "0.30"    # SQLite cache
anyhow = "1.0"       # Error handling
tracing = "0.1"      # Logging

[lib]
crate-type = ["cdylib", "staticlib", "rlib"]

[build-dependencies]
cbindgen = "0.26"    # C header generation
```

**Initial code structure:**
```rust
// src/lib.rs
pub mod repository;
pub mod status;
pub mod cache;
pub mod ffi;

// src/repository.rs
pub struct Repository {
    path: PathBuf,
    inner: git2::Repository,
}

impl Repository {
    pub fn open<P: AsRef<Path>>(path: P) -> Result<Self> {
        // TODO: Week 2
    }
}

// src/status.rs
pub enum FileStatus {
    Modified,
    Added,
    Deleted,
    Ignored,
    Clean,
    Conflicted,
    Untracked,
}

// src/ffi.rs (C API)
#[no_mangle]
pub extern "C" fn gs_repository_open(path: *const c_char) -> *mut Repository {
    // TODO: Week 2
}
```

#### Day 5: First Build
- [ ] `cargo build` succeeds
- [ ] `cargo test` passes (empty tests)
- [ ] C header generated: `include/gitscribe_core.h`

**Success Criteria:**
- ✅ Rust library compiles
- ✅ DLL generated at `target/debug/gitscribe_core.dll`
- ✅ C header exists
- ✅ Can link from test C++ program

---

### Week 2: Core Library Implementation

**Objective**: Implement Git status queries with caching

#### Day 1-2: Git Status Queries

**Implement `Repository::status()`:**
```rust
impl Repository {
    pub fn status(&self) -> Result<Vec<FileStatusEntry>> {
        let mut statuses = Vec::new();
        let opts = git2::StatusOptions::new();

        for entry in self.inner.statuses(Some(&mut opts))?.iter() {
            let path = entry.path().unwrap();
            let status = match entry.status() {
                s if s.is_wt_modified() => FileStatus::Modified,
                s if s.is_wt_new() => FileStatus::Untracked,
                s if s.is_wt_deleted() => FileStatus::Deleted,
                s if s.is_ignored() => FileStatus::Ignored,
                s if s.is_conflicted() => FileStatus::Conflicted,
                _ => FileStatus::Clean,
            };

            statuses.push(FileStatusEntry { path, status });
        }

        Ok(statuses)
    }
}
```

**Test with real repos:**
- [ ] Clone test repos (small, medium, large)
- [ ] Verify status detection works
- [ ] Benchmark: target <100ms for 10k file repo

#### Day 3-4: SQLite Caching

**Schema:**
```sql
CREATE TABLE file_status (
    path TEXT PRIMARY KEY,
    status INTEGER NOT NULL,  -- 0=Clean, 1=Modified, etc.
    mtime INTEGER NOT NULL,   -- File modification time
    cached_at INTEGER NOT NULL -- Cache timestamp
);

CREATE INDEX idx_cached_at ON file_status(cached_at);
```

**Implementation:**
```rust
// src/cache.rs
pub struct StatusCache {
    conn: rusqlite::Connection,
}

impl StatusCache {
    pub fn get(&self, path: &str) -> Option<(FileStatus, SystemTime)> {
        // Query cache
    }

    pub fn set(&mut self, path: &str, status: FileStatus) {
        // Update cache
    }

    pub fn invalidate(&mut self, path: &str) {
        // Remove from cache
    }

    pub fn cleanup(&mut self) {
        // Remove entries older than TTL (5 minutes)
    }
}
```

**Cache integration:**
```rust
impl Repository {
    pub fn status_cached(&self, cache: &mut StatusCache) -> Result<Vec<FileStatusEntry>> {
        // Check cache first
        // Fall back to Git if cache miss
        // Update cache
    }
}
```

#### Day 5: C API

**Generate C bindings:**
```rust
// src/ffi.rs
#[repr(C)]
pub struct GSRepository {
    _private: [u8; 0],
}

#[repr(C)]
pub struct GSFileStatus {
    path: *const c_char,
    status: i32,
}

#[no_mangle]
pub unsafe extern "C" fn gs_repository_open(path: *const c_char) -> *mut GSRepository {
    // Convert C string to Rust
    // Open repository
    // Return opaque pointer
}

#[no_mangle]
pub unsafe extern "C" fn gs_repository_status(repo: *mut GSRepository) -> *mut GSFileStatus {
    // Get status
    // Return array
}

#[no_mangle]
pub unsafe extern "C" fn gs_repository_free(repo: *mut GSRepository) {
    // Drop repository
}
```

**Test from C++:**
```cpp
// test.cpp
#include "gitscribe_core.h"

int main() {
    auto repo = gs_repository_open("C:/repos/test");
    auto status = gs_repository_status(repo);
    // Print results
    gs_repository_free(repo);
}
```

**Success Criteria:**
- ✅ Status queries work on real repos
- ✅ Cache hit rate >90% on repeated queries
- ✅ Performance: <10ms cached, <100ms uncached
- ✅ C API usable from C++

---

### Week 3: Shell Extension (Lite)

**Objective**: Windows overlay icons working

#### Day 1-2: COM Boilerplate

**Create Visual Studio project:**
- DLL project: `gitscribe-shell-lite`
- Add C++20 support
- Link to `gitscribe_core.lib`

**Implement COM basics:**
```cpp
// dllmain.cpp
HINSTANCE g_hInst = nullptr;

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    if (rclsid == CLSID_GitScribeOverlayModified) {
        // Return class factory
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllRegisterServer() {
    // Register overlay handlers
}

STDAPI DllUnregisterServer() {
    // Unregister
}
```

#### Day 3-4: Overlay Icon Handlers

**Implement IShellIconOverlayIdentifier:**
```cpp
// IconOverlayModified.h
class IconOverlayModified : public IShellIconOverlayIdentifier {
private:
    ULONG m_refCount;

public:
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IShellIconOverlayIdentifier
    STDMETHOD(GetOverlayInfo)(LPWSTR, int, int*, DWORD*);
    STDMETHOD(GetPriority)(int*);
    STDMETHOD(IsMemberOf)(LPCWSTR, DWORD);
};

STDMETHODIMP IconOverlayModified::IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib) {
    // Convert path to UTF-8
    char path[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, pwszPath, -1, path, MAX_PATH, nullptr, nullptr);

    // Query Git status via C API
    auto repo = gs_repository_open_from_path(path);
    if (!repo) return S_FALSE;

    auto status = gs_file_status(repo, path);
    gs_repository_free(repo);

    // Return S_OK if modified, S_FALSE otherwise
    return (status == GS_STATUS_MODIFIED) ? S_OK : S_FALSE;
}
```

**Create 8 overlay handlers:**
- GitScribeOverlayModified (yellow dot)
- GitScribeOverlayAdded (green plus)
- GitScribeOverlayDeleted (red minus)
- GitScribeOverlayConflicted (red exclamation)
- GitScribeOverlayIgnored (gray X)
- GitScribeOverlayUntracked (purple question)
- GitScribeOverlayClean (green check)
- GitScribeOverlayLocked (blue lock)

#### Day 5: Icons & Registration

**Design icons:**
- Create 8 PNG icons (16x16, 32x32 for high DPI)
- Simple, clear designs
- Windows 11 aesthetic

**Registry setup:**
```cpp
STDAPI DllRegisterServer() {
    // Register CLSID
    // HKCR\CLSID\{GUID}\InprocServer32

    // Register overlay
    // HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\GitScribeModified

    // Set priority (use spaces to sort before TortoiseGit)
    // "  GitScribeModified" sorts before "1TortoiseModified"
}
```

**Success Criteria:**
- ✅ Shell extension registers without errors
- ✅ Overlay icons appear in Explorer
- ✅ Correct icons for file states
- ✅ No Explorer crashes
- ✅ Performance acceptable (<50ms per query)

---

### Week 4: Polish & Ship

**Objective**: Production-ready installer and public release

#### Day 1-2: Installer (WiX)

**Create WiX project:**
```xml
<!-- Product.wxs -->
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
  <Product Id="*" Name="GitScribe Lite" Version="0.1.0"
           Manufacturer="GitScribe" UpgradeCode="PUT-GUID-HERE">

    <Package InstallerVersion="500" Compressed="yes" />

    <Directory Id="TARGETDIR" Name="SourceDir">
      <Directory Id="ProgramFilesFolder">
        <Directory Id="INSTALLFOLDER" Name="GitScribe Lite">
          <Component Id="CoreDLL">
            <File Source="gitscribe_core.dll" KeyPath="yes" />
          </Component>
          <Component Id="ShellDLL">
            <File Source="gitscribe_shell_lite.dll" KeyPath="yes" />
            <RegistryValue Root="HKLM" Key="SOFTWARE\GitScribe"
                          Name="InstallPath" Value="[INSTALLFOLDER]" Type="string" />
          </Component>
          <Component Id="Icons">
            <File Source="icons\modified.png" />
            <!-- ... other icons ... -->
          </Component>
        </Directory>
      </Directory>
    </Directory>

    <CustomAction Id="RegisterShellExt"
                  ExeCommand="regsvr32.exe /s [INSTALLFOLDER]gitscribe_shell_lite.dll" />

    <InstallExecuteSequence>
      <Custom Action="RegisterShellExt" After="InstallFiles" />
    </InstallExecuteSequence>
  </Product>
</Wix>
```

**Build installer:**
```bash
candle Product.wxs
light Product.wixobj -out GitScribeLite-0.1.0.msi
```

**Test installation:**
- [ ] Install on clean VM
- [ ] Verify files copied
- [ ] Verify registration
- [ ] Verify icons appear
- [ ] Uninstall works cleanly

#### Day 3: Testing Gauntlet

**Test matrix:**
| OS | Repos | Result |
|----|-------|--------|
| Win 10 22H2 | Small (100 files) | ✅ |
| Win 11 23H2 | Medium (1k files) | ✅ |
| Win 11 23H2 | Large (10k files) | ✅ |
| Win 11 23H2 | Huge (100k files) | ✅ |

**Stress tests:**
- [ ] Leave Explorer running overnight
- [ ] Open/close/switch repos rapidly
- [ ] Network drive repos
- [ ] OneDrive/Dropbox synced repos
- [ ] Submodules
- [ ] Multiple monitors

**Performance profiling:**
- Use Process Monitor to track IsMemberOf calls
- Target: <50ms per query average

#### Day 4: Documentation

**Write docs:**
- [ ] Installation guide (README in installer)
- [ ] Troubleshooting (icons not showing)
- [ ] Known limitations
- [ ] How to upgrade to full version

**Landing page update:**
- [ ] Add download link
- [ ] Add screenshots
- [ ] Add "What's Lite?" section

#### Day 5: Launch! 🚀

**Pre-launch checklist:**
- [ ] GitHub release with installer
- [ ] Tag version 0.1.0
- [ ] Update landing page
- [ ] Prepare announcement posts

**Launch:**
- [ ] Post on r/programming
- [ ] Post on Hacker News
- [ ] Tweet announcement
- [ ] Submit to winget repository

**Announcement template:**
```
GitScribe Lite - Free overlay icons for Git in Windows Explorer

Tired of TortoiseGit crashes? Want just the visual status without all the bloat?

GitScribe Lite is a 2MB installer that adds overlay icons to Windows Explorer,
showing Git status at a glance. No context menus, no dialogs, no Electron app.
Just fast, reliable status indicators.

Built in Rust for stability. Free forever.

Download: https://gitscribe.dev/lite
GitHub: https://github.com/gitscribe/gitscribe-lite
```

**Success Metrics (Week 1 after launch):**
- [ ] 100+ downloads
- [ ] <5 bug reports
- [ ] >80% positive feedback
- [ ] 0 reports of Explorer crashes

---

## Phase 1: Full Shell Integration (Weeks 5-8)

*To be detailed after Lite ships*

## Phase 2: Electron App (Weeks 9-16)

*To be detailed after Phase 1 ships*

---

## Current Next Steps

1. Set up development environment
2. Initialize gitscribe-core Rust project
3. Implement basic Repository struct
4. Get first build working

**Ready to start Week 1, Day 1? Let's go! 🚀**
