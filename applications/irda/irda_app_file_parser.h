#pragma once

#include "irda_app_signal.h"

#include <irda.h>
#include <file_worker_cpp.h>
#include <memory>
#include <string>
#include <cstdint>

class IrdaAppFileParser {
public:
    typedef struct {
        char name[32];
        IrdaAppSignal signal;
    } IrdaFileSignal;

    bool open_irda_file_read(const char* filename);
    bool open_irda_file_write(const char* filename);
    bool is_irda_file_exist(const char* filename, bool* exist);
    bool rename_irda_file(const char* filename, const char* newname);
    bool remove_irda_file(const char* name);
    bool close();
    bool check_errors();

    std::unique_ptr<IrdaAppFileParser::IrdaFileSignal> read_signal();
    bool save_signal(const IrdaAppSignal& signal, const char* name);
    std::string file_select(const char* selected);

    std::string make_name(const std::string& full_name) const;

private:
    size_t stringify_message(
        const IrdaAppSignal& signal,
        const char* name,
        char* content,
        size_t content_len);
    size_t stringify_raw_signal(
        const IrdaAppSignal& signal,
        const char* name,
        char* content,
        size_t content_len);
    std::unique_ptr<IrdaFileSignal> parse_signal(const std::string& str) const;
    std::unique_ptr<IrdaFileSignal> parse_signal_raw(const std::string& str) const;
    std::string make_full_name(const std::string& name) const;

    static inline const char* const irda_directory = "/any/irda";
    static inline const char* const irda_extension = ".ir";
    static inline const uint32_t max_raw_timings_in_signal = 512;
    static inline const uint32_t max_line_length =
        (9 + 1) * IrdaAppFileParser::max_raw_timings_in_signal + 100;

    FileWorkerCpp file_worker;
    char file_buf[128];
    size_t file_buf_cnt = 0;
};
