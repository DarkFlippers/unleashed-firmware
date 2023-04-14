#include "custom_btn.h"

static uint8_t custom_btn_id = 0;
static uint8_t custom_btn_original = 0;
static uint8_t custom_btn_max_btns = 0;

void subghz_custom_btn_set(uint8_t b) {
    if(b > custom_btn_max_btns) {
        custom_btn_id = 0;
    } else {
        custom_btn_id = b;
    }
}

uint8_t subghz_custom_btn_get() {
    return custom_btn_id;
}

void subghz_custom_btn_set_original(uint8_t b) {
    custom_btn_original = b;
}

uint8_t subghz_custom_btn_get_original() {
    return custom_btn_original;
}

void subghz_custom_btn_set_max(uint8_t b) {
    custom_btn_max_btns = b;
}

void subghz_custom_btns_reset() {
    custom_btn_original = 0;
    custom_btn_max_btns = 0;
}