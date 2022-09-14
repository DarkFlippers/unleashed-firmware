#include "slideshow.h"

#include <stddef.h>
#include <storage/storage.h>
#include <gui/icon.h>
#include <gui/icon_i.h>
#include <core/dangerous_defines.h>

#define SLIDESHOW_MAGIC 0x72676468
#define SLIDESHOW_MAX_SUPPORTED_VERSION 1

struct Slideshow {
    Icon icon;
    uint32_t current_frame;
    bool loaded;
};

#pragma pack(push, 1)

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t width;
    uint8_t height;
    uint8_t frame_count;
} SlideshowFileHeader;
_Static_assert(sizeof(SlideshowFileHeader) == 8, "Incorrect SlideshowFileHeader size");

typedef struct {
    uint16_t size;
} SlideshowFrameHeader;
_Static_assert(sizeof(SlideshowFrameHeader) == 2, "Incorrect SlideshowFrameHeader size");

#pragma pack(pop)

Slideshow* slideshow_alloc() {
    Slideshow* ret = malloc(sizeof(Slideshow));
    ret->loaded = false;
    return ret;
}

void slideshow_free(Slideshow* slideshow) {
    Icon* icon = &slideshow->icon;
    if(icon) {
        for(int frame_idx = 0; frame_idx < icon->frame_count; ++frame_idx) {
            uint8_t* frame_data = (uint8_t*)icon->frames[frame_idx];
            free(frame_data);
        }
        free((uint8_t**)icon->frames);
    }
    free(slideshow);
}

bool slideshow_load(Slideshow* slideshow, const char* fspath) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* slideshow_file = storage_file_alloc(storage);
    slideshow->loaded = false;
    do {
        if(!storage_file_open(slideshow_file, fspath, FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }
        SlideshowFileHeader header;
        if((storage_file_read(slideshow_file, &header, sizeof(header)) != sizeof(header)) ||
           (header.magic != SLIDESHOW_MAGIC) ||
           (header.version > SLIDESHOW_MAX_SUPPORTED_VERSION)) {
            break;
        }
        Icon* icon = &slideshow->icon;
        FURI_CONST_ASSIGN(icon->frame_count, header.frame_count);
        FURI_CONST_ASSIGN(icon->width, header.width);
        FURI_CONST_ASSIGN(icon->height, header.height);
        icon->frames = malloc(header.frame_count * sizeof(uint8_t*));
        for(int frame_idx = 0; frame_idx < header.frame_count; ++frame_idx) {
            SlideshowFrameHeader frame_header;
            if(storage_file_read(slideshow_file, &frame_header, sizeof(frame_header)) !=
               sizeof(frame_header)) {
                break;
            }
            FURI_CONST_ASSIGN_PTR(icon->frames[frame_idx], malloc(frame_header.size));
            uint8_t* frame_data = (uint8_t*)icon->frames[frame_idx];
            if(storage_file_read(slideshow_file, frame_data, frame_header.size) !=
               frame_header.size) {
                break;
            }
            slideshow->loaded = (frame_idx + 1) == header.frame_count;
        }
    } while(false);
    storage_file_free(slideshow_file);
    furi_record_close(RECORD_STORAGE);
    return slideshow->loaded;
}

bool slideshow_is_loaded(Slideshow* slideshow) {
    return slideshow->loaded;
}

bool slideshow_is_one_page(Slideshow* slideshow) {
    return slideshow->loaded && (slideshow->icon.frame_count == 1);
}

bool slideshow_advance(Slideshow* slideshow) {
    uint8_t next_frame = slideshow->current_frame + 1;
    if(next_frame < slideshow->icon.frame_count) {
        slideshow->current_frame = next_frame;
        return true;
    }
    return false;
}

void slideshow_goback(Slideshow* slideshow) {
    if(slideshow->current_frame > 0) {
        slideshow->current_frame--;
    }
}

void slideshow_draw(Slideshow* slideshow, Canvas* canvas, uint8_t x, uint8_t y) {
    furi_assert(slideshow->current_frame < slideshow->icon.frame_count);
    canvas_draw_bitmap(
        canvas,
        x,
        y,
        slideshow->icon.width,
        slideshow->icon.height,
        slideshow->icon.frames[slideshow->current_frame]);
}
