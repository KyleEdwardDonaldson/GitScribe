#include "ContextMenu.h"
#include "AppLauncher.h"
#include "PerformanceCache.h"
#include "PerformanceProfiler.h"
#include "GitScribeOverlay.h"
#include "resource.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <sstream>

extern HINSTANCE g_hInstance;
extern LONG g_dllRefCount;

ContextMenu::ContextMenu()
    : m_refCount(1)
    , m_idCmdFirst(0)
{
    InterlockedIncrement(&g_dllRefCount);
}

ContextMenu::~ContextMenu() {
    InterlockedDecrement(&g_dllRefCount);
}

// IUnknown methods
STDMETHODIMP ContextMenu::QueryInterface(REFIID riid, void** ppv) {
    static const QITAB qit[] = {
        QITABENT(ContextMenu, IContextMenu3),
        QITABENT(ContextMenu, IContextMenu2),
        QITABENT(ContextMenu, IContextMenu),
        QITABENT(ContextMenu, IShellExtInit),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) ContextMenu::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) ContextMenu::Release() {
    LONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

// IShellExtInit
STDMETHODIMP ContextMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID) {
    OutputDebugStringA("[GitScribe] Initialize called\n");

    if (!pdtobj) {
        OutputDebugStringA("[GitScribe] ERROR: pdtobj is NULL\n");
        return E_INVALIDARG;
    }

    // Get selected files
    FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stg = { TYMED_HGLOBAL };

    if (FAILED(pdtobj->GetData(&fmt, &stg))) {
        OutputDebugStringA("[GitScribe] ERROR: GetData failed\n");
        return E_INVALIDARG;
    }

    HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
    if (!hDrop) {
        OutputDebugStringA("[GitScribe] ERROR: GlobalLock failed\n");
        ReleaseStgMedium(&stg);
        return E_INVALIDARG;
    }

    // Get file count
    UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

    char msg[128];
    sprintf_s(msg, "[GitScribe] Initialize: %d files selected\n", fileCount);
    OutputDebugStringA(msg);

    // Get each file path
    m_selectedPaths.clear();
    for (UINT i = 0; i < fileCount; i++) {
        WCHAR szFile[MAX_PATH];
        if (DragQueryFileW(hDrop, i, szFile, ARRAYSIZE(szFile))) {
            m_selectedPaths.push_back(szFile);

            char pathMsg[MAX_PATH + 64];
            sprintf_s(pathMsg, "[GitScribe] File %d: %S\n", i, szFile);
            OutputDebugStringA(pathMsg);
        }
    }

    GlobalUnlock(stg.hGlobal);
    ReleaseStgMedium(&stg);

    OutputDebugStringA("[GitScribe] Initialize succeeded\n");
    return S_OK;
}

// IContextMenu
STDMETHODIMP ContextMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu,
                                            UINT idCmdFirst, UINT idCmdLast,
                                            UINT uFlags) {
    PROFILE_SCOPE("QueryContextMenu TOTAL");
    OutputDebugStringA("[GitScribe] QueryContextMenu called\n");

    // Notify overlay system that context menu is being shown (for Fast Mode)
    GitScribeOverlay::NotifyContextMenu();

    // Don't add menu in these cases
    if ((uFlags & CMF_DEFAULTONLY) || (uFlags & CMF_VERBSONLY)) {
        OutputDebugStringA("[GitScribe] Skipping - CMF_DEFAULTONLY or CMF_VERBSONLY\n");
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
    }

    try {
        m_idCmdFirst = idCmdFirst;

        int itemsAdded = 0;

        // Fast check: is this likely a repository? (uses cache, no libgit2)
        bool isRepo = false;
        if (!m_selectedPaths.empty()) {
            PROFILE_SCOPE("IsLikelyRepository");
            isRepo = GetCache().IsLikelyRepository(m_selectedPaths[0]);
        }

        if (isRepo) {
            OutputDebugStringA("[GitScribe] In repository - building repo menu\n");
            // For now, use FindRepository to get repo handle
            // TODO: Later we can defer this until command execution
            std::unique_ptr<GitRepository> repo;
            {
                PROFILE_SCOPE("FindRepository");
                repo = FindRepository(m_selectedPaths[0]);
            }
            if (repo && repo->IsValid()) {
                PROFILE_SCOPE("BuildSimpleMenu");
                itemsAdded = BuildSimpleMenu(hMenu, indexMenu, repo.get());
            } else {
                // Fast check was wrong, show global menu
                PROFILE_SCOPE("BuildGlobalMenu (fallback)");
                itemsAdded = BuildGlobalMenu(hMenu, indexMenu);
            }
        } else {
            OutputDebugStringA("[GitScribe] Not in repository - building global menu\n");
            PROFILE_SCOPE("BuildGlobalMenu");
            itemsAdded = BuildGlobalMenu(hMenu, indexMenu);
        }

        char msg[128];
        sprintf_s(msg, "[GitScribe] Returning %d items added\n", itemsAdded);
        OutputDebugStringA(msg);

        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(itemsAdded));
    }
    catch (...) {
        // Never crash Explorer
        OutputDebugStringA("[GitScribe] ERROR: Exception caught in QueryContextMenu\n");
        return E_FAIL;
    }
}

