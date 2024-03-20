#include <core/common_defines.h>
#include "../js_modules.h"
#include <furi_hal.h>

typedef struct {
    FuriHalUsbHidConfig* hid_cfg;
    FuriHalUsbInterface* usb_if_prev;
    uint8_t key_hold_cnt;
} JsBadusbInst;

static const struct {
    char* name;
    uint16_t code;
} key_codes[] = {
    {"CTRL", KEY_MOD_LEFT_CTRL},
    {"SHIFT", KEY_MOD_LEFT_SHIFT},
    {"ALT", KEY_MOD_LEFT_ALT},
    {"GUI", KEY_MOD_LEFT_GUI},

    {"DOWN", HID_KEYBOARD_DOWN_ARROW},
    {"LEFT", HID_KEYBOARD_LEFT_ARROW},
    {"RIGHT", HID_KEYBOARD_RIGHT_ARROW},
    {"UP", HID_KEYBOARD_UP_ARROW},

    {"ENTER", HID_KEYBOARD_RETURN},
    {"PAUSE", HID_KEYBOARD_PAUSE},
    {"CAPSLOCK", HID_KEYBOARD_CAPS_LOCK},
    {"DELETE", HID_KEYBOARD_DELETE_FORWARD},
    {"BACKSPACE", HID_KEYBOARD_DELETE},
    {"END", HID_KEYBOARD_END},
    {"ESC", HID_KEYBOARD_ESCAPE},
    {"HOME", HID_KEYBOARD_HOME},
    {"INSERT", HID_KEYBOARD_INSERT},
    {"NUMLOCK", HID_KEYPAD_NUMLOCK},
    {"PAGEUP", HID_KEYBOARD_PAGE_UP},
    {"PAGEDOWN", HID_KEYBOARD_PAGE_DOWN},
    {"PRINTSCREEN", HID_KEYBOARD_PRINT_SCREEN},
    {"SCROLLLOCK", HID_KEYBOARD_SCROLL_LOCK},
    {"SPACE", HID_KEYBOARD_SPACEBAR},
    {"TAB", HID_KEYBOARD_TAB},
    {"MENU", HID_KEYBOARD_APPLICATION},

    {"F1", HID_KEYBOARD_F1},
    {"F2", HID_KEYBOARD_F2},
    {"F3", HID_KEYBOARD_F3},
    {"F4", HID_KEYBOARD_F4},
    {"F5", HID_KEYBOARD_F5},
    {"F6", HID_KEYBOARD_F6},
    {"F7", HID_KEYBOARD_F7},
    {"F8", HID_KEYBOARD_F8},
    {"F9", HID_KEYBOARD_F9},
    {"F10", HID_KEYBOARD_F10},
    {"F11", HID_KEYBOARD_F11},
    {"F12", HID_KEYBOARD_F12},
    {"F13", HID_KEYBOARD_F13},
    {"F14", HID_KEYBOARD_F14},
    {"F15", HID_KEYBOARD_F15},
    {"F16", HID_KEYBOARD_F16},
    {"F17", HID_KEYBOARD_F17},
    {"F18", HID_KEYBOARD_F18},
    {"F19", HID_KEYBOARD_F19},
    {"F20", HID_KEYBOARD_F20},
    {"F21", HID_KEYBOARD_F21},
    {"F22", HID_KEYBOARD_F22},
    {"F23", HID_KEYBOARD_F23},
    {"F24", HID_KEYBOARD_F24},
};

static bool setup_parse_params(struct mjs* mjs, mjs_val_t arg, FuriHalUsbHidConfig* hid_cfg) {
    if(!mjs_is_object(arg)) {
        return false;
    }
    mjs_val_t vid_obj = mjs_get(mjs, arg, "vid", ~0);
    mjs_val_t pid_obj = mjs_get(mjs, arg, "pid", ~0);
    mjs_val_t mfr_obj = mjs_get(mjs, arg, "mfr_name", ~0);
    mjs_val_t prod_obj = mjs_get(mjs, arg, "prod_name", ~0);

    if(mjs_is_number(vid_obj) && mjs_is_number(pid_obj)) {
        hid_cfg->vid = mjs_get_int32(mjs, vid_obj);
        hid_cfg->pid = mjs_get_int32(mjs, pid_obj);
    } else {
        return false;
    }

    if(mjs_is_string(mfr_obj)) {
        size_t str_len = 0;
        const char* str_temp = mjs_get_string(mjs, &mfr_obj, &str_len);
        if((str_len == 0) || (str_temp == NULL)) {
            return false;
        }
        strlcpy(hid_cfg->manuf, str_temp, sizeof(hid_cfg->manuf));
    }

    if(mjs_is_string(prod_obj)) {
        size_t str_len = 0;
        const char* str_temp = mjs_get_string(mjs, &prod_obj, &str_len);
        if((str_len == 0) || (str_temp == NULL)) {
            return false;
        }
        strlcpy(hid_cfg->product, str_temp, sizeof(hid_cfg->product));
    }

    return true;
}

