//! C Foreign Function Interface (FFI)
//!
//! Provides C-compatible API for use by the C++ shell extension

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_uint};
use std::ptr;

use crate::Repository;

/// Opaque pointer to Repository (for C code)
#[repr(C)]
pub struct GSRepository {
    _private: [u8; 0],
}

/// Repository context information (C-compatible struct)
#[repr(C)]
pub struct GSRepoInfo {
    pub state: c_int,           // RepoState as int
    pub is_clean: c_int,        // bool as int (0/1)
    pub modified_count: c_uint,
    pub conflicted_count: c_uint,
    pub ahead_count: c_uint,
    pub behind_count: c_uint,
}

impl Default for GSRepoInfo {
    fn default() -> Self {
        GSRepoInfo {
            state: 0,
            is_clean: 1,
            modified_count: 0,
            conflicted_count: 0,
            ahead_count: 0,
            behind_count: 0,
        }
    }
}

/// Open a Git repository
///
/// # Safety
/// `path` must be a valid null-terminated C string
/// Returns NULL on error
#[no_mangle]
pub unsafe extern "C" fn gs_repository_open(path: *const c_char) -> *mut GSRepository {
    if path.is_null() {
        return ptr::null_mut();
    }

    let c_str = match CStr::from_ptr(path).to_str() {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    match Repository::open(c_str) {
        Ok(repo) => Box::into_raw(Box::new(repo)) as *mut GSRepository,
        Err(_) => ptr::null_mut(),
    }
}

/// Get file status
///
/// # Safety
/// `repo` must be a valid repository pointer from gs_repository_open
/// `path` must be a valid null-terminated C string
/// Returns -1 on error
#[no_mangle]
pub unsafe extern "C" fn gs_file_status(
    repo: *mut GSRepository,
    path: *const c_char
) -> c_int {
    if repo.is_null() || path.is_null() {
        return -1;
    }

    let repo = &*(repo as *mut Repository);

    let c_str = match CStr::from_ptr(path).to_str() {
        Ok(s) => s,
        Err(_) => return -1,
    };

    match repo.file_status(c_str) {
        Ok(status) => status as c_int,
        Err(_) => -1,
    }
}

/// Free repository
///
/// # Safety
/// `repo` must be a valid repository pointer from gs_repository_open
/// Can be called with NULL (no-op)
#[no_mangle]
pub unsafe extern "C" fn gs_repository_free(repo: *mut GSRepository) {
    if !repo.is_null() {
        drop(Box::from_raw(repo as *mut Repository));
    }
}

/// Get library version string
///
/// Returns a null-terminated C string. Caller must NOT free this string.
#[no_mangle]
pub extern "C" fn gs_version() -> *const c_char {
    static VERSION: &str = concat!(env!("CARGO_PKG_VERSION"), "\0");
    VERSION.as_ptr() as *const c_char
}

/// Get repository information (state, branch, counts)
///
/// # Safety
/// `repo` must be a valid repository pointer from gs_repository_open
/// `info` must be a valid pointer to GSRepoInfo struct
/// Returns 0 on success, -1 on error
#[no_mangle]
pub unsafe extern "C" fn gs_repository_info(
    repo: *mut GSRepository,
    info: *mut GSRepoInfo
) -> c_int {
    if repo.is_null() || info.is_null() {
        return -1;
    }

    let repo = &*(repo as *mut Repository);
    let info = &mut *info;

    // Get state
    info.state = repo.state() as c_int;

    // Get counts
    info.modified_count = repo.count_modified().unwrap_or(0) as c_uint;
    info.conflicted_count = repo.count_conflicted().unwrap_or(0) as c_uint;
    info.is_clean = if repo.is_clean().unwrap_or(false) { 1 } else { 0 };

    // Get remote status
    if let Ok(Some(remote)) = repo.remote_status() {
        info.ahead_count = remote.ahead as c_uint;
        info.behind_count = remote.behind as c_uint;
    } else {
        info.ahead_count = 0;
        info.behind_count = 0;
    }

    0
}

/// Get current branch name
///
/// # Safety
/// `repo` must be a valid repository pointer from gs_repository_open
/// Returns null-terminated string. Caller MUST free with gs_string_free()
/// Returns NULL on error
#[no_mangle]
pub unsafe extern "C" fn gs_repository_current_branch(
    repo: *mut GSRepository
) -> *mut c_char {
    if repo.is_null() {
        return ptr::null_mut();
    }

    let repo = &*(repo as *mut Repository);

    match repo.current_branch() {
        Ok(branch) => {
            match CString::new(branch) {
                Ok(c_str) => c_str.into_raw(),
                Err(_) => ptr::null_mut(),
            }
        }
        Err(_) => ptr::null_mut(),
    }
}

/// Free string allocated by Rust
///
/// # Safety
/// `s` must be a string returned by a gs_* function that allocates strings
/// Can be called with NULL (no-op)
#[no_mangle]
pub unsafe extern "C" fn gs_string_free(s: *mut c_char) {
    if !s.is_null() {
        drop(CString::from_raw(s));
    }
}

/// C-compatible file status entry
#[repr(C)]
pub struct GSFileStatus {
    pub path: *mut c_char,  // Owned null-terminated string (caller must free with gs_string_free)
    pub status: c_int,      // FileStatus as int
}

/// C-compatible list of file statuses
#[repr(C)]
pub struct GSStatusList {
    pub entries: *mut GSFileStatus,
    pub count: usize,
}

/// Get status of all files in repository (bulk query)
///
/// This is much faster than calling gs_file_status() for each file individually.
/// Returns a list of all files with non-clean status.
///
/// # Safety
/// `repo` must be a valid repository pointer from gs_repository_open
/// Returns NULL on error
/// Caller MUST free with gs_status_list_free()
#[no_mangle]
pub unsafe extern "C" fn gs_repository_all_statuses(
    repo: *mut GSRepository
) -> *mut GSStatusList {
    if repo.is_null() {
        return ptr::null_mut();
    }

    let repo = &*(repo as *mut Repository);

    // Get all file statuses
    let statuses = match repo.status() {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    // Convert to C-compatible format
    let mut entries: Vec<GSFileStatus> = Vec::with_capacity(statuses.len());

    for entry in statuses {
        // Convert path to C string
        let path_str = match entry.path.to_str() {
            Some(s) => s,
            None => continue, // Skip invalid UTF-8 paths
        };

        let c_path = match CString::new(path_str) {
            Ok(s) => s.into_raw(),
            Err(_) => continue, // Skip paths with null bytes
        };

        entries.push(GSFileStatus {
            path: c_path,
            status: entry.status as c_int,
        });
    }

    // Allocate list structure
    let list = Box::new(GSStatusList {
        entries: entries.as_mut_ptr(),
        count: entries.len(),
    });

    // Prevent Vec from freeing the entries (they're now owned by the list)
    std::mem::forget(entries);

    Box::into_raw(list)
}

/// Free status list allocated by gs_repository_all_statuses
///
/// # Safety
/// `list` must be a valid pointer from gs_repository_all_statuses
/// Can be called with NULL (no-op)
#[no_mangle]
pub unsafe extern "C" fn gs_status_list_free(list: *mut GSStatusList) {
    if list.is_null() {
        return;
    }

    let list = Box::from_raw(list);

    // Free all path strings
    if !list.entries.is_null() {
        let entries = std::slice::from_raw_parts_mut(list.entries, list.count);
        for entry in entries {
            if !entry.path.is_null() {
                drop(CString::from_raw(entry.path));
            }
        }

        // Free entries array
        drop(Vec::from_raw_parts(list.entries, list.count, list.count));
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;
    use std::ffi::CString;

    #[test]
    fn test_ffi_open_close() {
        let temp_dir = TempDir::new().unwrap();
        git2::Repository::init(temp_dir.path()).unwrap();

        let path_str = temp_dir.path().to_str().unwrap();
        let c_path = CString::new(path_str).unwrap();

        unsafe {
            let repo = gs_repository_open(c_path.as_ptr());
            assert!(!repo.is_null());
            gs_repository_free(repo);
        }
    }

    #[test]
    fn test_ffi_null_path() {
        unsafe {
            let repo = gs_repository_open(ptr::null());
            assert!(repo.is_null());
        }
    }

    #[test]
    fn test_ffi_version() {
        let version = gs_version();
        assert!(!version.is_null());

        unsafe {
            let c_str = CStr::from_ptr(version);
            let rust_str = c_str.to_str().unwrap();
            assert_eq!(rust_str, "0.1.0");
        }
    }
}
