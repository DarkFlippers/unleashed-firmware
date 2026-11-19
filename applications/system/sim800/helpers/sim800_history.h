/**
 * @file sim800_history.h
 * @brief Incoming calls and messages, logged to a file rather than held in RAM.
 *
 * Entries are appended as they arrive and only read back when a screen asks for them, so the
 * app carries no history in memory between visits. The file is trimmed from the front once it
 * passes ::SIM800_HISTORY_MAX so it cannot grow without end.
 */
#pragma once

#include <furi.h>

typedef enum {
    Sim800HistoryCall,
    Sim800HistorySms,
} Sim800HistoryKind;

/** Append one dated line. @p detail is the number, or the storage index for a message. */
void sim800_history_add(Sim800HistoryKind kind, const char* detail);

/** Append a message with its text, indented so the body stays with its header line. */
void sim800_history_add_sms(const char* from, const char* date, const char* body);

/** True if @p needle already appears in the log, to keep re-reads from logging twice. */
bool sim800_history_contains(const char* needle);

/** Read the log back, oldest first. @p kind_filter of -1 takes everything. */
bool sim800_history_read(FuriString* out, int32_t kind_filter);

void sim800_history_clear(void);
