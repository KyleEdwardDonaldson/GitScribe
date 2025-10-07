# GitScribe Context Menu - Project Blueprint

## Vision Statement

The GitScribe context menu is the gateway to Git productivity in Windows Explorer. It's the first touch point that transforms file management into version control mastery, providing instant Git operations without leaving your workflow. Working in perfect harmony with GitScribe Status overlays and the GitScribe app, it creates a seamless trinity of visual feedback, contextual actions, and powerful operations.

## Core Philosophy

### Speed Above All
- **Sub-20ms rule**: Menu must appear instantly (<20ms generation time)
- **Predictive caching**: Pre-cache likely menu states before right-click
- **Non-blocking operations**: Never freeze Explorer, even on massive repos
- **Quick Actions integration**: Same operations available via Win+G globally

### Progressive Power
- **Context is king**: Menu adapts to what you clicked, not generic
- **Show state, not just actions**: "↑ Push 3 commits" not just "Push"
- **Smart defaults**: Most likely action always first
- **Power user shortcuts**: Every action has a keyboard shortcut

### Native Integration Trinity
- **Status provides state**: Overlay icons show what's happening
- **Context menu provides actions**: Right-click to act on that state
- **App provides depth**: Complex operations launch app to exact context
- **Quick Actions provide speed**: Same operations from anywhere via Win+G

## The Integration Vision

### How the Three Components Work Together

```
GitScribe Status (Free Forever)
    ↓ Shows file state via overlays
    ↓ User sees modified file
    ↓
GitScribe Context Menu (Shell Extension)
    ↓ Right-click for actions
    ↓ "Commit this file..."
    ↓
GitScribe App (When Needed)
    ↓ Opens to exact context
    ↓ Shows diff of that specific file
    ↓ Commit dialog pre-filled
```

### Shared Intelligence

All three components share:
- **SQLite cache**: Same status data, instant everywhere
- **Operation queue**: Context menu can queue, app executes
- **Predictive engine**: Learn user patterns, optimize all three
- **Settings sync**: One preference system for all components

## Design Principles

### 1. Context Awareness Excellence

The menu should feel telepathic - knowing exactly what you want to do:

**Examples:**
- Right-click modified `README.md` → "📝 Commit README.md..." (not generic "Commit")
- Right-click clean repo → "↻ Pull from origin/main" (knows you're synced)
- Right-click during merge → "⚠️ MERGE IN PROGRESS" header (alerts to special state)

### 2. Visual Status Communication

Every menu item tells a complete story:

**Bad:** "Push"
**Good:** "↑ Push 3 commits to origin/main"
**Perfect:** "↑ Push to origin/main (3 commits, 2.3MB)"

### 3. Seamless App Handoff

When operations need more UI, the handoff is magical:
- Context menu item: "📝 Commit 5 files..." → Click
- App opens instantly to commit dialog
- Files pre-staged, diff visible
- Cursor in commit message field

### 4. Quick Actions Parity

Every context menu operation is available globally:
- Context menu: "📝 Commit README.md..."
- Quick Actions (Win+G): Type "commit readme" → Same operation
- Both use same backend, same shortcuts, same results

### 5. Performance Budget

- Menu generation: <20ms
- Cache queries: <1ms
- App launch from menu: <500ms
- Status overlay update after operation: <50ms


### 1. Context is King

The menu should feel like it was designed specifically for what the user right-clicked on.

**Examples:**
- Right-click on `README.md` (modified) → "Commit README.md..."
- Right-click on `src/` (10 changes) → "Commit 10 changed files..."
- Right-click on repo root (clean, 3 commits ahead) → "Push 3 commits to origin/main"

### 2. Show State, Not Just Actions

Menu items display current state and what will happen.

**Bad:** "Push"
**Good:** "↑ Push 3 commits to origin/main"

**Bad:** "Pull"
**Good:** "↻ Pull from origin/main (↓ 2 new commits)"

### 3. Progressive Disclosure

- **First menu item**: Most likely action (based on context)
- **Next 3-5 items**: Common related operations
- **Separators**: Group related operations
- **Submenu**: Advanced/less common operations

### 4. Fast by Default

- Menu builds in <20ms (no blocking operations)
- Use cached status (1-second TTL)
- Show placeholders for slow operations ("Loading...")
- Never block Explorer

### 5. Visual Hierarchy

- **Icons**: Help quick scanning and categorization
- **Emphasis**: Bold most likely action
- **Color coding**: Via icon colors (red=destructive, green=commit, blue=sync)
- **Separators**: Clear visual grouping

### 6. Forgiving and Informative

- Show why an operation is disabled ("Can't push: no commits ahead")
- Provide escape hatches ("Abort merge")
- Offer help for unusual states ("Repository in rebase state. Learn more")

---

## Integration with GitScribe App

### Handoff Principles

The context menu and app work as one cohesive system:

1. **Simple operations stay in shell**: Revert, stage, unstage - no app needed
2. **Visual operations launch app**: Diff, history, blame - need rich UI
3. **Complex operations require app**: Merge, rebase, conflict resolution
4. **App opens to exact context**: Not just "open app", but "open to this file's diff"

### App Launch Scenarios

#### From File Context
```cpp
// User right-clicks modified file, selects "Commit"
LaunchApp("commit", {
    "focus_file": "src/main.cpp",
    "auto_stage": true,
    "diff_visible": true
});
// App opens with file pre-staged, diff shown
```

#### From Repository Context
```cpp
// User right-clicks repo folder, selects "Show All Changes"
LaunchApp("status", {
    "repo_path": "C:\\Projects\\MyApp",
    "filter": "modified"
});
// App opens to status view, modified files filtered
```

#### From Merge Context
```cpp
// User right-clicks conflicted file, selects "Resolve"
LaunchApp("conflict", {
    "file": "src/auth.js",
    "mode": "3way"
});
// App opens to 3-way merge tool for that file
```

### Quick Actions Integration

The context menu can trigger Quick Actions Bar for complex commands:

```
User right-clicks → "⚡ Quick Actions..." → Win+G
Quick Actions Bar appears with context pre-filled:
┌─────────────────────────────────────┐
│ 🔍 src/main.cpp >                  │ ← File context auto-filled
├─────────────────────────────────────┤
│ > commit "fix: memory leak"        │
│ > diff with main                   │
│ > blame line 42                    │
└─────────────────────────────────────┘
```

### Shared Operation Queue

Context menu can queue operations for app to execute:

```cpp
// User selects multiple operations quickly
QueueOperation("stage", { files: selected });
QueueOperation("commit", { message: "feat: new feature" });
QueueOperation("push", { remote: "origin", branch: "main" });

// App processes queue atomically
ExecuteOperationQueue(); // All succeed or rollback
```

---

## Context Detection

The menu generation algorithm follows this decision tree:

```
┌─────────────────────────────────────┐
│ What did user right-click?          │
└──────────┬──────────────────────────┘
           │
           ├─ Outside Git repo? → [No Git operations]
           │
           ├─ Single file? ─┬─ Modified? → [File Modified Menu]
           │                ├─ Untracked? → [File Untracked Menu]
           │                ├─ Conflicted? → [File Conflict Menu]
           │                └─ Clean? → [File Clean Menu]
           │
           ├─ Multiple files? → [Multi-Selection Menu]
           │
           └─ Folder/Repo? ─┬─ Merging/Rebasing? → [Merge State Menu]
                            ├─ Has changes? → [Repo Dirty Menu]
                            ├─ Clean + ahead? → [Repo Ahead Menu]
                            ├─ Clean + behind? → [Repo Behind Menu]
                            └─ Clean + synced? → [Repo Clean Menu]
```

### Context Types

#### 1. File Modified
- **Detection**: `git status` shows `M ` or ` M`
- **Primary action**: Commit this file
- **Show**: Diff, revert, blame, history

#### 2. File Untracked
- **Detection**: `git status` shows `??`
- **Primary action**: Add to Git
- **Show**: Ignore options, .gitignore templates

#### 3. File Conflicted
- **Detection**: `git status` shows `UU`, `AA`, `DD`, etc.
- **Primary action**: Resolve conflict
- **Show**: Accept ours, accept theirs, 3-way merge tool

#### 4. File Clean
- **Detection**: File tracked, no modifications
- **Primary action**: Show history
- **Show**: Blame, compare with branch

#### 5. Multi-Selection
- **Detection**: Multiple items selected
- **Primary action**: Commit selected files (if any modified)
- **Show**: Diff selected, revert selected

#### 6. Repository Dirty
- **Detection**: Working tree has changes
- **Primary action**: Commit all changes
- **Show**: Diff, pull, push (if also ahead)

#### 7. Repository Ahead
- **Detection**: Clean working tree, commits ahead of remote
- **Primary action**: Push commits
- **Show**: View commits to be pushed, pull, sync

#### 8. Repository Behind
- **Detection**: Clean working tree, commits behind remote
- **Primary action**: Pull
- **Show**: View incoming commits, fetch

#### 9. Repository Clean
- **Detection**: No changes, synced with remote
- **Primary action**: Pull (to check for updates)
- **Show**: Branch manager, history, settings

#### 10. Merge/Rebase in Progress
- **Detection**: `.git/MERGE_HEAD` or `.git/rebase-merge/` exists
- **Primary action**: Resolve conflicts / Continue
- **Show**: Abort, list conflicted files

---

## Menu Structures

### Template: File Modified

```
┌─────────────────────────────────────────────┐
│ 📝 Commit "filename.ext"...         Ctrl+K  │ ← Primary (opens app to commit dialog)
│ 📊 Diff with HEAD                   Ctrl+D  │ ← Opens app to diff view
│ ↩️  Revert changes...                        │ ← Simple operation, no app needed
├─────────────────────────────────────────────┤
│ 📋 Compare with...                  ▶       │
│ 📜 Show History                     Ctrl+L  │ ← Opens app to history view
│ 🏷️  Blame                           Ctrl+B  │ ← Opens app to blame view
├─────────────────────────────────────────────┤
│ 🚀 Open in GitScribe                Ctrl+G  │ ← Direct app launch
│ ⚡ Quick Actions...                 Win+G   │ ← Launch Quick Actions Bar
├─────────────────────────────────────────────┤
│ GitScribe                           ▶       │ ← Advanced submenu
└─────────────────────────────────────────────┘
```

**Submenu: Compare with...**
```
┌─────────────────────────────────────────────┐
│ 📋 Working tree vs HEAD                     │
│ 📋 Working tree vs Staged                   │
│ 📋 Staged vs HEAD                           │
├─────────────────────────────────────────────┤
│ 🌿 Compare with Branch...                   │
│ 🏷️  Compare with Tag...                     │
│ 📅 Compare with Commit...                   │
└─────────────────────────────────────────────┘
```

### Template: File Untracked

```
┌─────────────────────────────────────────────┐
│ ➕ Add "filename.ext" to Git        Ctrl+A  │
│ 🚫 Ignore "filename.ext"                    │
│    └─ 🚫 Ignore *.ext files                 │
│    └─ 🚫 Ignore /folder/ directory          │
│    └─ 📝 Edit .gitignore                    │
├─────────────────────────────────────────────┤
│ 📋 Show File                                │
├─────────────────────────────────────────────┤
│ GitScribe                           ▶       │
└─────────────────────────────────────────────┘
```

### Template: File Conflicted

```
┌─────────────────────────────────────────────┐
│ ⚠️  CONFLICT IN FILE                         │ ← Warning header
├─────────────────────────────────────────────┤
│ ✅ Resolve in Editor...                     │
│ ⬅️  Accept Ours (current branch)            │
│ ➡️  Accept Theirs (merging branch)          │
│ 🔀 3-Way Merge Tool...                      │
├─────────────────────────────────────────────┤
│ 📊 Show Conflict                            │
│ ❓ Help with Conflicts                      │
├─────────────────────────────────────────────┤
│ GitScribe                           ▶       │
└─────────────────────────────────────────────┘
```

### Template: Repository Dirty

```
┌─────────────────────────────────────────────┐
│ 📝 Commit 12 changed files...       Ctrl+K  │ ← Count shown
│ 📊 Show All Changes                 Ctrl+D  │
│ ↻  Pull from origin/main            Ctrl+P  │
│ ↑  Push 3 commits                   Ctrl+⇧P │ ← Only if ahead
├─────────────────────────────────────────────┤
│ 🌿 Branches (main ⭐)               ▶       │
│ 📜 Repository History               Ctrl+L  │
│ ⚙️  Repository Settings...                  │
├─────────────────────────────────────────────┤
│ GitScribe                           ▶       │
└─────────────────────────────────────────────┘
```

### Template: Repository Ahead

```
┌─────────────────────────────────────────────┐
│ ↑  Push 3 commits to origin/main    Ctrl+⇧P │
│ 🔄 Sync (pull then push)            Ctrl+Y  │
│ 📋 View Commits to Push...                  │
├─────────────────────────────────────────────┤
│ ↻  Pull from origin/main            Ctrl+P  │
│ 📡 Fetch All Remotes                        │
├─────────────────────────────────────────────┤
│ 🌿 Branches (main ⭐)               ▶       │
│ 📜 Repository History               Ctrl+L  │
│ ⚙️  Repository Settings...                  │
├─────────────────────────────────────────────┤
│ GitScribe                           ▶       │
└─────────────────────────────────────────────┘
```

### Template: Merge in Progress

```
┌─────────────────────────────────────────────┐
│ ⚠️  MERGE IN PROGRESS                        │
│ Merging feature/auth into main              │
├─────────────────────────────────────────────┤
│ ✅ Continue Merge (3 conflicts remain)      │ ← Disabled if conflicts
│ ❌ Abort Merge                              │
├─────────────────────────────────────────────┤
│ 📊 Show Conflicts (3 files)                 │
│    📄 src/auth.js                   ▶       │
│    📄 src/users.js                  ▶       │
│    📄 tests/auth.test.js            ▶       │
├─────────────────────────────────────────────┤
│ ❓ Help with Merges                         │
└─────────────────────────────────────────────┘
```

### Submenu: GitScribe (Always Available)

```
┌─────────────────────────────────────────────┐
│ 🔍 Repository Status                F5      │
├─────────────────────────────────────────────┤
│ 🌿 Branch Manager...                        │
│ 🏷️  Tags...                                 │
│ 🔀 Merge Branch...                          │
│ 📈 Rebase...                                │
│ 🍒 Cherry-Pick...                           │
│ 📦 Stash Manager...                         │
├─────────────────────────────────────────────┤
│ 🔧 Advanced                         ▶       │
│ ⚙️  Repository Settings...                  │
│ 📖 Help                             F1      │
│ ℹ️  About GitScribe                         │
└─────────────────────────────────────────────┘
```

### Submenu: Branches

```
┌─────────────────────────────────────────────┐
│ 🌿 main ⭐ ↑3                                │ ← Current (bold)
│ 🌿 feature/new-ui 📝 ↑5 ↓2         Ctrl+1  │ ← Recent branches
│ 🌿 feature/auth ✅                  Ctrl+2  │    with quick switch
│ 🌿 bugfix/issue-123 📝              Ctrl+3  │
├─────────────────────────────────────────────┤
│ ✨ New Branch...                    Ctrl+N  │
│ 🔍 Search Branches...               Ctrl+F  │
│ 🧹 Cleanup Merged Branches...               │
├─────────────────────────────────────────────┤
│ 📡 Fetch All Remotes                Ctrl+⇧F │
│ 🔄 Fetch & Prune                            │
└─────────────────────────────────────────────┘

Legend:
⭐ = Current branch
📝 = Has uncommitted changes
✅ = Clean
↑N = N commits ahead
↓N = N commits behind
```

### Submenu: Advanced (under GitScribe)

```
┌─────────────────────────────────────────────┐
│ 🔍 Search Commits...                        │
│ 🔎 Find in Files (History)...               │
├─────────────────────────────────────────────┤
│ ♻️  Reflog                                   │
│ 🧹 Clean Working Directory...               │
│ 🗑️  GC & Optimize Repository                │
├─────────────────────────────────────────────┤
│ 📦 Submodules                       ▶       │
│ 🌲 Worktrees                        ▶       │
│ 💾 LFS                              ▶       │
├─────────────────────────────────────────────┤
│ 🔐 Credentials Manager...                   │
│ 🔗 Remote Repositories...                   │
│ 🎣 Git Hooks...                             │
└─────────────────────────────────────────────┘
```

---

## Visual Design

> **See also**: [GitScribe Brand Guidelines](./BRANDING_GUIDELINES.md) for complete visual specifications

### Design System Integration

The context menu follows the GitScribe brand guidelines:
- **Dark Mode**: Obsidian black with royal gold accents
- **Light Mode**: Heavenly white with seraph gold highlights
- **Typography**: Inter for UI text, JetBrains Mono for code
- **Motion**: Smooth transitions with $ease-smooth curve

### Icons

Icons serve three purposes:
1. **Quick visual scanning**: Recognize operation type without reading
2. **Color coding**: Semantic meaning (red=danger, green=add, blue=sync)
3. **Consistency**: Same icon always means same operation

#### Icon Mapping

| Operation Type | Icon | Color | Examples |
|---------------|------|-------|----------|
| Commit | 📝 | Green | Commit files, amend |
| Sync (Push) | ↑ | Blue | Push commits |
| Sync (Pull) | ↻ | Blue | Pull, fetch |
| Sync (General) | 🔄 | Blue | Sync, fetch all |
| Branch | 🌿 | Purple | Branch manager, switch |
| Diff/Compare | 📊📋 | Orange | Diff, compare |
| History | 📜 | Gray | Log, history, reflog |
| Blame | 🏷️ | Gray | Annotate, blame |
| Merge | 🔀 | Purple | Merge, rebase |
| Conflict | ⚠️ | Red | Resolve, conflict |
| Revert/Undo | ↩️ | Red | Revert, reset |
| Delete | 🗑️❌ | Red | Clean, delete, abort |
| Add | ➕ | Green | Stage, add |
| Ignore | 🚫 | Gray | Ignore, exclude |
| Settings | ⚙️ | Gray | Settings, config |
| Tools | 🔧 | Gray | Advanced, tools |
| Search | 🔍🔎 | Gray | Find, search |
| Help | ❓📖 | Blue | Help, docs |
| Info | ℹ️ | Blue | About, status |
| Tag | 🏷️ | Yellow | Tags, releases |
| Stash | 📦 | Purple | Stash, pop |
| Cherry-pick | 🍒 | Purple | Cherry-pick |

### Text Formatting

```
┌─────────────────────────────────────────────┐
│ 📝 Commit 12 files...               Ctrl+K  │
│ └┬──────┬─────────────────────────────┬───  │
│  │      │                             └──── Right-aligned shortcut
│  │      └──────────────────────────────── Action description
│  └────────────────────────────────────── Icon + action type
│
│ BOLD = Primary action or current item       │
│ Regular = Standard action                   │
│ Dimmed = Disabled action                    │
└─────────────────────────────────────────────┘
```

### Menu Width

- **Minimum**: 280px
- **Maximum**: 400px
- **Preferred**: 320px

### Menu Item Height

- **Standard**: 24px
- **With icon**: 24px (icon 16x16, 4px padding each side)
- **Section header**: 20px (slightly shorter)

### Separators

- **Thickness**: 1px
- **Color**: System default (light gray in light mode)
- **Margins**: 4px top, 4px bottom

### Tooltips

When hovering over a menu item for >500ms, show extended tooltip:

```
┌────────────────────────────────────────┐
│ ↑ Push 3 commits to origin/main        │
└────────────────────────────────────────┘
          │
          ├─→ Hovering triggers tooltip:
          │
     ┌────────────────────────────────────┐
     │ Push to origin/main                │
     │                                    │
     │ Will push:                         │
     │ • a3f21b9: Add dark mode toggle    │
     │ • b4e8c12: Fix navbar layout       │
     │ • c9d1a45: Update dependencies     │
     │                                    │
     │ Remote: github.com/user/repo       │
     │                                    │
     │ Shortcut: Ctrl+Shift+P             │
     └────────────────────────────────────┘
```

### Disabled Items

Items that are disabled show:
1. Dimmed text (50% opacity)
2. Dimmed icon (50% opacity)
3. Tooltip explaining why (on hover)

Example:
```
┌─────────────────────────────────────────────┐
│ ↑ Push to origin/main                       │ ← Dimmed
└─────────────────────────────────────────────┘

Tooltip on hover:
┌─────────────────────────────────────────────┐
│ Can't push: No commits ahead of remote      │
│                                             │
│ Try pulling first to get latest changes.    │
└─────────────────────────────────────────────┘
```

---

## Keyboard Shortcuts

### Global Shortcuts (Work Anywhere in Menu)

| Shortcut | Action | Notes |
|----------|--------|-------|
| `Ctrl+K` | Commit | Most common action |
| `Ctrl+D` | Diff | Show changes |
| `Ctrl+L` | Log/History | View commits |
| `Ctrl+B` | Blame | Annotate |
| `Ctrl+P` | Pull | Sync down |
| `Ctrl+Shift+P` | Push | Sync up |
| `Ctrl+Y` | Sync | Pull then push |
| `Ctrl+N` | New Branch | Create branch |
| `Ctrl+F` | Find/Search | Search branches/commits |
| `F5` | Refresh Status | Update repo state |
| `F1` | Help | Open help |
| `Esc` | Close Menu | Standard Windows |

### Contextual Shortcuts

| Shortcut | Action | When Available |
|----------|--------|----------------|
| `Ctrl+A` | Add to Git | Untracked file |
| `Ctrl+1-9` | Switch to Branch 1-9 | In Branches submenu |
| `Ctrl+Shift+F` | Fetch All | In Branches submenu |

### Navigation Shortcuts

| Shortcut | Action |
|----------|--------|
| `↑`/`↓` | Navigate menu items |
| `→` | Open submenu |
| `←` | Close submenu |
| `Enter` | Execute selected item |
| `Esc` | Close menu |
| `Alt+Letter` | Jump to item (standard Windows) |

### Shortcut Display

- Show shortcuts right-aligned in menu
- Use standard Windows format: `Ctrl+Shift+P`
- Don't show shortcuts for submenu indicators (just show `▶`)

---

## Performance Requirements

### Menu Generation

| Metric | Target | Maximum | Fallback |
|--------|--------|---------|----------|
| Time to display menu | <20ms | 50ms | Show "GitScribe (Loading...)" |
| Time to query status | <10ms | 100ms | Use cached status |
| Cache TTL | 1s | 5s | Based on system load |
| Menu item count | <15 | 20 | Move to submenus |

### Status Queries

All status queries must be non-blocking:

1. **Check cache first** (SQLite, <1ms)
2. **If cache fresh** (<1s old): Use cached data
3. **If cache stale**:
   - Return cached data immediately
   - Queue background refresh
   - Update menu when refresh completes (if still open)

### Memory Budget

- **Per menu instance**: <2MB
- **Shared cache**: <50MB
- **Icon cache**: <10MB

### Graceful Degradation

If status query times out (>100ms):
- Show basic menu with generic options
- Display "Status unavailable" notice
- Log to telemetry for debugging

---

## Integration Specification for Parallel Development

### Technology Stack Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Windows Explorer                         │
└──────────────────────────┬──────────────────────────────────┘
                           │
                ┌──────────▼──────────┐
                │  gitscribe-shell    │ ← C++ (Windows requirement)
                │  (C++ COM DLL)      │   Shell extensions MUST be COM
                └──────────┬──────────┘
                           │ C FFI
                ┌──────────▼──────────┐
                │  gitscribe-core     │ ← Rust (shared logic)
                │  (Rust library)     │   Memory safe, fast, cross-component
                └──────────┬──────────┘
                           │ N-API
                ┌──────────▼──────────┐
                │  gitscribe-app      │ ← Electron + TypeScript
                │  (Electron)         │   Rich UI
                └─────────────────────┘
```

**Why This Stack:**
- **C++ for shell extension**: Windows COM requirement (no choice)
- **Rust for core**: Write once, use everywhere + memory safety
- **Electron for app**: Best for rich, modern UI

The C++ shell extension is a **thin COM wrapper** around Rust core. All Git operations, caching, and business logic live in Rust.

### Rust Core C FFI Interface

The `gitscribe-core` Rust library exposes a C-compatible API for the C++ shell extension:

```rust
// gitscribe-core/src/ffi.rs

use std::os::raw::{c_char, c_int, c_void};

#[repr(C)]
pub struct gs_repository {
    _private: [u8; 0], // Opaque pointer
}

#[repr(C)]
pub struct gs_status {
    file_count: c_int,
    modified_count: c_int,
    added_count: c_int,
    deleted_count: c_int,
}

#[repr(C)]
pub struct gs_file_status {
    path: *const c_char,
    work_tree_status: c_int,  // 0=clean, 1=modified, 2=added, etc.
    index_status: c_int,
}

// Repository operations
#[no_mangle]
pub extern "C" fn gs_repository_open(
    path: *const c_char,
    error: *mut *mut c_char
) -> *mut gs_repository;

#[no_mangle]
pub extern "C" fn gs_repository_close(repo: *mut gs_repository);

// Status operations (cached)
#[no_mangle]
pub extern "C" fn gs_repository_status_cached(
    repo: *mut gs_repository,
    ttl_ms: c_int
) -> *mut gs_status;

#[no_mangle]
pub extern "C" fn gs_status_get_file(
    status: *mut gs_status,
    index: c_int
) -> *mut gs_file_status;

#[no_mangle]
pub extern "C" fn gs_status_free(status: *mut gs_status);

// Cache operations
#[no_mangle]
pub extern "C" fn gs_cache_init(
    db_path: *const c_char
) -> c_int;

#[no_mangle]
pub extern "C" fn gs_cache_invalidate_file(
    repo_path: *const c_char,
    file_path: *const c_char
) -> c_int;

// Remote status (async, cached)
#[no_mangle]
pub extern "C" fn gs_repository_remote_status_cached(
    repo: *mut gs_repository,
    ahead: *mut c_int,
    behind: *mut c_int
) -> c_int;

// Branch operations
#[no_mangle]
pub extern "C" fn gs_repository_current_branch(
    repo: *mut gs_repository
) -> *const c_char;

// String cleanup
#[no_mangle]
pub extern "C" fn gs_string_free(s: *mut c_char);
```

### C++ Shell Extension Usage

The C++ shell extension calls into Rust like this:

```cpp
// gitscribe-shell/src/GitRepository.cpp

#include "gitscribe_core.h" // Generated header from cbindgen

class GitRepository {
public:
    GitRepository(const std::wstring& path) {
        // Convert wstring to UTF-8 for Rust
        std::string utf8Path = WideToUtf8(path);

        char* error = nullptr;
        m_repo = gs_repository_open(utf8Path.c_str(), &error);

        if (!m_repo) {
            std::string errorMsg = error ? error : "Unknown error";
            gs_string_free(error);
            throw std::runtime_error(errorMsg);
        }
    }

    ~GitRepository() {
        if (m_repo) {
            gs_repository_close(m_repo);
        }
    }

    // Get cached status (fast!)
    FileStatusList GetStatus(int ttlMs = 1000) {
        gs_status* status = gs_repository_status_cached(m_repo, ttlMs);

        FileStatusList result;
        for (int i = 0; i < status->file_count; i++) {
            gs_file_status* file = gs_status_get_file(status, i);
            result.push_back({
                Utf8ToWide(file->path),
                static_cast<FileStatus>(file->work_tree_status),
                static_cast<FileStatus>(file->index_status)
            });
        }

        gs_status_free(status);
        return result;
    }

    std::wstring GetCurrentBranch() {
        const char* branch = gs_repository_current_branch(m_repo);
        std::wstring result = Utf8ToWide(branch);
        gs_string_free(const_cast<char*>(branch));
        return result;
    }

private:
    gs_repository* m_repo = nullptr;
};
```

### Build Configuration

**Rust Core:**
```toml
# gitscribe-core/Cargo.toml
[package]
name = "gitscribe-core"
version = "0.1.0"

[lib]
crate-type = ["cdylib", "staticlib", "rlib"]

[dependencies]
libgit2-sys = "0.16"
rusqlite = "0.31"

[build-dependencies]
cbindgen = "0.26" # Generates C header
```

**C++ Shell Extension:**
```cmake
# gitscribe-shell/CMakeLists.txt
add_library(gitscribe-shell SHARED
    src/ContextMenu.cpp
    src/OverlayHandler.cpp
    src/GitRepository.cpp
)

# Link against Rust static library
target_link_libraries(gitscribe-shell
    ${CMAKE_SOURCE_DIR}/../target/release/gitscribe_core.lib
    ws2_32 # Windows sockets (required by Rust)
    userenv # User environment (required by Rust)
    bcrypt # Crypto (required by Rust)
)

target_include_directories(gitscribe-shell PRIVATE
    ${CMAKE_SOURCE_DIR}/../target/release/include # Generated C headers
)
```

### Critical Integration Points

These interfaces MUST be implemented identically by all teams to ensure compatibility:

#### 1. Shared SQLite Schema
```sql
-- Version: 1.0.0 (DO NOT MODIFY without coordination)
-- Location: %APPDATA%\GitScribe\cache.db

CREATE TABLE IF NOT EXISTS repositories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT UNIQUE NOT NULL,
    last_accessed INTEGER NOT NULL,
    is_valid INTEGER DEFAULT 1
);

