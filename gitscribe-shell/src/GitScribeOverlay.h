#pragma once

#include <windows.h>
#include <shlobj.h>
#include <string>

// Git status enum (must match Rust core)
enum class GitStatus {
    Clean = 0,
    Modified = 1,
    Added = 2,
    Deleted = 3,
    Ignored = 4,
    Conflicted = 5,
    Untracked = 6,
    Locked = 7
};

// Base class for Git overlay icons
class GitScribeOverlay : public IShellIconOverlayIdentifier {
public:
    GitScribeOverlay(GitStatus status, int iconResourceId);
    virtual ~GitScribeOverlay();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IShellIconOverlayIdentifier
    STDMETHOD(GetOverlayInfo)(PWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags);
    STDMETHOD(GetPriority)(int* pPriority);
    STDMETHOD(IsMemberOf)(PCWSTR pwszPath, DWORD dwAttrib);

    // Public static method for context menu integration
    static void NotifyContextMenu();

protected:
    GitStatus m_status;
    int m_iconResourceId;
    LONG m_refCount;

    // Helper to check if file matches our status
    bool IsFileStatus(const std::wstring& path, GitStatus status);
};

// Specific overlay classes for each status
class ModifiedOverlay : public GitScribeOverlay {
public:
    ModifiedOverlay();
};

class CleanOverlay : public GitScribeOverlay {
public:
    CleanOverlay();
};

class AddedOverlay : public GitScribeOverlay {
public:
    AddedOverlay();
};

class UntrackedOverlay : public GitScribeOverlay {
public:
    UntrackedOverlay();
};

class ConflictedOverlay : public GitScribeOverlay {
public:
    ConflictedOverlay();
};

class IgnoredOverlay : public GitScribeOverlay {
public:
    IgnoredOverlay();
};
