#pragma once
#include <string>
#include <memory>
#include "sd-card-api.h"
#include "filesystem-api.h"

class FileReader {
private:
    char file_buf[48];
    size_t file_buf_cnt = 0;
    size_t max_line_length = 0;
    SdCard_Api* sd_ex_api;
    FS_Api* fs_api;

public:
    FileReader() {
        sd_ex_api = static_cast<SdCard_Api*>(furi_record_open("sdcard-ex"));
        fs_api = static_cast<FS_Api*>(furi_record_open("sdcard"));
        reset();
    }
    ~FileReader() {
        furi_record_close("sdcard");
        furi_record_close("sdcard-ex");
    }

    std::string getline(File* file);

    void reset(void) {
        file_buf_cnt = 0;
    }

    SdCard_Api& get_sd_api() {
        return *sd_ex_api;
    }

    FS_Api& get_fs_api() {
        return *fs_api;
    }

    void set_max_line_length(size_t value) {
        max_line_length = value;
    }
};