CREATE TABLE IF NOT EXISTS file_status (
    repo_id INTEGER NOT NULL,
    file_path TEXT NOT NULL,
    work_tree_status INTEGER NOT NULL,  -- 0=clean, 1=modified, 2=added, 3=deleted, 4=renamed, 5=copied, 6=ignored
    index_status INTEGER NOT NULL,      -- Same values as work_tree_status
    cache_time INTEGER NOT NULL,
    file_mtime INTEGER NOT NULL,
    PRIMARY KEY (repo_id, file_path),
    FOREIGN KEY (repo_id) REFERENCES repositories(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS repo_status (
    repo_id INTEGER PRIMARY KEY,
    current_branch TEXT,
    upstream_branch TEXT,
    ahead_count INTEGER DEFAULT 0,
    behind_count INTEGER DEFAULT 0,
    merge_head TEXT,                    -- If merging
    rebase_head TEXT,                   -- If rebasing
    cache_time INTEGER NOT NULL,
    FOREIGN KEY (repo_id) REFERENCES repositories(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS operation_queue (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    repo_id INTEGER NOT NULL,
    operation_type TEXT NOT NULL,       -- 'stage', 'commit', 'push', 'pull', etc.
    operation_data TEXT NOT NULL,       -- JSON payload
    status INTEGER DEFAULT 0,           -- 0=pending, 1=running, 2=complete, 3=failed
    created_at INTEGER NOT NULL,
    started_at INTEGER,
    completed_at INTEGER,
    error_message TEXT,
    FOREIGN KEY (repo_id) REFERENCES repositories(id) ON DELETE CASCADE
);

-- Indexes for performance
CREATE INDEX idx_file_status_cache ON file_status(cache_time);
CREATE INDEX idx_repo_status_cache ON repo_status(cache_time);
CREATE INDEX idx_operation_queue_status ON operation_queue(status, created_at);
```

#### 2. Named Pipe Protocol
```cpp
// Pipe name: \\.\pipe\GitScribe.{UserSID}
// Protocol: JSON-RPC 2.0

// Request from Context Menu to App
{
  "jsonrpc": "2.0",
  "method": "launchApp",
  "params": {
    "action": "commit",              // Action type
    "context": {
      "files": ["C:\\repo\\file.cpp"],
      "repo": "C:\\repo",
      "branch": "main",
      "status": 1                    // Modified
    }
  },
  "id": 1
}

// Response from App
{
  "jsonrpc": "2.0",
  "result": {
    "status": "success",
    "pid": 12345                    // Process ID if launched
  },
  "id": 1
}

// Notification from App to Shell
{
  "jsonrpc": "2.0",
  "method": "refreshStatus",
  "params": {
    "repo": "C:\\repo",
    "files": ["file.cpp"]           // Specific files, or null for all
  }
}
```

#### 3. Command Line Protocol
```bash
# App launch protocol (context menu → app)
gitscribe.exe [command] [options]

Commands:
  --commit --file="path" --repo="path"     # Open commit dialog for file
  --status --repo="path"                   # Open status view
  --diff --file="path" --line=42          # Open diff at specific line
  --resolve --file="path"                  # Open conflict resolver
  --blame --file="path" --line=42         # Open blame view
  --history --file="path"                  # Open file history
  --quick --context="path"                 # Open Quick Actions with context
  --settings --page="general"              # Open settings to specific page

# Deep linking protocol
gitscribe://[action]/[params]
gitscribe://commit?file=C:\repo\file.cpp&line=42
gitscribe://diff?from=HEAD~1&to=HEAD
gitscribe://settings?page=pro
```

#### 4. Registry Structure
```reg
# Shared registry keys (coordinate any changes)
HKEY_CURRENT_USER\Software\GitScribe
├── Settings
│   ├── Theme (DWORD: 0=Dark, 1=Light, 2=System)
│   ├── CachePath (STRING: path to cache.db)
│   ├── LogLevel (DWORD: 0=Error, 1=Warning, 2=Info, 3=Debug)
│   └── Language (STRING: "en-US", "es-ES", etc.)
├── Shell
│   ├── ContextMenuEnabled (DWORD: 0/1)
│   ├── OverlaysEnabled (DWORD: 0/1)
│   └── MenuStyle (DWORD: 0=Full, 1=Compact)
├── App
│   ├── WindowPosition (STRING: "x,y,width,height")
│   ├── LastOpenRepo (STRING: path)
│   └── QuickActionsHotkey (STRING: "Win+G")
└── Pro
    ├── LicenseKey (STRING: encrypted)
    ├── AIEnabled (DWORD: 0/1)
    └── Features (DWORD: bitmask)
```

#### 5. Status Code Enumeration
```cpp
// Must match across all components
enum class FileStatus : int {
    Clean = 0,
    Modified = 1,
    Added = 2,
    Deleted = 3,
    Renamed = 4,
    Copied = 5,
    Ignored = 6,
    Conflicted = 7,
    Untracked = 8
};

enum class RepoState : int {
    Clean = 0,
    Dirty = 1,
    Merging = 2,
    Rebasing = 3,
    CherryPicking = 4,
    Reverting = 5,
    Bisecting = 6
};

enum class OperationType : int {
    Stage = 0,
    Unstage = 1,
    Commit = 2,
    Push = 3,
    Pull = 4,
    Fetch = 5,
    Merge = 6,
    Rebase = 7,
    Reset = 8,
    Revert = 9
};
```

#### 6. Icon Resource IDs
```cpp
// Shared icon resources (must be consistent)
#define IDI_OVERLAY_MODIFIED    100
#define IDI_OVERLAY_ADDED       101
#define IDI_OVERLAY_DELETED     102
#define IDI_OVERLAY_IGNORED     103
#define IDI_OVERLAY_CONFLICTED  104
#define IDI_OVERLAY_CLEAN       105

#define IDI_MENU_COMMIT         200
#define IDI_MENU_PUSH           201
#define IDI_MENU_PULL           202
#define IDI_MENU_DIFF           203
#define IDI_MENU_HISTORY        204
#define IDI_MENU_BRANCH         205
#define IDI_MENU_MERGE          206
#define IDI_MENU_SETTINGS       207
```

### Development Synchronization Points

#### Week 1-4 Checkpoint
- [ ] SQLite schema finalized and tested
- [ ] Cache read/write library shared between teams
- [ ] Status enumeration headers shared

#### Week 5-8 Checkpoint
- [ ] Named pipe protocol tested between components
- [ ] Command line launch protocol working
- [ ] Registry structure created and documented

#### Week 9-12 Checkpoint
- [ ] Full integration testing suite
- [ ] Performance benchmarks met
- [ ] Error handling coordinated

### Testing Harness

```cpp
// Shared test framework
class IntegrationTest {
public:
    // Test cache consistency
    void TestCacheReadWrite();
    void TestCacheConcurrency();

    // Test IPC
    void TestNamedPipeHandshake();
    void TestCommandLineLaunch();

    // Test state consistency
    void TestStatusEnumConsistency();
    void TestOperationQueueSync();
};
```

## Implementation Architecture

### C++ Component Structure

```
gitscribe-shell/
├── src/
│   ├── ContextMenu.cpp          # Main IContextMenu implementation
│   ├── ContextMenu.h
│   ├── MenuBuilder.cpp          # Menu generation logic
│   ├── MenuBuilder.h
│   ├── MenuContext.cpp          # Context detection
│   ├── MenuContext.h
│   ├── MenuItems.cpp            # Menu item definitions
│   ├── MenuItems.h
│   ├── KeyboardShortcuts.cpp    # Shortcut handling
│   ├── KeyboardShortcuts.h
│   └── MenuIcons.cpp            # Icon management
│       MenuIcons.h
```

### Class: ContextMenu (IContextMenu)

```cpp
class ContextMenu : public IContextMenu3,
                     public IShellExtInit {
public:
    // IContextMenu
    STDMETHOD(QueryContextMenu)(HMENU hMenu, UINT indexMenu,
                                UINT idCmdFirst, UINT idCmdLast,
                                UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType,
                                UINT* pReserved, LPSTR pszName,
                                UINT cchMax);

    // IContextMenu3 (for custom drawing, icons, tooltips)
    STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam,
                              LPARAM lParam, LRESULT* plResult);

    // IShellExtInit
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder,
                          IDataObject* pdtobj,
                          HKEY hkeyProgID);

private:
    std::vector<std::wstring> m_selectedPaths;
    MenuContext m_context;
    MenuBuilder m_menuBuilder;
};
```

### Class: MenuContext

Detects and encapsulates current context.

```cpp
enum class ContextType {
    None,
    FileModified,
    FileUntracked,
    FileConflicted,
    FileClean,
    MultiSelection,
    RepoDirty,
    RepoAhead,
    RepoBehind,
    RepoClean,
    MergeInProgress,
    RebaseInProgress
};

class MenuContext {
public:
    MenuContext(const std::vector<std::wstring>& paths);

    ContextType GetType() const;
    gs_repository* GetRepository() const;
    gs_status* GetStatus() const;
    gs_remote_status* GetRemoteStatus() const;

    // Context-specific queries
    int GetChangedFileCount() const;
    int GetAheadCount() const;
    int GetBehindCount() const;
    int GetConflictCount() const;
    std::wstring GetCurrentBranch() const;
    std::wstring GetRemoteBranch() const;

private:
    ContextType m_type;
    std::wstring m_repoPath;
    gs_repository* m_repo;
    gs_status* m_status;
    gs_remote_status* m_remoteStatus;

    void DetectContext();
};
```

### Class: MenuBuilder

Builds appropriate menu based on context.

```cpp
class MenuBuilder {
public:
    MenuBuilder(UINT idCmdFirst, UINT idCmdLast);

    // Build menu based on context
    int BuildMenu(HMENU hMenu, UINT insertPos,
                  const MenuContext& context);

    // Menu structure builders
    void BuildFileModifiedMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildFileUntrackedMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildRepoAheadMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildMergeMenu(HMENU hMenu, const MenuContext& ctx);
    // ... etc

    // Submenu builders
    void BuildBranchesSubmenu(HMENU hMenu, const MenuContext& ctx);
    void BuildGitScribeSubmenu(HMENU hMenu, const MenuContext& ctx);
    void BuildAdvancedSubmenu(HMENU hMenu, const MenuContext& ctx);

    // Menu item helpers
    void AddMenuItem(HMENU hMenu, const MenuItem& item);
    void AddSeparator(HMENU hMenu);
    void AddSubmenu(HMENU hMenu, const wchar_t* text,
                    HMENU hSubmenu, const wchar_t* icon);

private:
    UINT m_idCmdFirst;
    UINT m_idCmdLast;
    UINT m_nextCmdId;
    std::map<UINT, MenuItem> m_items; // Command ID -> MenuItem
};
```

### Struct: MenuItem

```cpp
struct MenuItem {
    UINT cmdId;                    // Command ID
    std::wstring text;             // Display text
    std::wstring icon;             // Icon (emoji or path)
    std::wstring shortcut;         // Keyboard shortcut text
    std::wstring tooltip;          // Extended tooltip
    bool enabled;                  // Is enabled?
    bool bold;                     // Primary action?
    std::function<void()> handler; // Action to execute

    // For submenus
    bool isSubmenu;
    HMENU hSubmenu;
};
```

### Integration with gitscribe-core

```cpp
// Status queries (cached)
gs_status* status = gs_repository_status_cached(repo);

// Remote status (async, cached)
gs_remote_status* remoteStatus =
    gs_repository_remote_status_cached(repo);

// Branch list (cached)
gs_branch_list* branches = gs_repository_branches_cached(repo);

// All queries return cached data if available,
// queue background refresh if stale
```

---

## Localization

### Supported Languages (Phase 1)

1. English (en-US)
2. Spanish (es-ES)
3. French (fr-FR)
4. German (de-DE)
5. Japanese (ja-JP)
6. Chinese Simplified (zh-CN)

### String Resources

All strings stored in resource files:

```
gitscribe-shell/
├── resources/
│   ├── strings-en-US.json
│   ├── strings-es-ES.json
│   ├── strings-fr-FR.json
│   └── ...
```

Example: `strings-en-US.json`
```json
{
  "menu.commit": "Commit \"{0}\"...",
  "menu.commit.multiple": "Commit {0} changed files...",
  "menu.push": "Push {0} commits to {1}",
  "menu.pull": "Pull from {0}",
  "menu.diff": "Diff with HEAD",
  "tooltip.push": "Push to {0}\n\nWill push:\n{1}\n\nRemote: {2}",
  "disabled.push": "Can't push: No commits ahead of remote\n\nTry pulling first to get latest changes."
}
```

### Icons and Shortcuts

- Icons are language-agnostic (Unicode emojis)
- Shortcuts adapt to keyboard layout (Ctrl+K is Ctrl+K in all layouts)
- For non-Latin keyboards, show shortcuts in Latin (e.g., Japanese sees "Ctrl+K")

---

## Customization

### User Preferences

Users can customize via settings dialog (`⚙️ Repository Settings`):

#### Menu Layout
```
┌────────────────────────────────────────────┐
│ ☑ Show icons in menu                      │
│ ☑ Show keyboard shortcuts                 │
│ ☑ Show status information (e.g., "3 commits ahead") │
│ ☐ Compact menu (smaller font, less spacing) │
└────────────────────────────────────────────┘
```

#### Menu Items
```
┌────────────────────────────────────────────┐
│ Customize which items appear in menu:     │
│                                            │
│ ☑ Commit                                   │
│ ☑ Push                                     │
│ ☑ Pull                                     │
│ ☐ Fetch (I use Pull instead)               │
│ ☑ Diff                                     │
│ ☐ Blame (I never use this)                 │
│ ☑ History                                  │
│ ...                                        │
└────────────────────────────────────────────┘
```

#### Custom Commands
```
┌────────────────────────────────────────────┐
│ Add custom commands to menu:               │
│                                            │
│ [+] Add Custom Command                     │
│                                            │
│ 📝 "Run Tests"                             │
│    Command: npm test                       │
│    Icon: 🧪                                │
│    Shortcut: Ctrl+T                        │
│    [Edit] [Remove]                         │
│                                            │
│ 🚀 "Deploy to Staging"                     │
│    Command: npm run deploy:staging         │
│    Icon: 🚀                                │
│    Shortcut: Ctrl+Shift+D                  │
│    [Edit] [Remove]                         │
└────────────────────────────────────────────┘
```

### Enterprise Customization

For GitScribe Enterprise, admins can:

1. **Lock menu layout**: Prevent users from customizing
2. **Add required commands**: Force certain operations to appear
3. **Disable operations**: Hide risky operations (e.g., force push)
4. **Deploy templates**: Standard menu config for all users

Configured via `gitscribe-enterprise.json`:
```json
{
  "contextMenu": {
    "locked": true,
    "hiddenItems": ["force-push", "reset-hard"],
    "customCommands": [
      {
        "text": "Submit for Code Review",
        "icon": "👀",
        "command": "company-review-tool {repo}",
        "position": 0
      }
    ]
  }
}
```

---

## Testing Requirements

### Unit Tests

Test each context detection case:

```cpp
TEST(MenuContext, DetectsFileModified) {
    // Setup: Create repo with modified file
    auto repo = CreateTestRepo();
    ModifyFile(repo, "test.txt");

    // Test: Should detect FileModified context
    MenuContext ctx({"C:\\repo\\test.txt"});
    EXPECT_EQ(ctx.GetType(), ContextType::FileModified);
}

TEST(MenuBuilder, BuildsFileModifiedMenu) {
    // Setup: Create context
    MenuContext ctx = CreateFileModifiedContext();
    MenuBuilder builder(0, 1000);

    // Test: Should build correct menu
    HMENU hMenu = CreatePopupMenu();
    int itemsAdded = builder.BuildMenu(hMenu, 0, ctx);

    // Verify: Check menu structure
    EXPECT_GT(itemsAdded, 0);
    EXPECT_TRUE(HasMenuItem(hMenu, "Commit"));
    EXPECT_TRUE(HasMenuItem(hMenu, "Diff"));
}
```

### Integration Tests

Test with real Git repositories:

1. **Small repo** (10 files): All operations should be <20ms
2. **Medium repo** (1,000 files): Should use cache, <50ms
3. **Large repo** (10,000 files): Should use cache, <100ms
4. **Huge repo** (Linux kernel): Should gracefully degrade if needed

### UI Tests

Manual test checklist:

- [ ] Icons display correctly at 100%, 125%, 150%, 200% DPI
- [ ] Keyboard shortcuts work
- [ ] Tooltips appear after 500ms hover
- [ ] Disabled items show helpful tooltips
- [ ] Submenus open and navigate correctly
- [ ] Menu doesn't flicker or redraw
- [ ] Works in Dark Mode
- [ ] Works in High Contrast mode
- [ ] Works in different languages

### Performance Tests

Automated performance tests:

```cpp
TEST(Performance, MenuGeneration) {
    auto repo = LoadRealWorldRepo(); // 5,000 files
    MenuContext ctx(repo.GetRootPath());
    MenuBuilder builder(0, 1000);

    auto start = HighResTimer::Now();
    HMENU hMenu = CreatePopupMenu();
    builder.BuildMenu(hMenu, 0, ctx);
    auto elapsed = HighResTimer::Now() - start;

    EXPECT_LT(elapsed, std::chrono::milliseconds(20))
        << "Menu generation took " << elapsed << "ms";
}
```

---

## Edge Cases

### 1. Very Long File Names

If filename is >40 characters, truncate:
```
📝 Commit "this-is-a-very-long-filename-th..."
```

### 2. Many Files Changed

If >50 files changed:
```
📝 Commit 127 changed files...
```
(Don't try to list them in menu)

### 3. Special Characters in Names

Escape special characters properly:
```
📝 Commit "file&name.txt"...  // & → &&
```

### 4. Detached HEAD

Show special icon and explanation:
```
⚠️  Detached HEAD
🌿 Branches                    ▶
   📍 Currently at abc123f
   ✨ Create Branch from Here...
   ↩️  Checkout Branch...
```

### 5. Submodules

Indicate submodule status:
```
📦 submodule/path (📝 Modified submodule)
   ↻  Update Submodule
   📊 Show Submodule Changes
```

### 6. LFS Files

Show LFS indicator:
```
💾 large-file.psd (LFS, 125 MB)
   📊 Diff (metadata only)
   ↻  Pull LFS Files
```

### 7. Extremely Large Repos

If repo is >1GB or >100k files:
- Show warning in status tooltip
- Recommend Git optimization
- Disable some expensive operations (blame on large files)

### 8. No Remote Configured

Don't show push/pull:
```
📝 Commit...
📊 Diff
📜 History
─────────────────────────────
⚠️  No remote configured
📡 Add Remote...
```

### 9. Multiple Remotes

Allow selecting which remote to push/pull:
```
↑ Push to...                   ▶
   📡 origin (github.com)
   📡 upstream (gitlab.com)
   📡 staging (company.com)
```

### 10. Network Offline

When fetch fails due to network:
```
❌ Can't fetch: Network unavailable
↻  Retry
📡 Work Offline Mode
```

---

## AI-Powered Features (Pro Tier)

### Smart Commit Messages

When right-clicking modified files, Pro users see AI suggestions:

```
┌─────────────────────────────────────────────┐
│ 📝 Commit "auth.js"...              Ctrl+K  │
│    └─ ✨ AI: "fix: resolve token expiration"│ ← AI suggestion inline
├─────────────────────────────────────────────┤
│ ✨ Generate Commit Message...               │ ← Dedicated AI option
│ 📊 Diff with HEAD                   Ctrl+D  │
└─────────────────────────────────────────────┘
```

### AI Code Review

Before push operations:

```
┌─────────────────────────────────────────────┐
│ ↑ Push 3 commits to origin/main    Ctrl+⇧P │
│ ✨ AI Review Before Push...                 │ ← Check for issues
├─────────────────────────────────────────────┤
│ Last AI Review: 2 suggestions              │
│   ⚠ Potential memory leak in auth.js:42   │
│   💡 Consider error handling in api.js:15  │
└─────────────────────────────────────────────┘
```

### Conflict Resolution Assistant

During merge conflicts:

```
┌─────────────────────────────────────────────┐
│ ⚠️  CONFLICT IN FILE                         │
├─────────────────────────────────────────────┤
│ ✨ AI Suggest Resolution...                 │ ← AI analyzes both sides
│ ✅ Resolve in Editor...                     │
│ ⬅️  Accept Ours                             │
│ ➡️  Accept Theirs                           │
└─────────────────────────────────────────────┘
```

### Pattern Detection

AI notices patterns and suggests improvements:

```
Tooltip on "Commit" menu item:
┌─────────────────────────────────────────────┐
│ 💡 AI Notice: You often forget to update    │
│ version.json when modifying package.json    │
│                                             │
│ Would you like to add version.json too?    │
│ [Yes] [Not this time] [Don't ask again]    │
└─────────────────────────────────────────────┘
```

### Implementation

```cpp
class AIProvider {
public:
    // Generate commit message based on diff
    std::wstring GenerateCommitMessage(const GitDiff& diff);

    // Review code before push
    std::vector<CodeIssue> ReviewCode(const GitChanges& changes);

    // Suggest conflict resolution
    ConflictResolution SuggestResolution(const ConflictInfo& conflict);

    // Detect patterns in user behavior
    std::vector<Pattern> DetectPatterns(const UserHistory& history);

private:
    // API key stored encrypted in registry
    std::wstring GetAPIKey();

    // Local model for offline operation
    void LoadLocalModel();
};
```

## Development Phases

### Phase 0: GitScribe Status Foundation (Weeks 1-4)
**Focus**: Overlay icons only
- Basic shell extension with overlays
- SQLite cache infrastructure
- No context menu yet
- **Ship**: GitScribe Status v0.1.0

### Phase 1: Context Menu Core (Weeks 5-8)
**Focus**: Basic context menu operations
- Context detection system
- Core menu templates (file/repo states)
- Simple operations (stage, revert, ignore)
- App launch for complex operations
- **Ship**: GitScribe Shell v0.5.0

### Phase 2: Advanced Context Menu (Weeks 9-12)
**Focus**: Power user features
- Keyboard shortcuts throughout
- Branch management in menu
- Conflict resolution helpers
- Quick Actions Bar integration
- Performance optimizations
- **Ship**: GitScribe Shell v1.0.0

### Phase 3: Intelligence Layer (Weeks 13-16)
**Focus**: Smart features
- AI-powered commit messages in menu
- Visual diff previews in tooltips
- Recently used actions
- Predictive menu ordering
- Search within menu (Ctrl+/)
- **Ship**: GitScribe Shell v1.5.0

### Phase 4: Team Features (Months 5-6)
**Focus**: Collaboration
- Plugin menu items
- Workflow templates
- Team presence indicators
- Code review integration
- Enterprise customization
- **Ship**: GitScribe Shell v2.0.0

## Monetization Alignment

### Free Forever (Status + Basic Context Menu)
- Overlay icons
- Basic context menu operations
- Stage, unstage, revert
- Simple diff viewing
- **Strategy**: Gateway to full app

### Free Tier (Full Context Menu + Basic App)
- All context menu operations
- App handoff for complex operations
- Single repository at a time
- Core Git features
- **Strategy**: Convert to Pro

### Pro Tier ($8/month)
- Multi-repo workspace
- **AI-Powered Features**:
  - ✨ Smart commit message generation
  - ✨ AI code review before push
  - ✨ Conflict resolution suggestions
  - ✨ Pattern detection and warnings
- Advanced conflict resolution
- Quick Actions Bar with natural language
- Predictive menu ordering
- Priority support
- **Strategy**: Power user features with AI magic

### Team Tier ($15/user/month)
- Custom menu items
- Workflow templates
- Team presence
- Enterprise policies
- SSO integration
- **Strategy**: Organization features

---

## Appendix A: Command ID Allocation

Reserve command ID ranges for different menu sections:

| Range | Purpose | Count |
|-------|---------|-------|
| 0-99 | Primary actions (commit, push, pull, diff) | 100 |
| 100-199 | File operations (revert, ignore, blame) | 100 |
| 200-299 | Branch operations | 100 |
| 300-399 | Advanced operations | 100 |
| 400-499 | Settings and help | 100 |
| 500-999 | Custom commands / plugins | 500 |

This prevents ID collisions and leaves room for expansion.

---

## Appendix B: Keyboard Shortcut Guidelines

### Shortcut Principles

1. **Use standard Windows conventions**: Ctrl+C for copy (if applicable)
2. **Mnemonic**: Ctrl+**K** for **K**ommit (Git spelling)
3. **Common operations**: Single modifier (Ctrl+P)
4. **Advanced operations**: Double modifier (Ctrl+Shift+P)
5. **Avoid conflicts**: Don't use Ctrl+A (conflicts with Select All in dialogs)

### Reserved for Future

- `Ctrl+R`: Rebase
- `Ctrl+M`: Merge
- `Ctrl+T`: Tag or Test
- `Ctrl+S`: Stash (conflicts with Save in dialogs)

---

## Appendix C: Accessibility

### Screen Reader Support

- All menu items have descriptive text
- Icons have alt-text equivalents
- Keyboard shortcuts announced
- State changes announced ("Menu expanded")

### High Contrast Mode

- Icons should have high-contrast variants
- Don't rely on color alone (use icons + text)
- Test in Windows High Contrast themes

### Keyboard-Only Navigation

- All operations accessible via keyboard
- Tab order is logical
- Focus indicators are clear

---

## Appendix D: Error Messages

All error messages should be:
1. **Clear**: Explain what went wrong
2. **Actionable**: Suggest how to fix it
3. **Non-technical**: Avoid Git jargon when possible

**Bad:**
```
Error: merge conflict in file.txt
```

**Good:**
```
⚠️  Can't complete merge: Conflicts found

Git found conflicting changes in file.txt that need
your manual review.

What to do:
• Right-click file.txt → Resolve Conflict
• Or: Abort Merge to cancel
• Or: Get Help with Conflicts
```

---

## The Unified GitScribe Vision

### Three Components, One Experience

GitScribe is not three separate tools - it's one cohesive Git experience delivered through three touchpoints:

1. **Status**: The silent guardian showing state through overlay icons
2. **Context Menu**: The action gateway providing immediate operations
3. **App**: The command center for complex visual operations

### Design Harmony

All three components share:
- **Visual language**: Same icons, same colors, same meaning
- **Performance targets**: Sub-100ms for everything cached
- **Keyboard shortcuts**: Ctrl+K always means commit, everywhere
- **Intelligence**: Learn from all interactions, optimize all touchpoints
- **Settings**: Configure once, applies to all components

### The User Journey

```
Morning: Developer opens Explorer
→ Status overlays show 3 modified files (instant, cached)
→ Right-clicks one file
→ Context menu shows "📝 Commit main.cpp..." as first option (predicted)
→ Selects commit
→ App opens with file diff visible, cursor in message field (context passed)
→ Types message, Ctrl+Enter to commit
→ Status overlays update immediately (shared cache)
→ Context menu would now show "↑ Push 1 commit" if opened

Throughout the day:
→ Uses Win+G for Quick Actions without leaving IDE
→ All operations update all components instantly
→ Each interaction trains the prediction engine
```

### Success Metrics (Unified)

The three components succeed together when:
- **Latency**: Status <50ms, Context Menu <20ms, App launch <500ms
- **Adoption**: 70% of Status users try Context Menu, 50% try App
- **Retention**: 80% still using all three after 30 days
- **Delight**: Users forget they're using three components

### The Competition Can't Match This

- **TortoiseGit**: Monolithic, slow, crashes affect everything
- **SourceTree**: Just an app, no shell integration
- **GitKraken**: Just an app, no Windows integration
- **GitHub Desktop**: Just an app, no power features

Only GitScribe provides the complete trinity of visual feedback, contextual actions, and powerful operations - all perfectly integrated into Windows itself.

---

**End of GitScribe Context Menu Blueprint**

*See also: [GitScribe App Blueprint](../gitscribe-app/CLAUDE.md)*
