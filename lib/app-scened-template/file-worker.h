#pragma once
#include "record-controller.hpp"
#include <sd-card-api.h>
#include <filesystem-api.h>
#include <m-string.h>

class FileWorker {
public:
    FileWorker();
    ~FileWorker();

    RecordController<FS_Api> fs_api;
    RecordController<SdCard_Api> sd_ex_api;

    bool open(const char* filename, FS_AccessMode access_mode, FS_OpenMode open_mode);
    bool close();

    bool mkdir(const char* dirname);
    bool remove(const char* filename);

private:
    File file;

    bool check_common_errors();
    void show_error_message(const char* error_text);
    string_t error_string;
};
