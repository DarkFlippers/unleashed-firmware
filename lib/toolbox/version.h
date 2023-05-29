#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Version Version;

/** Get current running firmware version handle.
 *
 * You can store it somewhere. But if you want to retrieve data, you have to use
 * 'version_*_get()' set of functions. Also, 'version_*_get()' imply to use this
 * handle if no handle (NULL_PTR) provided.
 *
 * @return     pointer to Version data.
 */
const Version* version_get(void);

/** Get git commit hash.
 *
 * @param      v     pointer to Version data. NULL for currently running
 *                   software.
 *
 * @return     git hash
 */
const char* version_get_githash(const Version* v);

/** Get git branch.
 *
 * @param      v     pointer to Version data. NULL for currently running
 *                   software.
 *
 * @return     git branch
 */
const char* version_get_gitbranch(const Version* v);

/** Get number of commit in git branch.
 *
 * @param      v     pointer to Version data. NULL for currently running
 *                   software.
 *
 * @return     number of commit
 */
const char* version_get_gitbranchnum(const Version* v);

/** Get build date.
 *
 * @param      v     pointer to Version data. NULL for currently running
 *                   software.
 *
 * @return     build date
 */
const char* version_get_builddate(const Version* v);

/** Get build version. Build version is last tag in git history.
 *
 * @param      v     pointer to Version data. NULL for currently running
 *                   software.
 *
 * @return     build date
 */
const char* version_get_version(const Version* v);

/** Get hardware target this firmware was built for
 *
 * @param      v     pointer to Version data. NULL for currently running
 *                   software.
 *
 * @return     build date
 */
uint8_t version_get_target(const Version* v);

/** Get flag indicating if this build is "dirty" (source code had uncommited changes)
 *
 * @param      v     pointer to Version data. NULL for currently running
 *                   software.
 *
 * @return     build date
 */
bool version_get_dirty_flag(const Version* v);

/** 
 * Get firmware origin. "Official" for mainline firmware, fork name for forks.
 * Set by FIRMWARE_ORIGIN fbt argument.
*/
const char* version_get_firmware_origin(const Version* v);

/** 
 * Get git repo origin
*/
const char* version_get_git_origin(const Version* v);

#ifdef __cplusplus
}
#endif
