#pragma once
#include <file_reader/file_reader.h>
#include <irda.h>
#include "irda-app-remote-manager.hpp"

class IrdaAppFileParser : public FileReader {
public:
    typedef struct {
        char name[32];
        IrdaAppSignal signal;
    } IrdaFileSignal;

    IrdaAppFileParser() {
        /* Assume we can save max 512 samples */
        set_max_line_length(max_line_length);
    }

    std::unique_ptr<IrdaAppFileParser::IrdaFileSignal> read_signal(File* file);
    bool store_signal(File* file, const IrdaAppSignal& signal, const char* name);

private:
    static const uint32_t max_line_length;
    std::unique_ptr<IrdaFileSignal> parse_signal(const std::string& str) const;
    std::unique_ptr<IrdaFileSignal> parse_signal_raw(const std::string& str) const;
};

