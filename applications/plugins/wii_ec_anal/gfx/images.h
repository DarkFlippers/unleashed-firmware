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
extern const image_t img_csLogo_Small;
extern const image_t img_3x5_v;
extern const image_t img_3x5_9;
extern const image_t img_3x5_8;
extern const image_t img_3x5_7;
extern const image_t img_3x5_6;
extern const image_t img_3x5_5;
extern const image_t img_3x5_4;
extern const image_t img_3x5_3;
extern const image_t img_3x5_2;
extern const image_t img_3x5_1;
extern const image_t img_3x5_0;
extern const image_t img_key_Ui;
extern const image_t img_key_OKi;
extern const image_t img_RIP;
extern const image_t img_cc_trg_R4;
extern const image_t img_cc_trg_R3;
extern const image_t img_cc_trg_R2;
extern const image_t img_cc_trg_R1;
extern const image_t img_cc_trg_L4;
extern const image_t img_cc_trg_L3;
extern const image_t img_cc_trg_L2;
extern const image_t img_cc_trg_L1;
extern const image_t img_cc_Joy;
extern const image_t img_cc_Main;
extern const image_t img_cc_Cable;
extern const image_t img_key_Back;
extern const image_t img_key_OK;
extern const image_t img_6x8_Z;
extern const image_t img_6x8_Y;
extern const image_t img_6x8_X;
extern const image_t img_key_U;
extern const image_t img_key_D;
extern const image_t img_csLogo_FULL;
extern const image_t img_6x8_7;
extern const image_t img_key_R;
extern const image_t img_key_L;
extern const image_t img_5x7_7;
extern const image_t img_5x7_F;
extern const image_t img_5x7_E;
extern const image_t img_5x7_D;
extern const image_t img_5x7_C;
extern const image_t img_5x7_B;
extern const image_t img_5x7_A;
extern const image_t img_5x7_9;
extern const image_t img_5x7_8;
extern const image_t img_5x7_6;
extern const image_t img_5x7_5;
extern const image_t img_5x7_4;
extern const image_t img_5x7_3;
extern const image_t img_5x7_2;
extern const image_t img_5x7_1;
extern const image_t img_5x7_0;
extern const image_t img_6x8_v;
extern const image_t img_6x8_n;
extern const image_t img_6x8_G;
extern const image_t img_6x8_F;
extern const image_t img_6x8_E;
extern const image_t img_6x8_d;
extern const image_t img_6x8_C;
extern const image_t img_6x8_B;
extern const image_t img_6x8_A;
extern const image_t img_6x8_9;
extern const image_t img_6x8_8;
extern const image_t img_6x8_6;
extern const image_t img_6x8_5;
extern const image_t img_6x8_4;
extern const image_t img_6x8_3;
extern const image_t img_6x8_2;
extern const image_t img_6x8_1;
extern const image_t img_6x8_0;
extern const image_t img_ecp_SDA;
extern const image_t img_ecp_SCL;
extern const image_t img_ecp_port;
extern const image_t img_cc_pad_UD1;
extern const image_t img_cc_pad_LR1;
extern const image_t img_cc_btn_Y1;
extern const image_t img_cc_btn_X1;
extern const image_t img_cc_btn_B1;
extern const image_t img_cc_btn_A1;
extern const image_t img_6x8_D;

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