int ContextMenu::BuildSimpleMenu(HMENU hMenu, UINT insertPos, GitRepository* repo) {
    OutputDebugStringA("[GitScribe] BuildSimpleMenu called\n");

// GitScribe Status v0.1: Single menu item with status, clicking copies path
#ifdef GITSCRIBE_STATUS
    // Get repository status for menu title
    std::wstring menuTitle = L"GitScribe";
    if (repo) {
        try {
            RepositoryInfo info = repo->GetInfo();
            menuTitle = L"GitScribe | ";

            // Priority order: State > Conflicts > Modifications > Clean
            if (info.state == RepoState::Merging) {
                menuTitle += L"Merging";
            } else if (info.state == RepoState::Rebasing) {
                menuTitle += L"Rebasing";
            } else if (info.state == RepoState::CherryPicking) {
                menuTitle += L"Cherry-Picking";
            } else if (info.state == RepoState::Reverting) {
                menuTitle += L"Reverting";
            } else if (info.state == RepoState::Bisecting) {
                menuTitle += L"Bisecting";
            } else if (info.conflictedCount > 0) {
                menuTitle += L"Conflicted";
            } else if (!info.isClean || info.modifiedCount > 0) {
                menuTitle += L"Modified";
            } else {
                menuTitle += L"Clean";
            }
        } catch (...) {
            // Fallback to simple title on error
            menuTitle = L"GitScribe";
        }
    }

    // Insert single menu item (no submenu)
    MENUITEMINFOW mii = { sizeof(mii) };
    mii.fMask = MIIM_ID | MIIM_STRING | MIIM_BITMAP;
    mii.wID = m_idCmdFirst + CMD_STATUS; // CMD_STATUS will copy path
    mii.dwTypeData = const_cast<LPWSTR>(menuTitle.c_str());

    // Use preloaded icon (fast!)
    HBITMAP menuIcon = GetCache().GetMenuIcon();
    if (menuIcon) {
        mii.hbmpItem = menuIcon;
    }

    BOOL result = InsertMenuItemW(hMenu, insertPos, TRUE, &mii);
    if (!result) {
        OutputDebugStringA("[GitScribe] ERROR: InsertMenuItemW failed\n");
        return 0;
    }

    OutputDebugStringA("[GitScribe] Status menu item inserted successfully\n");
    return 1; // Return 1 item added to main menu

#else // Full version with submenu
    // Create GitScribe submenu
    HMENU hSubMenu = CreatePopupMenu();
    if (!hSubMenu) {
        OutputDebugStringA("[GitScribe] ERROR: CreatePopupMenu failed\n");
        return 0;
    }

    // Build simple generic menu (no Git queries, very fast)
    AddMenuItem(hSubMenu, CMD_COMMIT, L"\U0001F4DD Commit...\tCtrl+K");
    AddMenuItem(hSubMenu, CMD_PULL, L"\u21BB Pull\tCtrl+P");
    AddMenuItem(hSubMenu, CMD_PUSH, L"\u2191 Push\tCtrl+Shift+P");

    AddSeparator(hSubMenu);

    AddMenuItem(hSubMenu, CMD_HISTORY, L"\U0001F4DC History\tCtrl+L");
    AddMenuItem(hSubMenu, CMD_BRANCHES, L"\U0001F33F Branches");
    AddMenuItem(hSubMenu, CMD_STATUS, L"\U0001F4CA Status");

    AddSeparator(hSubMenu);

    AddMenuItem(hSubMenu, CMD_SETTINGS, L"GitScribe Settings...");

    // Get repository status for menu title
    std::wstring menuTitle = L"GitScribe";
    if (repo) {
        try {
            RepositoryInfo info = repo->GetInfo();
            menuTitle = L"GitScribe | ";

            // Priority order: State > Conflicts > Modifications > Clean
            if (info.state == RepoState::Merging) {
                menuTitle += L"Merging";
            } else if (info.state == RepoState::Rebasing) {
                menuTitle += L"Rebasing";
            } else if (info.state == RepoState::CherryPicking) {
                menuTitle += L"Cherry-Picking";
            } else if (info.state == RepoState::Reverting) {
                menuTitle += L"Reverting";
            } else if (info.state == RepoState::Bisecting) {
                menuTitle += L"Bisecting";
            } else if (info.conflictedCount > 0) {
                menuTitle += L"Conflicted";
            } else if (!info.isClean || info.modifiedCount > 0) {
                menuTitle += L"Modified";
            } else {
                menuTitle += L"Clean";
            }
        } catch (...) {
            // Fallback to simple title on error
            menuTitle = L"GitScribe";
        }
    }

    // Insert GitScribe submenu into main menu
    MENUITEMINFOW mii = { sizeof(mii) };
    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_BITMAP;
    // DO NOT set wID for submenu items - it confuses Windows shell
    mii.hSubMenu = hSubMenu;
    mii.dwTypeData = const_cast<LPWSTR>(menuTitle.c_str());

    // Use preloaded icon (fast!)
    HBITMAP menuIcon = GetCache().GetMenuIcon();
    if (menuIcon) {
        mii.hbmpItem = menuIcon;
    }

    BOOL result = InsertMenuItemW(hMenu, insertPos, TRUE, &mii);
    if (!result) {
        OutputDebugStringA("[GitScribe] ERROR: InsertMenuItemW failed\n");
        DestroyMenu(hSubMenu);
        return 0;
    }

    OutputDebugStringA("[GitScribe] Simple menu inserted successfully\n");
    return 1; // Return 1 item added to main menu
#endif
}

