/**
 * GitScribe Core - C API
 *
 * High-performance Git operations library
 * Copyright (C) 2025 GitScribe
 */


#ifndef GITSCRIBE_CORE_H
#define GITSCRIBE_CORE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Opaque pointer to Repository (for C code)
 */
typedef struct GSRepository {
  uint8_t _private[0];
} GSRepository;

/**
 * Repository context information (C-compatible struct)
 */
typedef struct GSRepoInfo {
  int state;
  int is_clean;
  unsigned int modified_count;
  unsigned int conflicted_count;
  unsigned int ahead_count;
  unsigned int behind_count;
} GSRepoInfo;

/**
 * C-compatible file status entry
 */
typedef struct GSFileStatus {
  char *path;
  int status;
} GSFileStatus;

/**
 * C-compatible list of file statuses
 */
typedef struct GSStatusList {
  struct GSFileStatus *entries;
  uintptr_t count;
} GSStatusList;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Open a Git repository
 *
 * # Safety
 * `path` must be a valid null-terminated C string
 * Returns NULL on error
 */
struct GSRepository *gs_repository_open(const char *path);

/**
 * Get file status
 *
 * # Safety
 * `repo` must be a valid repository pointer from gs_repository_open
 * `path` must be a valid null-terminated C string
 * Returns -1 on error
 */
int gs_file_status(struct GSRepository *repo, const char *path);

/**
 * Free repository
 *
 * # Safety
 * `repo` must be a valid repository pointer from gs_repository_open
 * Can be called with NULL (no-op)
 */
void gs_repository_free(struct GSRepository *repo);

/**
 * Get library version string
 *
 * Returns a null-terminated C string. Caller must NOT free this string.
 */
const char *gs_version(void);

/**
 * Get repository information (state, branch, counts)
 *
 * # Safety
 * `repo` must be a valid repository pointer from gs_repository_open
 * `info` must be a valid pointer to GSRepoInfo struct
 * Returns 0 on success, -1 on error
 */
int gs_repository_info(struct GSRepository *repo, struct GSRepoInfo *info);

/**
 * Get current branch name
 *
 * # Safety
 * `repo` must be a valid repository pointer from gs_repository_open
 * Returns null-terminated string. Caller MUST free with gs_string_free()
 * Returns NULL on error
 */
char *gs_repository_current_branch(struct GSRepository *repo);

/**
 * Free string allocated by Rust
 *
 * # Safety
 * `s` must be a string returned by a gs_* function that allocates strings
 * Can be called with NULL (no-op)
 */
void gs_string_free(char *s);

/**
 * Get status of all files in repository (bulk query)
 *
 * This is much faster than calling gs_file_status() for each file individually.
 * Returns a list of all files with non-clean status.
 *
 * # Safety
 * `repo` must be a valid repository pointer from gs_repository_open
 * Returns NULL on error
 * Caller MUST free with gs_status_list_free()
 */
struct GSStatusList *gs_repository_all_statuses(struct GSRepository *repo);

/**
 * Free status list allocated by gs_repository_all_statuses
 *
 * # Safety
 * `list` must be a valid pointer from gs_repository_all_statuses
 * Can be called with NULL (no-op)
 */
void gs_status_list_free(struct GSStatusList *list);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  /* GITSCRIBE_CORE_H */
