#include "PropertySheet.h"
#include "resource.h"
#include "GitScribeOverlay.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>

#pragma comment(lib, "comctl32.lib")

extern HINSTANCE g_hInstance;
extern LONG g_dllRefCount;

GitPropSheet::GitPropSheet()
    : m_refCount(1) {
    InterlockedIncrement(&g_dllRefCount);
}

// IUnknown
STDMETHODIMP GitPropSheet::QueryInterface(REFIID riid, void** ppv) {
    static const QITAB qit[] = {
        QITABENT(GitPropSheet, IShellExtInit),
        QITABENT(GitPropSheet, IShellPropSheetExt),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) GitPropSheet::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) GitPropSheet::Release() {
    LONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

// IShellExtInit
STDMETHODIMP GitPropSheet::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID) {
    if (!pdtobj) {
        return E_INVALIDARG;
    }

    // Get selected file
    FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stg = { TYMED_HGLOBAL };

    if (FAILED(pdtobj->GetData(&fmt, &stg))) {
        return E_INVALIDARG;
    }

    HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
    if (!hDrop) {
        ReleaseStgMedium(&stg);
        return E_INVALIDARG;
    }

    // Get first file only
    WCHAR szFile[MAX_PATH];
    if (DragQueryFileW(hDrop, 0, szFile, ARRAYSIZE(szFile))) {
        m_filePath = szFile;

        // Try to open repository
        m_repo = FindRepository(m_filePath);
    }

    GlobalUnlock(stg.hGlobal);
    ReleaseStgMedium(&stg);

    // Only show property page if we're in a Git repository
    if (!m_repo || !m_repo->IsValid()) {
        return E_FAIL;
    }

    return S_OK;
}

// IShellPropSheetExt
STDMETHODIMP GitPropSheet::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam) {
    try {
        PROPSHEETPAGEW psp = { sizeof(psp) };
        psp.dwFlags = PSP_USEREFPARENT | PSP_USETITLE | PSP_DEFAULT;
        psp.hInstance = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCEW(IDD_GIT_PROPPAGE);
        psp.pfnDlgProc = DialogProc;
        psp.pszTitle = L"Git";
        psp.lParam = reinterpret_cast<LPARAM>(this);
        psp.pcRefParent = (UINT*)&g_dllRefCount;

        HPROPSHEETPAGE hPage = CreatePropertySheetPageW(&psp);
        if (hPage) {
            if (lpfnAddPage(hPage, lParam)) {
                AddRef();  // Page holds a reference
                return S_OK;
            }
            DestroyPropertySheetPage(hPage);
        }

        return E_FAIL;
    }
    catch (...) {
        return E_FAIL;
    }
}

STDMETHODIMP GitPropSheet::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam) {
    // We don't replace any pages
    return E_NOTIMPL;
}

// Dialog procedure
INT_PTR CALLBACK GitPropSheet::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GitPropSheet* pThis = nullptr;

    if (uMsg == WM_INITDIALOG) {
        LPPROPSHEETPAGE psp = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
        pThis = reinterpret_cast<GitPropSheet*>(psp->lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pThis));
        pThis->InitializeControls(hwndDlg);
        return TRUE;
    }
    else {
        pThis = reinterpret_cast<GitPropSheet*>(GetWindowLongPtr(hwndDlg, DWLP_USER));
    }

    if (!pThis) {
        return FALSE;
    }

    switch (uMsg) {
        case WM_DESTROY:
            pThis->Release();  // Release the reference held by the page
            return FALSE;

        case WM_NOTIFY: {
            LPNMHDR pnmh = reinterpret_cast<LPNMHDR>(lParam);
            if (pnmh->code == PSN_APPLY) {
                // No changes to apply (read-only page)
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}

void GitPropSheet::InitializeControls(HWND hwndDlg) {
    if (!m_repo || !m_repo->IsValid()) {
        SetDlgItemTextW(hwndDlg, IDC_GIT_STATUS, L"Not in a Git repository");
        return;
    }

    // Get repository info
    auto info = m_repo->GetInfo();

    // Repository path
    SetDlgItemTextW(hwndDlg, IDC_GIT_REPO_PATH, m_repo->GetPath().c_str());

    // Current branch
    SetDlgItemTextW(hwndDlg, IDC_GIT_BRANCH, info.currentBranch.c_str());

    // Repository state
    std::wstring stateText;
    switch (info.state) {
        case 0: stateText = info.isClean ? L"Clean" : L"Modified"; break;
        case 1: stateText = L"Merging"; break;
        case 2: stateText = L"Rebasing"; break;
        case 3: stateText = L"Cherry-picking"; break;
        case 4: stateText = L"Reverting"; break;
        case 5: stateText = L"Bisecting"; break;
        default: stateText = L"Unknown"; break;
    }
    SetDlgItemTextW(hwndDlg, IDC_GIT_STATE, stateText.c_str());

    // Modified files count
    std::wstring modifiedText = std::to_wstring(info.modifiedCount) + L" modified files";
    SetDlgItemTextW(hwndDlg, IDC_GIT_MODIFIED, modifiedText.c_str());

    // Ahead/behind
    std::wstring syncStatus;
    if (info.aheadCount > 0 && info.behindCount > 0) {
        syncStatus = std::to_wstring(info.aheadCount) + L" ahead, " +
                     std::to_wstring(info.behindCount) + L" behind";
    }
    else if (info.aheadCount > 0) {
        syncStatus = std::to_wstring(info.aheadCount) + L" commits ahead";
    }
    else if (info.behindCount > 0) {
        syncStatus = std::to_wstring(info.behindCount) + L" commits behind";
    }
    else {
        syncStatus = L"Up to date";
    }
    SetDlgItemTextW(hwndDlg, IDC_GIT_SYNC, syncStatus.c_str());

    // File-specific status
    GitStatus fileStatus = m_repo->GetFileStatus(m_filePath);
    std::wstring fileStatusText;
    switch (fileStatus) {
        case GitStatus::Clean:      fileStatusText = L"Unmodified"; break;
        case GitStatus::Modified:   fileStatusText = L"Modified"; break;
        case GitStatus::Added:      fileStatusText = L"Added"; break;
        case GitStatus::Untracked:  fileStatusText = L"Untracked"; break;
        case GitStatus::Conflicted: fileStatusText = L"Conflicted"; break;
        case GitStatus::Ignored:    fileStatusText = L"Ignored"; break;
        default:                    fileStatusText = L"Unknown"; break;
    }
    SetDlgItemTextW(hwndDlg, IDC_GIT_FILE_STATUS, fileStatusText.c_str());
}