int ContextMenu::BuildGlobalMenu(HMENU hMenu, UINT insertPos) {
    OutputDebugStringA("[GitScribe] BuildGlobalMenu called\n");

    // Create GitScribe submenu for global commands
    HMENU hSubMenu = CreatePopupMenu();
    if (!hSubMenu) {
        OutputDebugStringA("[GitScribe] ERROR: CreatePopupMenu failed\n");
        return 0;
    }

    // Add global commands
    AddMenuItem(hSubMenu, CMD_CLONE, L"\U0001F4E5 Clone Repository...");
    AddMenuItem(hSubMenu, CMD_CREATE, L"\u2795 Create Repository...");
    AddSeparator(hSubMenu);
    AddMenuItem(hSubMenu, CMD_SETTINGS, L"\u2699 GitScribe Settings...");
    AddSeparator(hSubMenu);
    AddMenuItem(hSubMenu, CMD_HELP, L"\u2753 Help");
    AddMenuItem(hSubMenu, CMD_ABOUT, L"\u2139 About GitScribe");

    // Insert submenu into main menu
    MENUITEMINFOW mii = { sizeof(mii) };
    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_BITMAP;
    mii.hSubMenu = hSubMenu;
    mii.dwTypeData = const_cast<LPWSTR>(L"GitScribe");

    // Use preloaded icon (fast!)
    HBITMAP menuIcon = GetCache().GetMenuIcon();
    if (menuIcon) {
        mii.hbmpItem = menuIcon;
    }

    BOOL result = InsertMenuItemW(hMenu, insertPos, TRUE, &mii);
    if (!result) {
        OutputDebugStringA("[GitScribe] ERROR: InsertMenuItemW failed\n");
        DestroyMenu(hSubMenu);
        return 0;
    }

    OutputDebugStringA("[GitScribe] Global menu inserted successfully\n");
    return 1;
}

