#pragma once

#include <windows.h>
#include <unknwn.h>

// Class factory for creating overlay instances
class ClassFactory : public IClassFactory {
public:
    ClassFactory(REFCLSID rclsid);
    virtual ~ClassFactory();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IClassFactory
    STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppv);
    STDMETHOD(LockServer)(BOOL fLock);

private:
    LONG m_refCount;
    CLSID m_clsid;
};
