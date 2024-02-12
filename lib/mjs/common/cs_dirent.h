/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CS_COMMON_CS_DIRENT_H_
#define CS_COMMON_CS_DIRENT_H_

#include <limits.h>

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CS_DEFINE_DIRENT
typedef struct { int dummy; } DIR;

struct dirent {
  int d_ino;
#ifdef _WIN32
  char d_name[MAX_PATH];
#else
  /* TODO(rojer): Use PATH_MAX but make sure it's sane on every platform */
  char d_name[256];
#endif
};

DIR *opendir(const char *dir_name);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
#endif /* CS_DEFINE_DIRENT */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_COMMON_CS_DIRENT_H_ */
