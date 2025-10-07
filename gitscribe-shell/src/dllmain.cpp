#include <windows.h>
#include <shlobj.h>
#include <new>
#include <strsafe.h>
#include "ClassFactory.h"
#include "PerformanceCache.h"

// Global DLL instance and ref count
HINSTANCE g_hInstance = nullptr;
LONG g_dllRefCount = 0;

// GUIDs (must match ClassFactory.cpp)
static const CLSID CLSID_ModifiedOverlay =
    { 0xF4C4A301, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };
static const CLSID CLSID_CleanOverlay =
    { 0xF4C4A302, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 } };
static const CLSID CLSID_AddedOverlay =
    { 0xF4C4A303, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 } };
static const CLSID CLSID_UntrackedOverlay =
    { 0xF4C4A304, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 } };
static const CLSID CLSID_ConflictedOverlay =
    { 0xF4C4A305, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05 } };
static const CLSID CLSID_IgnoredOverlay =
    { 0xF4C4A306, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06 } };
static const CLSID CLSID_ContextMenu =
    { 0xF4C4A310, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10 } };
static const CLSID CLSID_PropertySheet =
    { 0x8F4E0E50, 0x7B2D, 0x4A1E, { 0x9C, 0x3F, 0x1D, 0x2E, 0x3F, 0x4A, 0x5B, 0x6C } };

// Helper to convert CLSID to string
HRESULT GuidToString(REFCLSID clsid, LPWSTR pszGuid, int cchMax) {
    return StringCchPrintfW(pszGuid, cchMax,
        L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        clsid.Data1, clsid.Data2, clsid.Data3,
        clsid.Data4[0], clsid.Data4[1], clsid.Data4[2], clsid.Data4[3],
        clsid.Data4[4], clsid.Data4[5], clsid.Data4[6], clsid.Data4[7]);
}

// Helper to register a CLSID
HRESULT RegisterCLSID(REFCLSID clsid, LPCWSTR pszDescription) {
    WCHAR szCLSID[64];
    WCHAR szSubkey[MAX_PATH];
    WCHAR szModule[MAX_PATH];

    if (FAILED(GuidToString(clsid, szCLSID, ARRAYSIZE(szCLSID)))) {
        return E_FAIL;
    }

    GetModuleFileNameW(g_hInstance, szModule, ARRAYSIZE(szModule));

    // Register CLSID
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)pszDescription, (wcslen(pszDescription) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);

    // Register InprocServer32
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s\\InprocServer32", szCLSID);

    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szModule, (wcslen(szModule) + 1) * sizeof(WCHAR));
    RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (BYTE*)L"Apartment", 10 * sizeof(WCHAR));
    RegCloseKey(hKey);

    return S_OK;
}

// Helper to unregister a CLSID
HRESULT UnregisterCLSID(REFCLSID clsid) {
    WCHAR szCLSID[64];
    WCHAR szSubkey[MAX_PATH];

    if (FAILED(GuidToString(clsid, szCLSID, ARRAYSIZE(szCLSID)))) {
        return E_FAIL;
    }

    // Delete InprocServer32
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s\\InprocServer32", szCLSID);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, szSubkey);

    // Delete CLSID
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, szSubkey);

    return S_OK;
}

// Helper to register overlay handler
HRESULT RegisterOverlay(LPCWSTR pszName, REFCLSID clsid) {
    WCHAR szCLSID[64];
    WCHAR szSubkey[MAX_PATH];

    if (FAILED(GuidToString(clsid, szCLSID, ARRAYSIZE(szCLSID)))) {
        return E_FAIL;
    }

    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey),
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%s",
        pszName);

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szCLSID, (wcslen(szCLSID) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);

    return S_OK;
}

// Helper to unregister overlay handler
HRESULT UnregisterOverlay(LPCWSTR pszName) {
    WCHAR szSubkey[MAX_PATH];

    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey),
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%s",
        pszName);

    RegDeleteKeyW(HKEY_LOCAL_MACHINE, szSubkey);
    return S_OK;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hModule;
            DisableThreadLibraryCalls(hModule);
            // Preload icons for fast menu display
            GetCache().PreloadIcons();
            break;
        case DLL_PROCESS_DETACH:
            // Clean up resources
            GetCache().ReleaseIcons();
            break;
    }
    return TRUE;
}

// Can DLL be unloaded?
STDAPI DllCanUnloadNow() {
    return (g_dllRefCount == 0) ? S_OK : S_FALSE;
}

// Get class factory
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    if (!ppv) return E_INVALIDARG;

    *ppv = nullptr;

    // Create class factory for the requested CLSID
    ClassFactory* factory = new (std::nothrow) ClassFactory(rclsid);
    if (!factory) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = factory->QueryInterface(riid, ppv);
    factory->Release();

    return hr;
}

// Helper to register context menu handler
HRESULT RegisterContextMenu(REFCLSID clsid) {
    WCHAR szCLSID[64];
    WCHAR szSubkey[MAX_PATH];

    if (FAILED(GuidToString(clsid, szCLSID, ARRAYSIZE(szCLSID)))) {
        return E_FAIL;
    }

    // Register for all files (*) - use leading spaces for high priority
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey),
        L"*\\shellex\\ContextMenuHandlers\\  GitScribe");

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szCLSID, (wcslen(szCLSID) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);

    // Also register for directories
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey),
        L"Directory\\shellex\\ContextMenuHandlers\\  GitScribe");

    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szCLSID, (wcslen(szCLSID) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);

    // And for directory background
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey),
        L"Directory\\Background\\shellex\\ContextMenuHandlers\\  GitScribe");

    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szCLSID, (wcslen(szCLSID) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);

    return S_OK;
}

