#include "irda-app-file-parser.hpp"
#include "furi/check.h"
#include "irda-app-remote-manager.hpp"
#include "irda-app-signal.h"
#include "m-string.h"
#include <text-store.h>
#include <irda.h>
#include <cstdio>
#include <stdint.h>
#include <string>
#include <string_view>
#include <furi.h>
#include <file-worker-cpp.h>

uint32_t const IrdaAppFileParser::max_line_length = ((9 + 1) * 512 + 100);
const char* IrdaAppFileParser::irda_directory = "/any/irda";
const char* IrdaAppFileParser::irda_extension = ".ir";
uint32_t const IrdaAppFileParser::max_raw_timings_in_signal = 512;

bool IrdaAppFileParser::open_irda_file_read(const char* name) {
    std::string full_filename;
    if(name[0] != '/')
        full_filename = make_full_name(name);
    else
        full_filename = name;

    return file_worker.open(full_filename.c_str(), FSAM_READ, FSOM_OPEN_EXISTING);
}

bool IrdaAppFileParser::open_irda_file_write(const char* name) {
    std::string dirname(irda_directory);
    auto full_filename = make_full_name(name);

    if(!file_worker.mkdir(dirname.c_str())) return false;

    return file_worker.open(full_filename.c_str(), FSAM_WRITE, FSOM_CREATE_ALWAYS);
}

size_t IrdaAppFileParser::stringify_message(
    const IrdaAppSignal& signal,
    const char* name,
    char* buf,
    size_t buf_size) {
    auto message = signal.get_message();
    auto protocol = message.protocol;
    size_t written = 0;

    written += sniprintf(
        buf,
        buf_size,
        "%.31s %.31s A:%0*lX C:%0*lX\n",
        name,
        irda_get_protocol_name(protocol),
        irda_get_protocol_address_length(protocol),
        message.address,
        irda_get_protocol_command_length(protocol),
        message.command);

    furi_assert(written < buf_size);
    if(written >= buf_size) {
        written = 0;
    }

    return written;
}

size_t IrdaAppFileParser::stringify_raw_signal(
    const IrdaAppSignal& signal,
    const char* name,
    char* buf,
    size_t buf_size) {
    size_t written = 0;
    int duty_cycle = 100 * IRDA_COMMON_DUTY_CYCLE;
    written += sniprintf(
        &buf[written],
        max_line_length - written,
        "%.31s RAW F:%d DC:%d",
        name,
        IRDA_COMMON_CARRIER_FREQUENCY,
        duty_cycle);

    auto& raw_signal = signal.get_raw_signal();
    for(size_t i = 0; i < raw_signal.timings_cnt; ++i) {
        written += sniprintf(&buf[written], buf_size - written, " %ld", raw_signal.timings[i]);
        if(written > buf_size) {
            return false;
        }
    }
    written += snprintf(&buf[written], buf_size - written, "\n");

    furi_assert(written < buf_size);
    if(written >= buf_size) {
        written = 0;
    }

    return written;
}

bool IrdaAppFileParser::save_signal(const IrdaAppSignal& signal, const char* name) {
    char* buf = new char[max_line_length];
    size_t buf_cnt = 0;
    bool write_result = false;

    if(signal.is_raw()) {
        buf_cnt = stringify_raw_signal(signal, name, buf, max_line_length);
    } else {
        buf_cnt = stringify_message(signal, name, buf, max_line_length);
    }

    if(buf_cnt) {
        write_result = file_worker.write(buf, buf_cnt);
    }
    delete[] buf;
    return write_result;
}

std::unique_ptr<IrdaAppFileParser::IrdaFileSignal> IrdaAppFileParser::read_signal(void) {
    string_t line;
    string_init(line);
    string_reserve(line, max_line_length);
    std::unique_ptr<IrdaAppFileParser::IrdaFileSignal> file_signal;

    while(!file_signal &&
          file_worker.read_until_buffered(line, file_buf, &file_buf_cnt, sizeof(file_buf))) {
        if(string_empty_p(line)) {
            continue;
        }
        auto c_str = string_get_cstr(line);
        file_signal = parse_signal(c_str);
        if(!file_signal) {
            file_signal = parse_signal_raw(c_str);
        }
    }
    string_clear(line);

    return file_signal;
}

