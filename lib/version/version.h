#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Version Version;

/**
 * Gets current running firmware version handle.
 * You can store it somewhere. But if you want to retrieve data,
 * you have to use 'version_*_get()' set of functions.
 * Also, 'version_*_get()' imply to use this
 * handle if no handle (NULL_PTR) provided.
 *
 * @return Handle to version data.
 */
const Version* version_get(void);

/**
 * Gets git hash of build commit.
 *
 * @param   v - ptr to version handle. If zero - gets current running fw info.
 * @return  git hash
 */
const char* version_get_githash(const Version* v);

/**
 * Gets git branch of build commit.
 *
 * @param   v - ptr to version handle. If zero - gets current running fw info.
 * @return  git branch
 */
const char* version_get_gitbranch(const Version* v);

/**
 * Gets git number of build commit.
 *
 * @param   v - ptr to version handle. If zero - gets current running fw info.
 * @return  number of commit
 */
const char* version_get_gitbranchnum(const Version* v);

/**
 * Gets build date.
 *
 * @param   v - ptr to version handle. If zero - gets current running fw info.
 * @return  build date
 */
const char* version_get_builddate(const Version* v);

/**
 * Gets build version.
 * Build version is last tag in git history.
 *
 * @param   v - ptr to version handle. If zero - gets current running fw info.
 * @return  build date
 */
const char* version_get_version(const Version* v);

/**
 * Gets firmware target.
 * Build version is last tag for build commit.
 *
 * @param   v - ptr to version handle. If zero - gets current running fw info.
 * @return  build date
 */
const char* version_get_target(const Version* v);

#ifdef __cplusplus
}
#endif

