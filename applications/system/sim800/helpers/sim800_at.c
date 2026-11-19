#include "sim800_at.h"

#define SIM800_AT_TIMEOUT 1000

bool sim800_at_query(Sim800Uart* uart, const char* cmd, const char* prefix, FuriString* out) {
    FuriString* response = furi_string_alloc();
    bool found = false;

    if(sim800_uart_command(uart, cmd, SIM800_AT_TIMEOUT, response) == Sim800AtResultOk) {
        found = sim800_response_field(response, prefix, out);
    }

    furi_string_free(response);
    return found;
}

bool sim800_at_line(Sim800Uart* uart, const char* cmd, FuriString* out) {
    FuriString* response = furi_string_alloc();
    bool found = false;

    if(sim800_uart_command(uart, cmd, SIM800_AT_TIMEOUT, response) == Sim800AtResultOk) {
        furi_string_trim(response);
        if(furi_string_size(response) > 0) {
            size_t newline = furi_string_search_char(response, '\n');
            if(newline != FURI_STRING_FAILURE) furi_string_left(response, newline);
            furi_string_trim(response);
            furi_string_set(out, response);
            found = true;
        }
    }

    furi_string_free(response);
    return found;
}

uint8_t sim800_at_rssi(Sim800Uart* uart) {
    FuriString* value = furi_string_alloc();
    FuriString* field = furi_string_alloc();
    uint8_t rssi = 99;

    if(sim800_at_query(uart, "AT+CSQ", "+CSQ", value)) {
        if(sim800_csv_field(furi_string_get_cstr(value), 0, field)) {
            rssi = (uint8_t)strtoul(furi_string_get_cstr(field), NULL, 10);
        }
    }

    furi_string_free(value);
    furi_string_free(field);
    return rssi;
}

void sim800_at_operator(Sim800Uart* uart, FuriString* out) {
    FuriString* value = furi_string_alloc();

    furi_string_set_str(out, "--");
    if(sim800_at_query(uart, "AT+COPS?", "+COPS", value)) {
        // +COPS: <mode>,<format>,"<name>" - the name is missing when not registered
        if(sim800_csv_field(furi_string_get_cstr(value), 2, out)) {
            if(furi_string_size(out) == 0) furi_string_set_str(out, "--");
        } else {
            furi_string_set_str(out, "--");
        }
    }

    furi_string_free(value);
}

void sim800_at_registration(Sim800Uart* uart, FuriString* out) {
    static const char* const text[] = {
        "Not registered",
        "Home network",
        "Searching",
        "Denied",
        "Unknown",
        "Roaming",
    };

    FuriString* value = furi_string_alloc();
    FuriString* field = furi_string_alloc();

    furi_string_set_str(out, "--");
    if(sim800_at_query(uart, "AT+CREG?", "+CREG", value)) {
        if(sim800_csv_field(furi_string_get_cstr(value), 1, field)) {
            unsigned long stat = strtoul(furi_string_get_cstr(field), NULL, 10);
            furi_string_set_str(out, (stat < COUNT_OF(text)) ? text[stat] : "Unknown");
        }
    }

    furi_string_free(value);
    furi_string_free(field);
}

void sim800_at_sim_status(Sim800Uart* uart, FuriString* out) {
    FuriString* value = furi_string_alloc();

    furi_string_set_str(out, "--");
    if(sim800_at_query(uart, "AT+CPIN?", "+CPIN", value)) {
        if(furi_string_cmp_str(value, "READY") == 0) {
            furi_string_set_str(out, "Ready");
        } else {
            furi_string_set(out, value);
        }
    } else {
        furi_string_set_str(out, "No SIM");
    }

    furi_string_free(value);
}

void sim800_at_power(Sim800Uart* uart, FuriString* out) {
    FuriString* value = furi_string_alloc();
    FuriString* percent = furi_string_alloc();
    FuriString* millivolt = furi_string_alloc();

    furi_string_set_str(out, "--");
    if(sim800_at_query(uart, "AT+CBC", "+CBC", value)) {
        const char* list = furi_string_get_cstr(value);
        if(sim800_csv_field(list, 1, percent) && sim800_csv_field(list, 2, millivolt)) {
            unsigned long mv = strtoul(furi_string_get_cstr(millivolt), NULL, 10);
            furi_string_printf(
                out,
                "%lu.%02lu V %s%%",
                mv / 1000,
                (mv % 1000) / 10,
                furi_string_get_cstr(percent));
        }
    }

    furi_string_free(value);
    furi_string_free(percent);
    furi_string_free(millivolt);
}
