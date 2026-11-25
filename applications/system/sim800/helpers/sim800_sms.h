// AT+CMGL listings, parsed into something drawable
#pragma once

#include <furi.h>
#include "sim800_types.h"

#define SIM800_SMS_MAX_PARTS 8

typedef struct {
    uint32_t index; //!< first storage slot
    uint32_t part_indices[SIM800_SMS_MAX_PARTS]; //!< all SIM slots belonging to this SMS
    uint8_t part_count;
    bool last_part_full; //!< true if the latest merged part hit GSM/UCS2 split length
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

/** Parse a single text-mode AT+CMGR response into @p entry. Returns false if slot was empty. */
bool sim800_sms_parse_cmgr(Sim800SmsEntry* entry, uint32_t index, const FuriString* response);

/** Shrink @p body in place to a single-line UTF-8 preview of at most @p max_chars codepoints. */
void sim800_sms_trim_preview(FuriString* body, size_t max_chars);

/** True if two GSM timestamps ("YY/MM/DD,HH:MM:SS+ZZ") are within 20 seconds of each other. */
bool sim800_sms_timestamps_close(const char* a, const char* b);

/** True if decoded UTF-8 @p body is long enough to be a non-final multipart SMS segment. */
bool sim800_sms_part_looks_full(const char* body);