// Helper to unregister context menu handler
HRESULT UnregisterContextMenu() {
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"*\\shellex\\ContextMenuHandlers\\  GitScribe");
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"Directory\\shellex\\ContextMenuHandlers\\  GitScribe");
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"Directory\\Background\\shellex\\ContextMenuHandlers\\  GitScribe");
    return S_OK;
}

// Helper to register property sheet handler
HRESULT RegisterPropertySheet(REFCLSID clsid) {
    WCHAR szCLSID[64];
    WCHAR szSubkey[MAX_PATH];

    if (FAILED(GuidToString(clsid, szCLSID, ARRAYSIZE(szCLSID)))) {
        return E_FAIL;
    }

    // Register for all files (*) - use leading spaces for high priority
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey),
        L"*\\shellex\\PropertySheetHandlers\\  GitScribe");

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szCLSID, (wcslen(szCLSID) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);

    // Also register for directories
    StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey),
        L"Directory\\shellex\\PropertySheetHandlers\\  GitScribe");

    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szCLSID, (wcslen(szCLSID) + 1) * sizeof(WCHAR));
    RegCloseKey(hKey);

    return S_OK;
}

// Helper to unregister property sheet handler
HRESULT UnregisterPropertySheet() {
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"*\\shellex\\PropertySheetHandlers\\  GitScribe");
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"Directory\\shellex\\PropertySheetHandlers\\  GitScribe");
    return S_OK;
}

// Register server
STDAPI DllRegisterServer() {
    HRESULT hr;

    // Register all CLSIDs
    hr = RegisterCLSID(CLSID_ModifiedOverlay, L"GitScribe Modified Overlay");
    if (FAILED(hr)) return hr;

    hr = RegisterCLSID(CLSID_CleanOverlay, L"GitScribe Clean Overlay");
    if (FAILED(hr)) return hr;

    hr = RegisterCLSID(CLSID_AddedOverlay, L"GitScribe Added Overlay");
    if (FAILED(hr)) return hr;

    hr = RegisterCLSID(CLSID_UntrackedOverlay, L"GitScribe Untracked Overlay");
    if (FAILED(hr)) return hr;

    hr = RegisterCLSID(CLSID_ConflictedOverlay, L"GitScribe Conflicted Overlay");
    if (FAILED(hr)) return hr;

    hr = RegisterCLSID(CLSID_IgnoredOverlay, L"GitScribe Ignored Overlay");
    if (FAILED(hr)) return hr;

    // Register context menu
    hr = RegisterCLSID(CLSID_ContextMenu, L"GitScribe Context Menu");
    if (FAILED(hr)) return hr;

    // Register property sheet
    hr = RegisterCLSID(CLSID_PropertySheet, L"GitScribe Property Sheet");
    if (FAILED(hr)) return hr;

    // Register overlay handlers (prefix with space for high priority)
    hr = RegisterOverlay(L" GitScribeModified", CLSID_ModifiedOverlay);
    if (FAILED(hr)) return hr;

    hr = RegisterOverlay(L" GitScribeClean", CLSID_CleanOverlay);
    if (FAILED(hr)) return hr;

    hr = RegisterOverlay(L" GitScribeAdded", CLSID_AddedOverlay);
    if (FAILED(hr)) return hr;

    hr = RegisterOverlay(L" GitScribeUntracked", CLSID_UntrackedOverlay);
    if (FAILED(hr)) return hr;

    hr = RegisterOverlay(L" GitScribeConflicted", CLSID_ConflictedOverlay);
    if (FAILED(hr)) return hr;

    hr = RegisterOverlay(L" GitScribeIgnored", CLSID_IgnoredOverlay);
    if (FAILED(hr)) return hr;

    // Register context menu handler
    hr = RegisterContextMenu(CLSID_ContextMenu);
    if (FAILED(hr)) return hr;

    // Register property sheet handler
    hr = RegisterPropertySheet(CLSID_PropertySheet);
    if (FAILED(hr)) return hr;

    // Notify shell of changes
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return S_OK;
}

// Unregister server
STDAPI DllUnregisterServer() {
    // Unregister overlay handlers
    UnregisterOverlay(L" GitScribeModified");
    UnregisterOverlay(L" GitScribeClean");
    UnregisterOverlay(L" GitScribeAdded");
    UnregisterOverlay(L" GitScribeUntracked");
    UnregisterOverlay(L" GitScribeConflicted");
    UnregisterOverlay(L" GitScribeIgnored");

    // Unregister context menu and property sheet
    UnregisterContextMenu();
    UnregisterPropertySheet();

    // Unregister CLSIDs
    UnregisterCLSID(CLSID_ModifiedOverlay);
    UnregisterCLSID(CLSID_CleanOverlay);
    UnregisterCLSID(CLSID_AddedOverlay);
    UnregisterCLSID(CLSID_UntrackedOverlay);
    UnregisterCLSID(CLSID_ConflictedOverlay);
    UnregisterCLSID(CLSID_IgnoredOverlay);
    UnregisterCLSID(CLSID_ContextMenu);
    UnregisterCLSID(CLSID_PropertySheet);

    // Notify shell of changes
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return S_OK;
}