int ContextMenu::BuildMenu(HMENU hMenu, UINT insertPos, const MenuContext& context) {
    OutputDebugStringA("[GitScribe] BuildMenu called\n");

    // Create GitScribe submenu
    HMENU hSubMenu = CreatePopupMenu();
    if (!hSubMenu) {
        OutputDebugStringA("[GitScribe] ERROR: CreatePopupMenu failed\n");
        return 0;
    }

    OutputDebugStringA("[GitScribe] Submenu created successfully\n");

    // Build submenu items based on context
    switch (context.GetType()) {
        case ContextType::FileModified:
            OutputDebugStringA("[GitScribe] Building FileModified menu\n");
            BuildFileModifiedMenu(hSubMenu, context);
            break;

        case ContextType::FileUntracked:
            OutputDebugStringA("[GitScribe] Building FileUntracked menu\n");
            BuildFileUntrackedMenu(hSubMenu, context);
            break;

        case ContextType::RepoDirty:
            OutputDebugStringA("[GitScribe] Building RepoDirty menu\n");
            BuildRepoDirtyMenu(hSubMenu, context);
            break;

        case ContextType::RepoAhead:
            OutputDebugStringA("[GitScribe] Building RepoAhead menu\n");
            BuildRepoAheadMenu(hSubMenu, context);
            break;

        case ContextType::MergeInProgress:
            OutputDebugStringA("[GitScribe] Building Merge menu\n");
            BuildMergeMenu(hSubMenu, context);
            break;

        case ContextType::RepoClean:
            OutputDebugStringA("[GitScribe] Building RepoClean menu\n");
            BuildRepoCleanMenu(hSubMenu, context);
            break;

        default:
            OutputDebugStringA("[GitScribe] Building generic menu\n");
            // Generic menu for non-repo files
            AddMenuItem(hSubMenu, CMD_STATUS, L"Repository Status");
            break;
    }

    // Build menu label with status (format: "GitScribe | Status")
    std::wstring menuLabel = L"GitScribe";
    switch (context.GetType()) {
        case ContextType::FileModified:
            menuLabel = L"GitScribe | Modified";
            break;
        case ContextType::FileUntracked:
            menuLabel = L"GitScribe | Untracked";
            break;
        case ContextType::FileConflicted:
            menuLabel = L"GitScribe | Conflicted";
            break;
        case ContextType::RepoDirty:
            menuLabel = L"GitScribe | Modified";
            break;
        case ContextType::RepoAhead:
            menuLabel = L"GitScribe | Push Needed";
            break;
        case ContextType::MergeInProgress:
            menuLabel = L"GitScribe | Merging";
            break;
        case ContextType::RepoClean:
            menuLabel = L"GitScribe | Clean";
            break;
        default:
            menuLabel = L"GitScribe";
            break;
    }

    // Insert GitScribe submenu into main menu
    MENUITEMINFOW mii = { sizeof(mii) };
    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_BITMAP;
    // DO NOT set wID for submenu items - it confuses Windows shell
    mii.hSubMenu = hSubMenu;
    mii.dwTypeData = const_cast<LPWSTR>(menuLabel.c_str());

    // Use preloaded icon (fast!)
    HBITMAP menuIcon = GetCache().GetMenuIcon();
    if (menuIcon) {
        mii.hbmpItem = menuIcon;
    }

    BOOL result = InsertMenuItemW(hMenu, insertPos, TRUE, &mii);
    if (!result) {
        OutputDebugStringA("[GitScribe] ERROR: InsertMenuItemW failed\n");
        DestroyMenu(hSubMenu);
        return 0;
    }

    OutputDebugStringA("[GitScribe] Menu inserted successfully\n");
    return 1; // Return 1 item added to main menu
}

