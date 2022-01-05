#include "irda_app_file_parser.h"
#include "irda_app_remote_manager.h"
#include "irda_app_signal.h"

#include <m-string.h>
#include <cstdio>
#include <text_store.h>
#include <irda.h>
#include <string_view>
#include <furi.h>
#include <furi_hal_irda.h>
#include <file_worker_cpp.h>

#define TAG "IrdaFileParser"

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
        ROUND_UP_TO(irda_get_protocol_address_length(protocol), 4),
        message.address,
        ROUND_UP_TO(irda_get_protocol_command_length(protocol), 4),
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
        size_t end_of_str = MIN(str.find_last_not_of(" \t\r\n") + 1, (size_t)30);
        FURI_LOG_E(
            TAG, "Unknown protocol(\'%.*s...\'): \'%s\'", end_of_str, str.c_str(), protocol_name);
        return nullptr;
    }

    uint32_t address_length = irda_get_protocol_address_length(protocol);
    uint32_t address_mask = (1LU << address_length) - 1;
    if(address != (address & address_mask)) {
        size_t end_of_str = MIN(str.find_last_not_of(" \t\r\n") + 1, (size_t)30);
        FURI_LOG_E(
            TAG,
            "Signal(\'%.*s...\'): address is too long (mask for this protocol is 0x%08X): 0x%X",
            end_of_str,
            str.c_str(),
            address_mask,
            address);
        return nullptr;
    }

    uint32_t command_length = irda_get_protocol_command_length(protocol);
    uint32_t command_mask = (1LU << command_length) - 1;
    if(command != (command & command_mask)) {
        size_t end_of_str = MIN(str.find_last_not_of(" \t\r\n") + 1, (size_t)30);
        FURI_LOG_E(
            TAG,
            "Signal(\'%.*s...\'): command is too long (mask for this protocol is 0x%08X): 0x%X",
            end_of_str,
            str.c_str(),
            command_mask,
            command);
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

static int remove_args32(std::string_view& str, size_t num) {
    int removed_length = 0;

    while(num--) {
        char buf[32];

        size_t index = str.find_first_not_of(" \t");
        if(index == std::string_view::npos) break;
        removed_length += index;
        str.remove_prefix(index);

        if(str.empty()) break;

        int parsed = std::sscanf(str.data(), "%31s", buf);
        if(!parsed) break;

        size_t len = strlen(buf);
        if(!len) break;
        removed_length += len;
        str.remove_prefix(len);

        if(str.empty()) break;
    }

    return removed_length;
}

std::unique_ptr<IrdaAppFileParser::IrdaFileSignal>
    IrdaAppFileParser::parse_signal_raw(const std::string& string) const {
    uint32_t frequency;
    uint32_t duty_cycle;
    std::string_view str(string.c_str());
    auto irda_file_signal = std::make_unique<IrdaFileSignal>();

    int parsed = std::sscanf(
        str.data(), "%31s RAW F:%ld DC:%ld", irda_file_signal->name, &frequency, &duty_cycle);

    if(parsed != 3) {
        return nullptr;
    }

    if((frequency < IRDA_MIN_FREQUENCY) || (frequency > IRDA_MAX_FREQUENCY)) {
        size_t end_of_str = MIN(string.find_last_not_of(" \t\r\n") + 1, (size_t)30);
        FURI_LOG_E(
            TAG,
            "RAW signal(\'%.*s...\'): frequency is out of bounds (%ld-%ld): %ld",
            end_of_str,
            string.c_str(),
            IRDA_MIN_FREQUENCY,
            IRDA_MAX_FREQUENCY,
            frequency);
        return nullptr;
    }

    if((duty_cycle == 0) || (duty_cycle > 100)) {
        size_t end_of_str = MIN(string.find_last_not_of(" \t\r\n") + 1, (size_t)30);
        FURI_LOG_E(
            TAG,
            "RAW signal(\'%.*s...\'): duty cycle is out of bounds (0-100): %ld",
            end_of_str,
            string.c_str(),
            duty_cycle);
        return nullptr;
    }

    int header_len = remove_args32(str, 4);

    size_t last_valid_ch = str.find_last_not_of(" \t\r\n");
    if(last_valid_ch != std::string_view::npos) {
        str.remove_suffix(str.size() - last_valid_ch - 1);
    } else {
        FURI_LOG_E(TAG, "RAW signal(\'%.*s\'): no timings", header_len, string.c_str());
        return nullptr;
    }

    /* move allocated timings into raw signal object */
    IrdaAppSignal::RawSignal raw_signal = {
        .timings_cnt = 0, .timings = new uint32_t[max_raw_timings_in_signal]};
    bool result = false;

    while(!str.empty()) {
        char buf[10];
        size_t index = str.find_first_not_of(" \t", 1);
        if(index == std::string_view::npos) {
            break;
        }
        str.remove_prefix(index);
        parsed = std::sscanf(str.data(), "%9s", buf);
        if(parsed != 1) {
            FURI_LOG_E(
                TAG,
                "RAW signal(\'%.*s...\'): failed on timing[%ld] \'%*s\'",
                header_len,
                string.c_str(),
                raw_signal.timings_cnt,
                str.size(),
                str.data());
            result = false;
            break;
        }
        str.remove_prefix(strlen(buf));

        int value = atoi(buf);
        if(value <= 0) {
            FURI_LOG_E(
                TAG,
                "RAW signal(\'%.*s...\'): failed on timing[%ld] \'%s\'",
                header_len,
                string.c_str(),
                raw_signal.timings_cnt,
                buf);
            result = false;
            break;
        }

        if(raw_signal.timings_cnt >= max_raw_timings_in_signal) {
            FURI_LOG_E(
                TAG,
                "RAW signal(\'%.*s...\'): too much timings (max %ld)",
                header_len,
                string.c_str(),
                max_raw_timings_in_signal);
            result = false;
            break;
        }
        raw_signal.timings[raw_signal.timings_cnt] = value;
        ++raw_signal.timings_cnt;
        result = true;
    }

    if(result) {
        /* copy timings instead of moving them to occupy less than max_raw_timings_in_signal */
        irda_file_signal->signal.copy_raw_signal(raw_signal.timings, raw_signal.timings_cnt);
    } else {
        irda_file_signal.reset();
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
    auto filename_ts = std::make_unique<TextStore>(IrdaAppRemoteManager::max_remote_name_length);
    bool result;

    result = file_worker.file_select(
        irda_directory, irda_extension, filename_ts->text, filename_ts->text_size, selected);

    return result ? std::string(filename_ts->text) : std::string();
}
