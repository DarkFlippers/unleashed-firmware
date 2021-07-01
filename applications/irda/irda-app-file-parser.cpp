#include "irda-app-file-parser.hpp"

std::unique_ptr<IrdaAppFileParser::IrdaFileMessage> IrdaAppFileParser::read_message(File* file) {
    while(1) {
        auto str = getline(file);
        if(str.empty()) return nullptr;

        auto message = parse_message(str);
        if(message) return message;
    }
}

std::unique_ptr<IrdaAppFileParser::IrdaFileMessage>
    IrdaAppFileParser::parse_message(const std::string& str) const {
    char protocol_name[32];
    uint32_t address;
    uint32_t command;
    auto irda_file_message = std::make_unique<IrdaFileMessage>();

    int parsed = std::sscanf(
        str.c_str(),
        "%31s %31s A:%lX C:%lX",
        irda_file_message->name,
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

    irda_file_message->message = {
        .protocol = protocol,
        .address = address,
        .command = command,
        .repeat = false,
    };

    return irda_file_message;
}