void ContextMenu::BuildFileModifiedMenu(HMENU hMenu, const MenuContext& ctx) {
    std::wstring fileName = ctx.GetPrimaryFileName();

    // Primary action
    AddMenuItem(hMenu, CMD_COMMIT, L"\U0001F4DD Commit \"" + fileName + L"\"...\tCtrl+K");

    // Common actions
    AddMenuItem(hMenu, CMD_DIFF, L"\U0001F4CA Diff with HEAD\tCtrl+D");
    AddMenuItem(hMenu, CMD_REVERT, L"\u21A9\uFE0F Revert changes...");

    AddSeparator(hMenu);

    // View operations
    AddMenuItem(hMenu, CMD_HISTORY, L"\U0001F4DC Show History\tCtrl+L");
    AddMenuItem(hMenu, CMD_BLAME, L"\U0001F3F7\uFE0F Blame\tCtrl+B");

    AddSeparator(hMenu);

    // GitScribe submenu (placeholder)
    AddMenuItem(hMenu, CMD_SETTINGS, L"GitScribe Settings...");
}

void ContextMenu::BuildFileUntrackedMenu(HMENU hMenu, const MenuContext& ctx) {
    std::wstring fileName = ctx.GetPrimaryFileName();

    AddMenuItem(hMenu, CMD_ADD, L"\u2795 Add \"" + fileName + L"\" to Git\tCtrl+A");
    AddMenuItem(hMenu, CMD_IGNORE, L"\U0001F6AB Ignore \"" + fileName + L"\"");

    AddSeparator(hMenu);

    AddMenuItem(hMenu, CMD_SETTINGS, L"GitScribe Settings...");
}

void ContextMenu::BuildRepoDirtyMenu(HMENU hMenu, const MenuContext& ctx) {
    const auto& info = ctx.GetRepoInfo();

    std::wstring commitText = FormatCount(L"\U0001F4DD Commit {0} changed files...\tCtrl+K", info.modifiedCount);
    AddMenuItem(hMenu, CMD_COMMIT, commitText);

    AddMenuItem(hMenu, CMD_DIFF, L"\U0001F4CA Show All Changes\tCtrl+D");
    AddMenuItem(hMenu, CMD_PULL, L"\u21BB Pull from origin/" + info.currentBranch + L"\tCtrl+P");

    if (info.aheadCount > 0) {
        std::wstring pushText = FormatCount(L"\u2191 Push {0} commits\tCtrl+Shift+P", info.aheadCount);
        AddMenuItem(hMenu, CMD_PUSH, pushText);
    }

    AddSeparator(hMenu);

    AddMenuItem(hMenu, CMD_BRANCHES, L"\U0001F33F Branches (" + info.currentBranch + L" \u2B50)");
    AddMenuItem(hMenu, CMD_HISTORY, L"\U0001F4DC Repository History\tCtrl+L");

    AddSeparator(hMenu);

    AddMenuItem(hMenu, CMD_SETTINGS, L"GitScribe Settings...");
}

void ContextMenu::BuildRepoAheadMenu(HMENU hMenu, const MenuContext& ctx) {
    const auto& info = ctx.GetRepoInfo();

    std::wstring pushText = FormatCount(L"\u2191 Push {0} commits to origin/" + info.currentBranch + L"\tCtrl+Shift+P", info.aheadCount);
    AddMenuItem(hMenu, CMD_PUSH, pushText);

    AddMenuItem(hMenu, CMD_SYNC, L"\U0001F504 Sync (pull then push)\tCtrl+Y");

    AddSeparator(hMenu);

    AddMenuItem(hMenu, CMD_PULL, L"\u21BB Pull from origin/" + info.currentBranch + L"\tCtrl+P");
    AddMenuItem(hMenu, CMD_HISTORY, L"\U0001F4DC Repository History\tCtrl+L");

    AddSeparator(hMenu);

    AddMenuItem(hMenu, CMD_SETTINGS, L"GitScribe Settings...");
}

