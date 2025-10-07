#pragma once

#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <map>
#include "MenuContext.h"

// Forward declarations
struct MenuItem;

// Command IDs (must fit within idCmdFirst..idCmdLast range)
enum CommandID {
    CMD_COMMIT = 0,
    CMD_DIFF,
    CMD_REVERT,
    CMD_PULL,
    CMD_PUSH,
    CMD_SYNC,
    CMD_ADD,
    CMD_IGNORE,
    CMD_RESOLVE,
    CMD_HISTORY,
    CMD_BLAME,
    CMD_BRANCHES,
    CMD_STATUS,
    CMD_CLONE,
    CMD_CREATE,
    CMD_SETTINGS,
    CMD_HELP,
    CMD_ABOUT,
    CMD_MAX
};

// Context menu implementation
class ContextMenu : public IContextMenu3, public IShellExtInit {
public:
    ContextMenu();
    virtual ~ContextMenu();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IShellExtInit
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID);

    // IContextMenu
    STDMETHOD(QueryContextMenu)(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax);

    // IContextMenu2 (for owner-drawn menus)
    STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);

    // IContextMenu3 (extended owner-drawn menus)
    STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

private:
    LONG m_refCount;
    std::vector<std::wstring> m_selectedPaths;
    UINT m_idCmdFirst;

    // Menu building
    int BuildSimpleMenu(HMENU hMenu, UINT insertPos, GitRepository* repo);
    int BuildGlobalMenu(HMENU hMenu, UINT insertPos);
    int BuildMenu(HMENU hMenu, UINT insertPos, const MenuContext& context);
    void BuildFileModifiedMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildFileUntrackedMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildRepoAheadMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildRepoDirtyMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildRepoCleanMenu(HMENU hMenu, const MenuContext& ctx);
    void BuildMergeMenu(HMENU hMenu, const MenuContext& ctx);

    // Command handlers
    void OnCommit();
    void OnDiff();
    void OnPush();
    void OnPull();

    // Helper to add menu item
    void AddMenuItem(HMENU hMenu, UINT cmdId, const std::wstring& text);
    void AddSeparator(HMENU hMenu);

    // Helper to format text with counts
    std::wstring FormatCount(const std::wstring& format, unsigned int count);
};
