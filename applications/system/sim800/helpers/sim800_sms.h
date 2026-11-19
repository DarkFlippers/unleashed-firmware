// AT+CMGL listings, parsed into something drawable
#pragma once

#include <furi.h>
#include "sim800_types.h"

typedef struct {
    uint32_t index; //!< module storage slot, which is what AT+CMGD takes
    FuriString* status;
    FuriString* from;
    FuriString* date;
    FuriString* body;
} Sim800SmsEntry;

typedef struct {
    Sim800SmsEntry items[SIM800_INBOX_MAX];
    size_t count;
} Sim800Inbox;

void sim800_inbox_init(Sim800Inbox* inbox);
void sim800_inbox_free(Sim800Inbox* inbox);
void sim800_inbox_reset(Sim800Inbox* inbox);

/** Parse a text-mode AT+CMGL listing, up to ::SIM800_INBOX_MAX entries. */
void sim800_inbox_parse(Sim800Inbox* inbox, const FuriString* response);
