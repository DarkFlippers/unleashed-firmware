#pragma once
#include "file_reader/file_reader.hpp"
#include "irda.h"

class IrdaAppFileParser : public FileReader {
public:
    typedef struct {
        char name[32];
        IrdaMessage message;
    } IrdaFileMessage;

    std::unique_ptr<IrdaAppFileParser::IrdaFileMessage> read_message(File* file);

private:
    std::unique_ptr<IrdaFileMessage> parse_message(const std::string& str) const;
};