void ContextMenu::BuildMergeMenu(HMENU hMenu, const MenuContext& ctx) {
    const auto& info = ctx.GetRepoInfo();

    AddMenuItem(hMenu, 0, L"\u26A0\uFE0F MERGE IN PROGRESS");

    AddSeparator(hMenu);

    if (info.conflictedCount > 0) {
        std::wstring text = FormatCount(L"\u2705 Resolve Conflicts ({0} files)...", info.conflictedCount);
        AddMenuItem(hMenu, CMD_RESOLVE, text);
    } else {
        AddMenuItem(hMenu, CMD_COMMIT, L"\u2705 Continue Merge");
    }

    AddMenuItem(hMenu, CMD_REVERT, L"\u274C Abort Merge");

    AddSeparator(hMenu);

    AddMenuItem(hMenu, CMD_STATUS, L"\U0001F4CA Show Status");
}

void ContextMenu::BuildRepoCleanMenu(HMENU hMenu, const MenuContext& ctx) {
    const auto& info = ctx.GetRepoInfo();

    // Pull/Fetch operations
    AddMenuItem(hMenu, CMD_PULL, L"\u21BB Pull from origin/" + info.currentBranch + L"\tCtrl+P");
    AddMenuItem(hMenu, CMD_SYNC, L"\U0001F504 Sync (pull then push)\tCtrl+Y");

    AddSeparator(hMenu);

    // Repository exploration
    AddMenuItem(hMenu, CMD_HISTORY, L"\U0001F4DC Repository History\tCtrl+L");
    AddMenuItem(hMenu, CMD_BRANCHES, L"\U0001F33F Branches (" + info.currentBranch + L" \u2B50)");
    AddMenuItem(hMenu, CMD_STATUS, L"\U0001F4CA Repository Status");

    AddSeparator(hMenu);

    // Settings
    AddMenuItem(hMenu, CMD_SETTINGS, L"GitScribe Settings...");
}

