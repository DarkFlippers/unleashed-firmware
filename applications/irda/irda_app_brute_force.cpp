
#include "helpers/irda_parser.h"
#include "irda_app_brute_force.h"
#include "irda_app_signal.h"
#include <memory>
#include <m-string.h>
#include <furi.h>
#include <file_worker_cpp.h>

void IrdaAppBruteForce::add_record(int index, const char* name) {
    records[name].index = index;
    records[name].amount = 0;
}

bool IrdaAppBruteForce::calculate_messages() {
    bool result = false;

    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));
    FlipperFormat* ff = flipper_format_file_alloc(storage);
    result = flipper_format_file_open_existing(ff, universal_db_filename);

    if(result) {
        IrdaAppSignal signal;

        string_t signal_name;
        string_init(signal_name);
        while(flipper_format_read_string(ff, "name", signal_name)) {
            auto element = records.find(string_get_cstr(signal_name));
            if(element != records.cend()) {
                ++element->second.amount;
            }
        }
        string_clear(signal_name);
    }

    flipper_format_free(ff);
    furi_record_close("storage");
    return result;
}

void IrdaAppBruteForce::stop_bruteforce() {
    furi_assert((current_record.size()));

    if(current_record.size()) {
        furi_assert(ff);
        current_record.clear();
        flipper_format_free(ff);
        furi_record_close("storage");
    }
}

bool IrdaAppBruteForce::send_next_bruteforce(void) {
    furi_assert(current_record.size());
    furi_assert(ff);

    IrdaAppSignal signal;
    std::string signal_name;
    bool result = false;
    do {
        result = irda_parser_read_signal(ff, signal, signal_name);
    } while(result && current_record.compare(signal_name));

    if(result) {
        signal.transmit();
    }
    return result;
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
        Storage* storage = static_cast<Storage*>(furi_record_open("storage"));
        ff = flipper_format_file_alloc(storage);
        result = flipper_format_file_open_existing(ff, universal_db_filename);
        if(!result) {
            flipper_format_free(ff);
            furi_record_close("storage");
        }
    }

    return result;
}