std::unique_ptr<IrdaAppFileParser::IrdaFileSignal>
    IrdaAppFileParser::parse_signal(const std::string& str) const {
    char protocol_name[32];
    uint32_t address;
    uint32_t command;
    auto irda_file_signal = std::make_unique<IrdaFileSignal>();

    int parsed = std::sscanf(
        str.c_str(),
        "%31s %31s A:%lX C:%lX",
        irda_file_signal->name,
        protocol_name,
        &address,
        &command);

    if(parsed != 4) {
        return nullptr;
    }

    IrdaProtocol protocol = irda_get_protocol_by_name(protocol_name);

    if(!irda_is_protocol_valid((IrdaProtocol)protocol)) {
        return nullptr;
    }

    int address_length = irda_get_protocol_address_length(protocol);
    uint32_t address_mask = (1LU << (4 * address_length)) - 1;
    if(address != (address & address_mask)) {
        return nullptr;
    }

    int command_length = irda_get_protocol_command_length(protocol);
    uint32_t command_mask = (1LU << (4 * command_length)) - 1;
    if(command != (command & command_mask)) {
        return nullptr;
    }

    IrdaMessage message = {
        .protocol = protocol,
        .address = address,
        .command = command,
        .repeat = false,
    };

    irda_file_signal->signal.set_message(&message);

    return irda_file_signal;
}

const char* find_first_not_of(const char* str, char symbol) {
    const char* str_start = nullptr;
    while(str != str_start) {
        str_start = str;
        str = strchr(str, symbol);
    }

    return str;
}

std::unique_ptr<IrdaAppFileParser::IrdaFileSignal>
    IrdaAppFileParser::parse_signal_raw(const std::string& string) const {
    uint32_t frequency;
    uint32_t duty_cycle;
    int str_len = string.size();
    std::string_view str(string.c_str());
    auto irda_file_signal = std::make_unique<IrdaFileSignal>();

    int parsed = std::sscanf(
        str.data(), "%31s RAW F:%ld DC:%ld", irda_file_signal->name, &frequency, &duty_cycle);

    if((parsed != 3) || (frequency > 42000) || (frequency < 32000) || (duty_cycle == 0) ||
       (duty_cycle >= 100)) {
        return nullptr;
    }

    char dummy[100] = {0};
    int header_len = 0;
    header_len = sniprintf(
        dummy,
        sizeof(dummy),
        "%.31s RAW F:%ld DC:%ld",
        irda_file_signal->name,
        frequency,
        duty_cycle);

    furi_assert(header_len < str_len);
    str.remove_prefix(header_len);

    /* move allocated timings into raw signal object */
    IrdaAppSignal::RawSignal raw_signal = {
        .timings_cnt = 0, .timings = new uint32_t[max_raw_timings_in_signal]};
    bool result = false;

    while(!str.empty()) {
        char buf[10];
        size_t index = str.find_first_not_of(' ', 1);
        if(index == std::string_view::npos) {
            break;
        }
        str.remove_prefix(index);
        parsed = std::sscanf(str.data(), "%9s", buf);
        if(parsed != 1) {
            result = false;
            furi_assert(0);
            break;
        }
        str.remove_prefix(strlen(buf));

        int value = atoi(buf);
        if(value <= 0) {
            result = false;
            furi_assert(0);
            break;
        }
        raw_signal.timings[raw_signal.timings_cnt] = value;
        ++raw_signal.timings_cnt;
        result = true;
        if(raw_signal.timings_cnt >= max_raw_timings_in_signal) {
            result = false;
            furi_assert(0);
            break;
        }
    }

    if(result) {
        /* copy timings instead of moving them to occupy less than max_raw_timings_in_signal */
        irda_file_signal->signal.copy_raw_signal(raw_signal.timings, raw_signal.timings_cnt);
    } else {
        (void)irda_file_signal.release();
    }
    delete[] raw_signal.timings;
    return irda_file_signal;
}

bool IrdaAppFileParser::is_irda_file_exist(const char* name, bool* exist) {
    std::string full_path = make_full_name(name);
    return file_worker.is_file_exist(full_path.c_str(), exist);
}

std::string IrdaAppFileParser::make_full_name(const std::string& remote_name) const {
    return std::string("") + irda_directory + "/" + remote_name + irda_extension;
}

std::string IrdaAppFileParser::make_name(const std::string& full_name) const {
    std::string str(full_name, full_name.find_last_of('/') + 1, full_name.size());
    str.erase(str.find_last_of('.'));

    return str;
}

bool IrdaAppFileParser::remove_irda_file(const char* name) {
    std::string full_filename = make_full_name(name);
    return file_worker.remove(full_filename.c_str());
}

bool IrdaAppFileParser::rename_irda_file(const char* old_name, const char* new_name) {
    std::string old_filename = make_full_name(old_name);
    std::string new_filename = make_full_name(new_name);
    return file_worker.rename(old_filename.c_str(), new_filename.c_str());
}

bool IrdaAppFileParser::close() {
    return file_worker.close();
}

bool IrdaAppFileParser::check_errors() {
    return file_worker.check_errors();
}

std::string IrdaAppFileParser::file_select(const char* selected) {
    TextStore* filename_ts = new TextStore(128);
    bool result;

    result = file_worker.file_select(
        irda_directory, irda_extension, filename_ts->text, filename_ts->text_size, selected);

    delete filename_ts;

    return result ? std::string(filename_ts->text) : std::string();
}