STDMETHODIMP ContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici) {
    // Check if command is from our menu
    if (HIWORD(pici->lpVerb)) {
        // String verb - not supported yet
        return E_INVALIDARG;
    }

    try {
        UINT cmdId = LOWORD(pici->lpVerb);

        // Validate command ID is in our range
        if (cmdId >= CMD_MAX) {
            OutputDebugStringA("[GitScribe] InvokeCommand: Command ID out of range\n");
            return E_INVALIDARG;
        }

        char msg[128];
        sprintf_s(msg, "[GitScribe] InvokeCommand: cmdId=%d\n", cmdId);
        OutputDebugStringA(msg);

        switch (cmdId) {
            case CMD_COMMIT:
                OutputDebugStringA("[GitScribe] Executing CMD_COMMIT\n");
                OnCommit();
                break;

            case CMD_DIFF:
                OutputDebugStringA("[GitScribe] Executing CMD_DIFF\n");
                OnDiff();
                break;

            case CMD_PUSH:
                OutputDebugStringA("[GitScribe] Executing CMD_PUSH\n");
                OnPush();
                break;

            case CMD_PULL:
                OutputDebugStringA("[GitScribe] Executing CMD_PULL\n");
                OnPull();
                break;

            case CMD_CLONE:
                OutputDebugStringA("[GitScribe] Executing CMD_CLONE\n");
                MessageBoxW(pici->hwnd, L"Clone Repository dialog will open here.\n\nThis feature is coming soon!",
                           L"GitScribe - Clone Repository", MB_OK | MB_ICONINFORMATION);
                break;

            case CMD_CREATE:
                OutputDebugStringA("[GitScribe] Executing CMD_CREATE\n");
                MessageBoxW(pici->hwnd, L"Create Repository dialog will open here.\n\nThis feature is coming soon!",
                           L"GitScribe - Create Repository", MB_OK | MB_ICONINFORMATION);
                break;

            case CMD_SETTINGS:
                OutputDebugStringA("[GitScribe] Executing CMD_SETTINGS\n");
                MessageBoxW(pici->hwnd, L"GitScribe Settings will open here.\n\nThis feature is coming soon!",
                           L"GitScribe Settings", MB_OK | MB_ICONINFORMATION);
                break;

            case CMD_HELP:
                OutputDebugStringA("[GitScribe] Executing CMD_HELP\n");
                ShellExecuteW(NULL, L"open", L"https://gitscribe.dev/docs", NULL, NULL, SW_SHOWNORMAL);
                break;

            case CMD_ABOUT:
                OutputDebugStringA("[GitScribe] Executing CMD_ABOUT\n");
                MessageBoxW(pici->hwnd, L"GitScribe v0.1.0\n\nA luxury Git client for Windows\n\nCopyright \u00A9 2025",
                           L"About GitScribe", MB_OK | MB_ICONINFORMATION);
                break;

            case CMD_STATUS:
                OutputDebugStringA("[GitScribe] Executing CMD_STATUS\n");
                // Copy selected file path to clipboard
                if (!m_selectedPaths.empty()) {
                    std::wstring pathToCopy = m_selectedPaths[0];

                    // Open clipboard
                    if (OpenClipboard(pici->hwnd)) {
                        EmptyClipboard();

                        // Allocate global memory for the path
                        size_t size = (pathToCopy.length() + 1) * sizeof(WCHAR);
                        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);

                        if (hGlobal) {
                            LPWSTR pGlobal = (LPWSTR)GlobalLock(hGlobal);
                            if (pGlobal) {
                                wcscpy_s(pGlobal, pathToCopy.length() + 1, pathToCopy.c_str());
                                GlobalUnlock(hGlobal);
                                SetClipboardData(CF_UNICODETEXT, hGlobal);
                            }
                        }

                        CloseClipboard();
                        OutputDebugStringA("[GitScribe] Path copied to clipboard\n");
                    }
                }
                break;

            default:
                sprintf_s(msg, "[GitScribe] Command %d not implemented\n", cmdId);
                OutputDebugStringA(msg);
                MessageBoxW(pici->hwnd, L"This operation is not yet implemented.",
                           L"GitScribe", MB_OK | MB_ICONINFORMATION);
                break;
        }

        return S_OK;
    }
    catch (...) {
        OutputDebugStringA("[GitScribe] InvokeCommand: Exception caught\n");
        return E_FAIL;
    }
}

