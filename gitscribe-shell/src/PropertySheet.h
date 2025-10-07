#pragma once

#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include "GitRepository.h"

// Property sheet extension showing Git info in file Properties dialog
class GitPropSheet : public IShellExtInit, public IShellPropSheetExt {
public:
    GitPropSheet();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IShellExtInit
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID);

    // IShellPropSheetExt
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

private:
    ULONG m_refCount;
    std::wstring m_filePath;
    std::unique_ptr<GitRepository> m_repo;

    // Dialog procedure for our property page
    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Initialize dialog controls with Git info
    void InitializeControls(HWND hwndDlg);

    // Format repository info for display
    std::wstring FormatRepoInfo();
    std::wstring FormatFileStatus();
    std::wstring FormatBranchInfo();
};
