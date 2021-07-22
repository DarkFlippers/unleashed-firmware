#include "irda-app-brute-force.hpp"
#include "irda/irda-app-file-parser.hpp"
#include "m-string.h"
#include <file-worker-cpp.h>
#include <memory>

void IrdaAppBruteForce::add_record(int index, const char* name) {
    records[name].index = index;
    records[name].amount = 0;
}

bool IrdaAppBruteForce::calculate_messages() {
    bool fs_res = false;
    furi_assert(!file_parser);

    file_parser = std::make_unique<IrdaAppFileParser>();
    fs_res = file_parser->open_irda_file_read(universal_db_filename);
    if(!fs_res) {
        file_parser.reset(nullptr);
        return false;
    }

    while(1) {
        auto file_signal = file_parser->read_signal();
        if(!file_signal) break;

        auto element = records.find(file_signal->name);
        if(element != records.cend()) {
            ++element->second.amount;
        }
    }

    file_parser->close();
    file_parser.reset(nullptr);

    return true;
}

void IrdaAppBruteForce::stop_bruteforce() {
    furi_assert((current_record.size()));

    if(current_record.size()) {
        furi_assert(file_parser);
        current_record.clear();
        file_parser->close();
        file_parser.reset(nullptr);
    }
}

// TODO: [FL-1418] replace with timer-chained consequence of messages.
bool IrdaAppBruteForce::send_next_bruteforce(void) {
    furi_assert(current_record.size());
    furi_assert(file_parser);

    std::unique_ptr<IrdaAppFileParser::IrdaFileSignal> file_signal;

    do {
        file_signal = file_parser->read_signal();
    } while(file_signal && current_record.compare(file_signal->name));

    if(file_signal) {
        file_signal->signal.transmit();
    }
    return !!file_signal;
}

bool IrdaAppBruteForce::start_bruteforce(int index, int& record_amount) {
    bool result = false;
    record_amount = 0;

    for(const auto& it : records) {
        if(it.second.index == index) {
            record_amount = it.second.amount;
            if(record_amount) {
                current_record = it.first;
            }
            break;
        }
    }

    if(record_amount) {
        file_parser = std::make_unique<IrdaAppFileParser>();
        result = file_parser->open_irda_file_read(universal_db_filename);
        if(!result) {
            (void)file_parser.reset(nullptr);
        }
    }

    return result;
}