static void js_badusb_setup(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsBadusbInst* badusb = mjs_get_ptr(mjs, obj_inst);
    furi_assert(badusb);

    if(badusb->usb_if_prev) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "HID is already started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool args_correct = false;
    size_t num_args = mjs_nargs(mjs);
    if(num_args == 0) {
        // No arguments: start USB HID with default settings
        args_correct = true;
    } else if(num_args == 1) {
        badusb->hid_cfg = malloc(sizeof(FuriHalUsbHidConfig));
        // Parse argument object
        args_correct = setup_parse_params(mjs, mjs_arg(mjs, 0), badusb->hid_cfg);
    }
    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    badusb->usb_if_prev = furi_hal_usb_get_config();

    if(!furi_hal_usb_set_config(&usb_hid, badusb->hid_cfg)) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "USB is locked, close companion app first");
        badusb->usb_if_prev = NULL;
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_badusb_is_connected(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsBadusbInst* badusb = mjs_get_ptr(mjs, obj_inst);
    furi_assert(badusb);

    if(badusb->usb_if_prev == NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "HID is not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool is_connected = furi_hal_hid_is_connected();
    mjs_return(mjs, mjs_mk_boolean(mjs, is_connected));
}

uint16_t get_keycode_by_name(const char* key_name, size_t name_len) {
    if(name_len == 1) { // Single char
        return (HID_ASCII_TO_KEY(key_name[0]));
    }

    for(size_t i = 0; i < COUNT_OF(key_codes); i++) {
        size_t key_cmd_len = strlen(key_codes[i].name);
        if(key_cmd_len != name_len) {
            continue;
        }

        if(strncmp(key_name, key_codes[i].name, name_len) == 0) {
            return key_codes[i].code;
        }
    }

    return HID_KEYBOARD_NONE;
}

static bool parse_keycode(struct mjs* mjs, size_t nargs, uint16_t* keycode) {
    uint16_t key_tmp = 0;
    for(size_t i = 0; i < nargs; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);
        if(mjs_is_string(arg)) {
            size_t name_len = 0;
            const char* key_name = mjs_get_string(mjs, &arg, &name_len);
            if((key_name == NULL) || (name_len == 0)) {
                // String error
                return false;
            }
            uint16_t str_key = get_keycode_by_name(key_name, name_len);
            if(str_key == HID_KEYBOARD_NONE) {
                // Unknown key code
                return false;
            }
            if((str_key & 0xFF) && (key_tmp & 0xFF)) {
                // Main key is already defined
                return false;
            }
            key_tmp |= str_key;
        } else if(mjs_is_number(arg)) {
            uint32_t keycode_number = (uint32_t)mjs_get_int32(mjs, arg);
            if(((key_tmp & 0xFF) != 0) || (keycode_number > 0xFF)) {
                return false;
            }
            key_tmp |= keycode_number & 0xFF;
        } else {
            return false;
        }
    }
    *keycode = key_tmp;
    return true;
}

