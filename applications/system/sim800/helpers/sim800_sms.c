#include "sim800_sms.h"
#include "sim800_uart.h"

#define SIM800_CMGL_PREFIX "+CMGL:"

void sim800_inbox_init(Sim800Inbox* inbox) {
    furi_assert(inbox);
    for(size_t i = 0; i < SIM800_INBOX_MAX; i++) {
        inbox->items[i].index = 0;
        inbox->items[i].status = furi_string_alloc();
        inbox->items[i].from = furi_string_alloc();
        inbox->items[i].date = furi_string_alloc();
        inbox->items[i].body = furi_string_alloc();
    }
    inbox->count = 0;
}

void sim800_inbox_free(Sim800Inbox* inbox) {
    furi_assert(inbox);
    for(size_t i = 0; i < SIM800_INBOX_MAX; i++) {
        furi_string_free(inbox->items[i].status);
        furi_string_free(inbox->items[i].from);
        furi_string_free(inbox->items[i].date);
        furi_string_free(inbox->items[i].body);
    }
    inbox->count = 0;
}

void sim800_inbox_reset(Sim800Inbox* inbox) {
    furi_assert(inbox);
    for(size_t i = 0; i < SIM800_INBOX_MAX; i++) {
        furi_string_reset(inbox->items[i].status);
        furi_string_reset(inbox->items[i].from);
        furi_string_reset(inbox->items[i].date);
        furi_string_reset(inbox->items[i].body);
    }
    inbox->count = 0;
}

void sim800_inbox_parse(Sim800Inbox* inbox, const FuriString* response) {
    furi_assert(inbox);
    furi_assert(response);

    sim800_inbox_reset(inbox);

    FuriString* line = furi_string_alloc();
    FuriString* field = furi_string_alloc();
    const char* cursor = furi_string_get_cstr(response);
    Sim800SmsEntry* current = NULL;

    while(*cursor != '\0') {
        const char* end = strchr(cursor, '\n');
        if(end) {
            furi_string_set_strn(line, cursor, (size_t)(end - cursor));
            cursor = end + 1;
        } else {
            furi_string_set_str(line, cursor);
            cursor += strlen(cursor);
        }

        if(furi_string_start_with_str(line, SIM800_CMGL_PREFIX)) {
            if(inbox->count >= SIM800_INBOX_MAX) break;
            current = &inbox->items[inbox->count];
            inbox->count++;

            const char* header = furi_string_get_cstr(line) + strlen(SIM800_CMGL_PREFIX);
            while(*header == ' ')
                header++;

            if(sim800_csv_field(header, 0, field)) {
                current->index = (uint32_t)strtoul(furi_string_get_cstr(field), NULL, 10);
            }
            if(sim800_csv_field(header, 1, field)) furi_string_set(current->status, field);
            if(sim800_csv_field(header, 2, field)) furi_string_set(current->from, field);
            // Field 3 is the alphanumeric name, which is empty on most networks
            if(sim800_csv_field(header, 4, field)) furi_string_set(current->date, field);
            continue;
        }

        if(current == NULL) continue; // header lines before the first message

        if(furi_string_size(current->body) > 0) furi_string_cat_str(current->body, "\n");
        furi_string_cat(current->body, line);
    }

    furi_string_free(line);
    furi_string_free(field);
}
