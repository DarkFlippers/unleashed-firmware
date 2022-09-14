#include "filesystem_api_defines.h"

const char* filesystem_api_error_get_desc(FS_Error error_id) {
    const char* result = "unknown error";
    switch(error_id) {
    case(FSE_OK):
        result = "OK";
        break;
    case(FSE_NOT_READY):
        result = "filesystem not ready";
        break;
    case(FSE_EXIST):
        result = "file/dir already exist";
        break;
    case(FSE_NOT_EXIST):
        result = "file/dir not exist";
        break;
    case(FSE_INVALID_PARAMETER):
        result = "invalid parameter";
        break;
    case(FSE_DENIED):
        result = "access denied";
        break;
    case(FSE_INVALID_NAME):
        result = "invalid name/path";
        break;
    case(FSE_INTERNAL):
        result = "internal error";
        break;
    case(FSE_NOT_IMPLEMENTED):
        result = "function not implemented";
        break;
    case(FSE_ALREADY_OPEN):
        result = "file is already open";
        break;
    }
    return result;
}
