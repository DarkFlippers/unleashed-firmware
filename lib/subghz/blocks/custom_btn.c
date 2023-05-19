#include "custom_btn.h"

static uint8_t custom_btn_id = SUBGHZ_CUSTOM_BTN_OK;
static uint8_t custom_btn_original = 0;
static uint8_t custom_btn_max_btns = 0;

bool subghz_custom_btn_set(uint8_t btn_id) {
    if(btn_id > custom_btn_max_btns) {
        custom_btn_id = SUBGHZ_CUSTOM_BTN_OK;
        return false;
    } else {
        custom_btn_id = btn_id;
        return true;
    }
}

uint8_t subghz_custom_btn_get() {
    return custom_btn_id;
}

void subghz_custom_btn_set_original(uint8_t btn_code) {
    custom_btn_original = btn_code;
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

bool subghz_custom_btn_is_allowed() {
    return custom_btn_max_btns != 0;
}