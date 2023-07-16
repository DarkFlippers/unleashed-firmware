#pragma once

#include <furi.h>
#include <m-array.h>

typedef struct {
    FuriString* text;
    uint32_t event;
} ArchiveContextMenuItem_t;

static void ArchiveContextMenuItem_t_init(ArchiveContextMenuItem_t* obj) {
    obj->text = furi_string_alloc();
    obj->event = 0; // ArchiveBrowserEventFileMenuNone
}

static void ArchiveContextMenuItem_t_init_set(
    ArchiveContextMenuItem_t* obj,
    const ArchiveContextMenuItem_t* src) {
    obj->text = furi_string_alloc_set(src->text);
    obj->event = src->event;
}

static void ArchiveContextMenuItem_t_set(
    ArchiveContextMenuItem_t* obj,
    const ArchiveContextMenuItem_t* src) {
    furi_string_set(obj->text, src->text);
    obj->event = src->event;
}

static void ArchiveContextMenuItem_t_clear(ArchiveContextMenuItem_t* obj) {
    furi_string_free(obj->text);
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
static void
    archive_menu_add_item(ArchiveContextMenuItem_t* obj, const char* text, uint32_t event) {
    obj->text = furi_string_alloc_set(text);
    obj->event = event;
}
#pragma GCC diagnostic pop