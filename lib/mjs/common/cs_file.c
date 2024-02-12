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

#include "cs_file.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef CS_MMAP
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#ifdef CS_MMAP
char* cs_read_file(const char* path, size_t* size) WEAK;
char* cs_read_file(const char* path, size_t* size) {
    FILE* fp;
    char* data = NULL;
    if((fp = fopen(path, "rb")) == NULL) {
    } else if(fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
    } else {
        *size = ftell(fp);
        data = (char*)malloc(*size + 1);
        if(data != NULL) {
            fseek(fp, 0, SEEK_SET); /* Some platforms might not have rewind(), Oo */
            if(fread(data, 1, *size, fp) != *size) {
                free(data);
                return NULL;
            }
            data[*size] = '\0';
        }
        fclose(fp);
    }
    return data;
}

char* cs_mmap_file(const char* path, size_t* size) WEAK;
char* cs_mmap_file(const char* path, size_t* size) {
    char* r;
    int fd = open(path, O_RDONLY, 0);
    struct stat st;
    if(fd < 0) return NULL;
    fstat(fd, &st);
    *size = (size_t)st.st_size;
    r = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(r == MAP_FAILED) return NULL;
    return r;
}
#endif
