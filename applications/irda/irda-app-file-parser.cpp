#include "irda-app-file-parser.hpp"
#include "irda-app-remote-manager.hpp"
#include "irda-app-signal.h"
#include <irda.h>
#include <cstdio>
#include <stdint.h>
#include <string_view>
#include <furi.h>

uint32_t const IrdaAppFileParser::max_line_length = ((9 + 1) * 512 + 100);

std::unique_ptr<IrdaAppFileParser::IrdaFileSignal> IrdaAppFileParser::read_signal(File* file) {
    while(1) {
        auto str = getline(file);
        if(str.empty()) return nullptr;

        auto message = parse_signal(str);
        if(!message.get()) {
            message = parse_signal_raw(str);
        }
        if(message) return message;
    }
}

bool IrdaAppFileParser::store_signal(File* file, const IrdaAppSignal& signal, const char* name) {
    char* content = new char[max_line_length];
    size_t written = 0;

    if(!signal.is_raw()) {
        auto message = signal.get_message();
        auto protocol = message.protocol;

        sniprintf(
            content,
            max_line_length,
            "%.31s %.31s A:%0*lX C:%0*lX\n",
            name,
            irda_get_protocol_name(protocol),
            irda_get_protocol_address_length(protocol),
            message.address,
            irda_get_protocol_command_length(protocol),
            message.command);
        written = strlen(content);
    } else {
        int duty_cycle = 100 * IRDA_COMMON_DUTY_CYCLE;
        written += sniprintf(
            &content[written],
            max_line_length - written,
            "%.31s RAW F:%d DC:%d",
            name,
            IRDA_COMMON_CARRIER_FREQUENCY,
            duty_cycle);

        auto& raw_signal = signal.get_raw_signal();
        for(size_t i = 0; i < raw_signal.timings_cnt; ++i) {
            written += sniprintf(
                &content[written], max_line_length - written, " %ld", raw_signal.timings[i]);
            furi_assert(written <= max_line_length);
        }
        written += snprintf(&content[written], max_line_length - written, "\n");
    }
    furi_assert(written < max_line_length);

    size_t write_count = 0;
    write_count = get_fs_api().file.write(file, content, written);
    delete[] content;
    return (file->error_id == FSE_OK) && (write_count == written);
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
    char protocol_name[32];
    uint32_t frequency;
    uint32_t duty_cycle;
    int str_len = string.size();
    std::string_view str(string.c_str());
    auto irda_file_signal = std::make_unique<IrdaFileSignal>();

    int parsed = std::sscanf(
        str.data(),
        "%31s %31s F:%ld DC:%ld",
        irda_file_signal->name,
        protocol_name,
        &frequency,
        &duty_cycle);

    if(parsed != 4) {
        return nullptr;
    }

    char dummy[100] = {0};
    int header_len = 0;
    header_len = sniprintf(
        dummy,
        sizeof(dummy),
        "%.31s %.31s F:%ld DC:%ld",
        irda_file_signal->name,
        protocol_name,
        frequency,
        duty_cycle);

    furi_assert(header_len < str_len);
    str.remove_prefix(header_len);

    /* move allocated timings into raw signal object */
    IrdaAppSignal::RawSignal raw_signal = {.timings_cnt = 0, .timings = new uint32_t[500]};
    bool result = false;

    while(!str.empty()) {
        char buf[10];
        size_t index = str.find_first_not_of(' ', 1);
        if(index == std::string_view::npos) {
            result = true;
            break;
        }
        str.remove_prefix(index);
        parsed = std::sscanf(str.data(), "%9s", buf);
        if(parsed != 1) {
            break;
        }
        str.remove_prefix(strlen(buf));

        int value = atoi(buf);
        if(value <= 0) {
            break;
        }
        raw_signal.timings[raw_signal.timings_cnt] = value;
        ++raw_signal.timings_cnt;
        if(raw_signal.timings_cnt >= 500) {
            break;
        }
    }

    if(result) {
        irda_file_signal->signal.set_raw_signal(raw_signal.timings, raw_signal.timings_cnt);
    } else {
        (void)irda_file_signal.release();
        delete[] raw_signal.timings;
    }
    return irda_file_signal;
}
