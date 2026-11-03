/**
 * @file ra_raw_list_view.h
 * @brief What came out of a capture, in the shape the Sub-GHz app's own list has.
 *
 * Four rows of twelve with a fixed-code or rolling-code icon each, the selected one inverted,
 * a scrollbar once there are more than fit, and a line at the bottom saying what the capture
 * was. The icons are passed in: no I_* symbol reaches a plugin, and the app already ships the
 * three its Read screen draws.
 */
#pragma once

#include <gui/view.h>
#include <gui/icon.h>

/** Entries the list holds. A capture of a held down remote is one of them, not hundreds. */
#define RA_RAW_LIST_MAX 48

/** Icons the list indexes by the kind it is given. A slot may be NULL for no icon. */
#define RA_RAW_LIST_ICONS 4

typedef struct RaRawListView RaRawListView;

typedef void (*RaRawListCallback)(void* context, uint32_t index);

RaRawListView* ra_raw_list_view_alloc(void);
void ra_raw_list_view_free(RaRawListView* instance);
View* ra_raw_list_view_get_view(RaRawListView* instance);

/** Called on OK, with the index of the entry that was selected. */
void ra_raw_list_view_set_callback(
    RaRawListView* instance,
    RaRawListCallback callback,
    void* context);

/** @p icons holds RA_RAW_LIST_ICONS entries, indexed by the kind passed to _add(). */
void ra_raw_list_view_set_icons(RaRawListView* instance, const Icon** icons);

/** Append an entry. @p kind indexes the icons. False once the list is full. */
bool ra_raw_list_view_add(RaRawListView* instance, const char* label, uint8_t kind);

/** The line under the list: what the capture was, and how much came out of it. */
void ra_raw_list_view_set_footer(RaRawListView* instance, const char* text);