STDMETHODIMP ContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType,
                                            UINT* pReserved, LPSTR pszName,
                                            UINT cchMax) {
    // Return help strings for menu items
    if (uType == GCS_HELPTEXTW) {
        LPCWSTR helpText = nullptr;

        switch (idCmd) {
            case CMD_COMMIT:
                helpText = L"Commit selected files to the repository";
                break;
            case CMD_DIFF:
                helpText = L"Show differences from HEAD";
                break;
            case CMD_PUSH:
                helpText = L"Push commits to remote repository";
                break;
            case CMD_PULL:
                helpText = L"Pull changes from remote repository";
                break;
        }

        if (helpText) {
            StringCchCopyW(reinterpret_cast<LPWSTR>(pszName), cchMax, helpText);
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

STDMETHODIMP ContextMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT lResult;
    return HandleMenuMsg2(uMsg, wParam, lParam, &lResult);
}

STDMETHODIMP ContextMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult) {
    // Handle owner-drawn menu messages if needed
    // For now, we use standard menus
    return S_OK;
}

// Helper to launch Git command in terminal
void LaunchGitCommand(const std::wstring& repoPath, const std::wstring& command) {
    // Create command line to launch git in a new terminal window
    std::wstring cmdLine = L"cmd.exe /k \"cd /d \"" + repoPath + L"\" && git " + command + L" && pause\"";

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    if (CreateProcessW(NULL, const_cast<LPWSTR>(cmdLine.c_str()), NULL, NULL, FALSE,
                       CREATE_NEW_CONSOLE, NULL, repoPath.c_str(), &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

// Helper to get repository path from selected file
std::wstring GetRepoPathFromFile(const std::wstring& filePath) {
    auto repo = FindRepository(filePath);
    if (repo && repo->IsValid()) {
        return repo->GetPath();
    }
    return L"";
}

// Command handlers
void ContextMenu::OnCommit() {
    if (m_selectedPaths.empty()) return;

    std::wstring repoPath = GetRepoPathFromFile(m_selectedPaths[0]);
    if (repoPath.empty()) {
        MessageBoxW(NULL, L"Not in a Git repository", L"GitScribe", MB_OK | MB_ICONWARNING);
        return;
    }

    // Try to launch GitScribe app
    AppContext ctx;
    ctx.repoPath = repoPath;
    ctx.files = m_selectedPaths;

    auto repo = FindRepository(m_selectedPaths[0]);
    if (repo && repo->IsValid()) {
        auto info = repo->GetInfo();
        ctx.branch = info.currentBranch;
    }

    // Attempt to launch app
    if (!AppLauncher::Launch(AppAction::Commit, ctx)) {
        // App not available - fallback to git command
        LaunchGitCommand(repoPath, L"status");
    }
}

void ContextMenu::OnDiff() {
    if (m_selectedPaths.empty()) return;

    std::wstring repoPath = GetRepoPathFromFile(m_selectedPaths[0]);
    if (repoPath.empty()) return;

    // Try to launch GitScribe app for diff
    AppContext ctx;
    ctx.repoPath = repoPath;
    ctx.files = m_selectedPaths;

    if (!AppLauncher::Launch(AppAction::Diff, ctx)) {
        // Fallback to git command
        LaunchGitCommand(repoPath, L"diff");
    }
}

void ContextMenu::OnPush() {
    if (m_selectedPaths.empty()) return;

    std::wstring repoPath = GetRepoPathFromFile(m_selectedPaths[0]);
    if (repoPath.empty()) return;

    // Get repository info to show what will be pushed
    auto repo = FindRepository(m_selectedPaths[0]);
    if (repo && repo->IsValid()) {
        auto info = repo->GetInfo();

        std::wstring message = L"Push " + std::to_wstring(info.aheadCount) + L" commits to remote?\n\n";
        message += L"Branch: " + info.currentBranch + L"\n\n";
        message += L"This will open a terminal window to execute the push.";

        int result = MessageBoxW(NULL, message.c_str(), L"GitScribe - Push",
                                MB_OKCANCEL | MB_ICONQUESTION);

        if (result == IDOK) {
            LaunchGitCommand(repoPath, L"push");
        }
    }
}

void ContextMenu::OnPull() {
    if (m_selectedPaths.empty()) return;

    std::wstring repoPath = GetRepoPathFromFile(m_selectedPaths[0]);
    if (repoPath.empty()) return;

    std::wstring message = L"Pull latest changes from remote?\n\n";
    message += L"Repository: " + repoPath + L"\n\n";
    message += L"This will open a terminal window to execute the pull.";

    int result = MessageBoxW(NULL, message.c_str(), L"GitScribe - Pull",
                           MB_OKCANCEL | MB_ICONQUESTION);

    if (result == IDOK) {
        LaunchGitCommand(repoPath, L"pull");
    }
}

// Helper methods
void ContextMenu::AddMenuItem(HMENU hMenu, UINT cmdId, const std::wstring& text) {
    MENUITEMINFOW mii = { sizeof(mii) };
    mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
    mii.fType = MFT_STRING;
    mii.fState = (cmdId == 0) ? MFS_DISABLED : MFS_ENABLED;
    mii.wID = m_idCmdFirst + cmdId;
    mii.dwTypeData = const_cast<LPWSTR>(text.c_str());

    InsertMenuItemW(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);
}

void ContextMenu::AddSeparator(HMENU hMenu) {
    MENUITEMINFOW mii = { sizeof(mii) };
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_SEPARATOR;

    InsertMenuItemW(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);
}

std::wstring ContextMenu::FormatCount(const std::wstring& format, unsigned int count) {
    std::wstring result = format;
    size_t pos = result.find(L"{0}");
    if (pos != std::wstring::npos) {
        result.replace(pos, 3, std::to_wstring(count));
    }
    return result;
}
