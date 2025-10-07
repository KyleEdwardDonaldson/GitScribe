#pragma once

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

// Icon resource IDs
#define IDI_MODIFIED    100
#define IDI_CLEAN       101
#define IDI_ADDED       102
#define IDI_UNTRACKED   103
#define IDI_CONFLICTED  104
#define IDI_IGNORED     105
#define IDI_MENU        106

// String resource IDs
#define IDS_MODIFIED    200
#define IDS_CLEAN       201
#define IDS_ADDED       202
#define IDS_UNTRACKED   203
#define IDS_CONFLICTED  204
#define IDS_IGNORED     205

// Property sheet dialog
#define IDD_GIT_PROPPAGE    300

// Property page control IDs
#define IDC_GIT_REPO_PATH   301
#define IDC_GIT_BRANCH      302
#define IDC_GIT_STATE       303
#define IDC_GIT_MODIFIED    304
#define IDC_GIT_SYNC        305
#define IDC_GIT_FILE_STATUS 306
#define IDC_GIT_STATUS      307

// CLSID for property sheet
// {8F4E0E50-7B2D-4A1E-9C3F-1D2E3F4A5B6C}
#define CLSID_PropertySheet L"{8F4E0E50-7B2D-4A1E-9C3F-1D2E3F4A5B6C}"
