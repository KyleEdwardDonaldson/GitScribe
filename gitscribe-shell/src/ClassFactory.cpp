#include "ClassFactory.h"
#include "GitScribeOverlay.h"
#include "ContextMenu.h"
#include "PropertySheet.h"
#include <new>

// GUIDs for each overlay handler
// {F4C4A300-xxxx-4xxx-xxxx-xxxxxxxxxxxx} - GitScribe overlays
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

// Context menu handler
static const CLSID CLSID_ContextMenu =
    { 0xF4C4A310, 0x0000, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10 } };

// Property sheet handler
// {8F4E0E50-7B2D-4A1E-9C3F-1D2E3F4A5B6C}
static const CLSID CLSID_PropertySheet =
    { 0x8F4E0E50, 0x7B2D, 0x4A1E, { 0x9C, 0x3F, 0x1D, 0x2E, 0x3F, 0x4A, 0x5B, 0x6C } };

extern LONG g_dllRefCount;

ClassFactory::ClassFactory(REFCLSID rclsid)
    : m_refCount(1)
    , m_clsid(rclsid) {
    InterlockedIncrement(&g_dllRefCount);
}

ClassFactory::~ClassFactory() {
    InterlockedDecrement(&g_dllRefCount);
}

STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_INVALIDARG;

    *ppv = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)) {
        *ppv = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ClassFactory::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) ClassFactory::Release() {
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

STDMETHODIMP ClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) {
    if (!ppv) return E_INVALIDARG;

    *ppv = nullptr;

    if (pUnkOuter) {
        return CLASS_E_NOAGGREGATION;
    }

    // Check if creating context menu
    if (IsEqualCLSID(m_clsid, CLSID_ContextMenu)) {
        ContextMenu* contextMenu = new (std::nothrow) ContextMenu();
        if (!contextMenu) {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = contextMenu->QueryInterface(riid, ppv);
        contextMenu->Release();
        return hr;
    }

    // Check if creating property sheet
    if (IsEqualCLSID(m_clsid, CLSID_PropertySheet)) {
        GitPropSheet* propSheet = new (std::nothrow) GitPropSheet();
        if (!propSheet) {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = propSheet->QueryInterface(riid, ppv);
        propSheet->Release();
        return hr;
    }

    // Otherwise, create overlay
    GitScribeOverlay* overlay = nullptr;

    // Create the appropriate overlay based on CLSID
    if (IsEqualCLSID(m_clsid, CLSID_ModifiedOverlay)) {
        overlay = new (std::nothrow) ModifiedOverlay();
    }
    else if (IsEqualCLSID(m_clsid, CLSID_CleanOverlay)) {
        overlay = new (std::nothrow) CleanOverlay();
    }
    else if (IsEqualCLSID(m_clsid, CLSID_AddedOverlay)) {
        overlay = new (std::nothrow) AddedOverlay();
    }
    else if (IsEqualCLSID(m_clsid, CLSID_UntrackedOverlay)) {
        overlay = new (std::nothrow) UntrackedOverlay();
    }
    else if (IsEqualCLSID(m_clsid, CLSID_ConflictedOverlay)) {
        overlay = new (std::nothrow) ConflictedOverlay();
    }
    else if (IsEqualCLSID(m_clsid, CLSID_IgnoredOverlay)) {
        overlay = new (std::nothrow) IgnoredOverlay();
    }
    else {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    if (!overlay) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = overlay->QueryInterface(riid, ppv);
    overlay->Release();

    return hr;
}

STDMETHODIMP ClassFactory::LockServer(BOOL fLock) {
    if (fLock) {
        InterlockedIncrement(&g_dllRefCount);
    } else {
        InterlockedDecrement(&g_dllRefCount);
    }
    return S_OK;
}
