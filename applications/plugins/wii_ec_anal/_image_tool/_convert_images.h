#ifndef IMAGES_H_
#define IMAGES_H_

#include <stdint.h>
#include <stdbool.h>

//----------------------------------------------------------------------------- ----------------------------------------
typedef enum showMode {
    // {INV:--:WHT:BLK::--:--:CLR:SET}
    SHOW_SET_ = 0x01,
    SHOW_CLR_ = 0x02,
    SHOW_ALL_ = SHOW_SET_ | SHOW_CLR_,

    SHOW_BLK_ = 0x10,
    SHOW_WHT_ = 0x20,
    SHOW_NRM_ = 0x00,
    SHOW_INV_ = SHOW_BLK_ | SHOW_WHT_,

    SHOW_SET_BLK = SHOW_SET_ | SHOW_BLK_,
    SHOW_SET_WHT = SHOW_SET_ | SHOW_WHT_,

    SHOW_CLR_BLK = SHOW_CLR_ | SHOW_BLK_,
    SHOW_CLR_WHT = SHOW_CLR_ | SHOW_WHT_,

    SHOW_ALL = SHOW_ALL_ | SHOW_NRM_,
    SHOW_ALL_INV = SHOW_ALL_ | SHOW_INV_,
} showMode_t;

//----------------------------------------------------------------------------- ----------------------------------------
typedef struct image {
    uint8_t w; // width
    uint8_t h; // height
    bool c; // compressed?
    uint16_t len; // image data length
    uint8_t tag; // rle tag
    uint8_t data[]; // image data
} image_t;

//----------------------------------------------------------------------------- ----------------------------------------
//[TAG]

//----------------------------------------------------------------------------- ----------------------------------------
#ifndef IMGTEST
#include <gui/gui.h>
void show(
    Canvas* const canvas,
    const uint8_t tlx,
    const uint8_t tly,
    const image_t* img,
    const showMode_t mode);
#endif

#endif //IMAGES_H_
