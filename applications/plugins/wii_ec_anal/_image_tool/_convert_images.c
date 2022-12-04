#include <gui/gui.h> // GUI (screen/keyboard) API

#include "images.h"

//----------------------------------------------------------------------------- ----------------------------------------
static Canvas* _canvas;
static uint8_t _tlx;
static uint8_t _tly;

static uint8_t _x;
static uint8_t _y;

static const image_t* _img;

static bool _blk;
static Color _set;
static Color _clr;

//+============================================================================
static void _showByteSet(const uint8_t b) {
    for(uint8_t m = 0x80; m; m >>= 1) {
        if(b & m) // plot only SET bits
            canvas_draw_dot(_canvas, (_tlx + _x), (_tly + _y));
        if(((++_x) == _img->w) && !(_x = 0) && ((++_y) == _img->h)) break;
    }
}

//+============================================================================
static void _showByteClr(const uint8_t b) {
    for(uint8_t m = 0x80; m; m >>= 1) {
        if(!(b & m)) // plot only CLR bits
            canvas_draw_dot(_canvas, (_tlx + _x), (_tly + _y));
        if(((++_x) == _img->w) && !(_x = 0) && ((++_y) == _img->h)) break;
    }
}

//+============================================================================
static void _showByteAll(const uint8_t b) {
    for(uint8_t m = 0x80; m; m >>= 1) {
        if((!!(b & m)) ^ _blk) { // Change colour only when required
            canvas_set_color(_canvas, ((b & m) ? _set : _clr));
            _blk = !_blk;
        }
        canvas_draw_dot(_canvas, (_tlx + _x), (_tly + _y));
        if(((++_x) == _img->w) && !(_x = 0) && ((++_y) == _img->h)) break;
    }
}

//+============================================================================
// available modes are SHOW_SET_BLK - plot image pixels that are  SET in BLACK
//                     SHOW_XOR     - same as SET_BLACK
//                     SHOW_SET_WHT - plot image pixels that are  SET in WHITE
//                     SHOW_CLR_BLK - plot image pixels that are CLEAR in BLACK
//                     SHOW_CLR_WHT - plot image pixels that are CLEAR in WHITE
//                     SHOW_ALL     - plot all images pixels as they are
//                     SHOW_ALL_INV - plot all images pixels inverted
//
void show(
    Canvas* const canvas,
    const uint8_t tlx,
    const uint8_t tly,
    const image_t* img,
    const showMode_t mode) {
    void (*fnShow)(const uint8_t) = NULL;

    const uint8_t* bp = img->data;

    // code size optimisation
    switch(mode & SHOW_INV_) {
    case SHOW_NRM_:
        _set = ColorBlack;
        _clr = ColorWhite;
        break;

    case SHOW_INV_:
        _set = ColorWhite;
        _clr = ColorBlack;
        break;

    case SHOW_BLK_:
        canvas_set_color(canvas, ColorBlack);
        break;

    case SHOW_WHT_:
        canvas_set_color(canvas, ColorWhite);
        break;
    }
    switch(mode & SHOW_INV_) {
    case SHOW_NRM_:
    case SHOW_INV_:
        fnShow = _showByteAll;
        canvas_set_color(canvas, ColorWhite);
        _blk = 0;
        break;

    case SHOW_BLK_:
    case SHOW_WHT_:
        switch(mode & SHOW_ALL_) {
        case SHOW_SET_:
            fnShow = _showByteSet;
            break;
        case SHOW_CLR_:
            fnShow = _showByteClr;
            break;
        }
        break;
    }
    furi_check(fnShow);

    // I want nested functions!
    _canvas = canvas;
    _img = img;
    _tlx = tlx;
    _tly = tly;
    _x = 0;
    _y = 0;

    // Compressed
    if(img->c) {
        for(unsigned int i = 0; i < img->len; i++, bp++) {
            // Compressed data? {tag, length, value}
            if(*bp == img->tag) {
                for(uint16_t c = 0; c < bp[1]; c++) fnShow(bp[2]);
                bp += 3 - 1;
                i += 3 - 1;

                // Uncompressed byte
            } else {
                fnShow(*bp);
            }
        }

        // Not compressed
    } else {
        for(unsigned int i = 0; i < img->len; i++, bp++) fnShow(*bp);
    }
}
