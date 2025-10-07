#include "MenuContext.h"
#include <filesystem>

MenuContext::MenuContext(const std::vector<std::wstring>& selectedPaths)
    : m_type(ContextType::None)
    , m_selectedPaths(selectedPaths)
{
    // Try to find repository
    if (!selectedPaths.empty()) {
        m_repo = FindRepository(selectedPaths[0]);

        if (m_repo && m_repo->IsValid()) {
            m_repoInfo = m_repo->GetInfo();
            DetectContext();
        }
    }
}

void MenuContext::DetectContext() {
    // Check for merge/rebase in progress
    if (m_repoInfo.state != RepoState::Clean) {
        m_type = ContextType::MergeInProgress;
        return;
    }

    // Check if multiple files selected
    if (m_selectedPaths.size() > 1) {
        m_type = ContextType::MultiSelection;
        return;
    }

    // Single file/folder selected
    if (m_selectedPaths.size() == 1) {
        GitStatus status = GetPrimaryFileStatus();

        // Check file status
        switch (status) {
            case GitStatus::Modified:
                m_type = ContextType::FileModified;
                return;
            case GitStatus::Untracked:
                m_type = ContextType::FileUntracked;
                return;
            case GitStatus::Conflicted:
                m_type = ContextType::FileConflicted;
                return;
            case GitStatus::Clean:
            case GitStatus::Locked:
            default:
                // Check if it's a directory (repository root)
                if (std::filesystem::is_directory(m_selectedPaths[0])) {
                    // Repository context
                    if (!m_repoInfo.isClean) {
                        m_type = ContextType::RepoDirty;
                    } else if (m_repoInfo.aheadCount > 0 && m_repoInfo.behindCount == 0) {
                        m_type = ContextType::RepoAhead;
                    } else if (m_repoInfo.behindCount > 0) {
                        m_type = ContextType::RepoBehind;
                    } else {
                        m_type = ContextType::RepoClean;
                    }
                } else {
                    m_type = ContextType::FileClean;
                }
                return;
        }
    }

    m_type = ContextType::None;
}

const std::wstring& MenuContext::GetPrimaryFile() const {
    static const std::wstring empty;
    if (m_selectedPaths.empty()) {
        return empty;
    }
    return m_selectedPaths[0];
}

std::wstring MenuContext::GetPrimaryFileName() const {
    if (m_selectedPaths.empty()) {
        return L"";
    }

    std::filesystem::path p(m_selectedPaths[0]);
    return p.filename().wstring();
}

GitStatus MenuContext::GetPrimaryFileStatus() const {
    if (!m_repo || m_selectedPaths.empty()) {
        return GitStatus::Clean;
    }

    return m_repo->GetFileStatus(m_selectedPaths[0]);
}
