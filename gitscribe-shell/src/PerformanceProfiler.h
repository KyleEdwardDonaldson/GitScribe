#pragma once

#include <windows.h>
#include <string>
#include <sstream>

// Simple RAII profiler for measuring performance
class ScopedTimer {
public:
    ScopedTimer(const char* name) : m_name(name) {
        m_start = GetTickCount();

        std::ostringstream oss;
        oss << "[GitScribe][PERF] START: " << m_name << "\n";
        OutputDebugStringA(oss.str().c_str());
    }

    ~ScopedTimer() {
        DWORD elapsed = GetTickCount() - m_start;

        std::ostringstream oss;
        oss << "[GitScribe][PERF] END: " << m_name << " (" << elapsed << "ms)\n";
        OutputDebugStringA(oss.str().c_str());
    }

private:
    const char* m_name;
    DWORD m_start;
};

// Use this macro to create a scoped timer
#define PROFILE_SCOPE(name) ScopedTimer __timer##__LINE__(name)
