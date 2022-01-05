#pragma once

#include "irda_app_file_parser.h"

#include <unordered_map>
#include <memory>

class IrdaAppBruteForce {
    const char* universal_db_filename;
    std::string current_record;
    std::unique_ptr<IrdaAppFileParser> file_parser;

    typedef struct {
        int index;
        int amount;
    } Record;

    // 'key' is record name, because we have to search by both, index and name,
    // but index search has place once per button press, and should not be
    // noticed, but name search should occur during entering universal menu,
    // and will go through container for every record in file, that's why
    // more critical to have faster search by record name.
    std::unordered_map<std::string, Record> records;

public:
    bool calculate_messages();
    void stop_bruteforce();
    bool send_next_bruteforce();
    bool start_bruteforce(int index, int& record_amount);
    void add_record(int index, const char* name);

    IrdaAppBruteForce(const char* filename)
        : universal_db_filename(filename) {
    }
    ~IrdaAppBruteForce() {
    }
};