static void js_badusb_press(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsBadusbInst* badusb = mjs_get_ptr(mjs, obj_inst);
    furi_assert(badusb);
    if(badusb->usb_if_prev == NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "HID is not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool args_correct = false;
    uint16_t keycode = HID_KEYBOARD_NONE;
    size_t num_args = mjs_nargs(mjs);
    if(num_args > 0) {
        args_correct = parse_keycode(mjs, num_args, &keycode);
    }
    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    furi_hal_hid_kb_press(keycode);
    furi_hal_hid_kb_release(keycode);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_badusb_hold(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsBadusbInst* badusb = mjs_get_ptr(mjs, obj_inst);
    furi_assert(badusb);
    if(badusb->usb_if_prev == NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "HID is not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool args_correct = false;
    uint16_t keycode = HID_KEYBOARD_NONE;
    size_t num_args = mjs_nargs(mjs);
    if(num_args > 0) {
        args_correct = parse_keycode(mjs, num_args, &keycode);
    }
    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    if(keycode & 0xFF) {
        badusb->key_hold_cnt++;
        if(badusb->key_hold_cnt > (HID_KB_MAX_KEYS - 1)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Too many keys are hold");
            furi_hal_hid_kb_release_all();
            mjs_return(mjs, MJS_UNDEFINED);
            return;
        }
    }
    furi_hal_hid_kb_press(keycode);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_badusb_release(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsBadusbInst* badusb = mjs_get_ptr(mjs, obj_inst);
    furi_assert(badusb);
    if(badusb->usb_if_prev == NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "HID is not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool args_correct = false;
    uint16_t keycode = HID_KEYBOARD_NONE;
    size_t num_args = mjs_nargs(mjs);
    if(num_args == 0) {
        furi_hal_hid_kb_release_all();
        badusb->key_hold_cnt = 0;
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    } else {
        args_correct = parse_keycode(mjs, num_args, &keycode);
    }
    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    if((keycode & 0xFF) && (badusb->key_hold_cnt > 0)) {
        badusb->key_hold_cnt--;
    }
    furi_hal_hid_kb_release(keycode);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void badusb_print(struct mjs* mjs, bool ln) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsBadusbInst* badusb = mjs_get_ptr(mjs, obj_inst);
    furi_assert(badusb);
    if(badusb->usb_if_prev == NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "HID is not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    bool args_correct = false;
    const char* text_str = NULL;
    size_t text_len = 0;
    uint32_t delay_val = 0;
    do {
        mjs_val_t obj_string = MJS_UNDEFINED;
        size_t num_args = mjs_nargs(mjs);
        if(num_args == 1) {
            obj_string = mjs_arg(mjs, 0);
        } else if(num_args == 2) {
            obj_string = mjs_arg(mjs, 0);
            mjs_val_t obj_delay = mjs_arg(mjs, 1);
            if(!mjs_is_number(obj_delay)) {
                break;
            }
            delay_val = (uint32_t)mjs_get_int32(mjs, obj_delay);
            if(delay_val > 60000) {
                break;
            }
        }

        if(!mjs_is_string(obj_string)) {
            break;
        }
        text_str = mjs_get_string(mjs, &obj_string, &text_len);
        if((text_str == NULL) || (text_len == 0)) {
            break;
        }
        args_correct = true;
    } while(0);

    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    for(size_t i = 0; i < text_len; i++) {
        uint16_t keycode = HID_ASCII_TO_KEY(text_str[i]);
        furi_hal_hid_kb_press(keycode);
        furi_hal_hid_kb_release(keycode);
        if(delay_val > 0) {
            bool need_exit = js_delay_with_flags(mjs, delay_val);
            if(need_exit) {
                mjs_return(mjs, MJS_UNDEFINED);
                return;
            }
        }
    }
    if(ln) {
        furi_hal_hid_kb_press(HID_KEYBOARD_RETURN);
        furi_hal_hid_kb_release(HID_KEYBOARD_RETURN);
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_badusb_print(struct mjs* mjs) {
    badusb_print(mjs, false);
}

static void js_badusb_println(struct mjs* mjs) {
    badusb_print(mjs, true);
}

static void* js_badusb_create(struct mjs* mjs, mjs_val_t* object) {
    JsBadusbInst* badusb = malloc(sizeof(JsBadusbInst));
    mjs_val_t badusb_obj = mjs_mk_object(mjs);
    mjs_set(mjs, badusb_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, badusb));
    mjs_set(mjs, badusb_obj, "setup", ~0, MJS_MK_FN(js_badusb_setup));
    mjs_set(mjs, badusb_obj, "isConnected", ~0, MJS_MK_FN(js_badusb_is_connected));
    mjs_set(mjs, badusb_obj, "press", ~0, MJS_MK_FN(js_badusb_press));
    mjs_set(mjs, badusb_obj, "hold", ~0, MJS_MK_FN(js_badusb_hold));
    mjs_set(mjs, badusb_obj, "release", ~0, MJS_MK_FN(js_badusb_release));
    mjs_set(mjs, badusb_obj, "print", ~0, MJS_MK_FN(js_badusb_print));
    mjs_set(mjs, badusb_obj, "println", ~0, MJS_MK_FN(js_badusb_println));
    *object = badusb_obj;
    return badusb;
}

static void js_badusb_destroy(void* inst) {
    JsBadusbInst* badusb = inst;
    if(badusb->usb_if_prev) {
        furi_hal_hid_kb_release_all();
        furi_check(furi_hal_usb_set_config(badusb->usb_if_prev, NULL));
    }
    if(badusb->hid_cfg) {
        free(badusb->hid_cfg);
    }
    free(badusb);
}

static const JsModuleDescriptor js_badusb_desc = {
    "badusb",
    js_badusb_create,
    js_badusb_destroy,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_badusb_desc,
};

const FlipperAppPluginDescriptor* js_badusb_ep(void) {
    return &plugin_descriptor;
}
