#include "file-worker.h"

FileWorker::FileWorker()
    : fs_api{"sdcard"}
    , sd_ex_api{"sdcard-ex"} {
    string_init(error_string);
}

FileWorker::~FileWorker() {
    string_clear(error_string);
}

bool FileWorker::open(const char* filename, FS_AccessMode access_mode, FS_OpenMode open_mode) {
    bool result = fs_api.get()->file.open(&file, filename, access_mode, open_mode);

    if(!result) {
        show_error_message("Cannot open\nfile");
        close();
        return false;
    }

    return check_common_errors();
}

bool FileWorker::close() {
    fs_api.get()->file.close(&file);

    return check_common_errors();
}

bool FileWorker::mkdir(const char* dirname) {
    FS_Error fs_result = fs_api.get()->common.mkdir(dirname);

    if(fs_result != FSE_OK && fs_result != FSE_EXIST) {
        show_error_message("Cannot create\nfolder");
        return false;
    };

    return check_common_errors();
}

bool FileWorker::remove(const char* filename) {
    FS_Error fs_result = fs_api.get()->common.remove(filename);
    if(fs_result != FSE_OK && fs_result != FSE_NOT_EXIST) {
        show_error_message("Cannot remove\nold file");
        return false;
    };

    return check_common_errors();
}

bool FileWorker::check_common_errors() {
    sd_ex_api.get()->check_error(sd_ex_api.get()->context);
    return true;
}

void FileWorker::show_error_message(const char* error_text) {
    string_set_str(error_string, error_text);
    sd_ex_api.get()->show_error(sd_ex_api.get()->context, string_get_cstr(error_string));
}
