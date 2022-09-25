#pragma once

#include <m-array.h>
#include <m-string.h>

typedef struct {
    string_t text;
    uint32_t event;
} ArchiveContextMenuItem_t;

static void ArchiveContextMenuItem_t_init(ArchiveContextMenuItem_t* obj) {
    string_init(obj->text);
    obj->event = 0; // ArchiveBrowserEventFileMenuNone
}

static void ArchiveContextMenuItem_t_init_set(
    ArchiveContextMenuItem_t* obj,
    const ArchiveContextMenuItem_t* src) {
    string_init_set(obj->text, src->text);
    obj->event = src->event;
}

static void ArchiveContextMenuItem_t_set(
    ArchiveContextMenuItem_t* obj,
    const ArchiveContextMenuItem_t* src) {
    string_init_set(obj->text, src->text);
    obj->event = src->event;
}

static void ArchiveContextMenuItem_t_clear(ArchiveContextMenuItem_t* obj) {
    string_clear(obj->text);
}

ARRAY_DEF(
    menu_array,
    ArchiveContextMenuItem_t,
    (INIT(API_2(ArchiveContextMenuItem_t_init)),
     SET(API_6(ArchiveContextMenuItem_t_set)),
     INIT_SET(API_6(ArchiveContextMenuItem_t_init_set)),
     CLEAR(API_2(ArchiveContextMenuItem_t_clear))))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
// Using in applications/archive/views/archive_browser_view.c
static void archive_menu_add_item(ArchiveContextMenuItem_t* obj, string_t text, uint32_t event) {
    string_init_move(obj->text, text);
    obj->event = event;
}
#pragma GCC diagnostic pop