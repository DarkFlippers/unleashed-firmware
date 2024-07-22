#include "../js_modules.h"
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <expansion/expansion.h>

typedef struct {
    FuriHalAdcHandle* handle;
} JsGpioInst;

typedef struct {
    const GpioPin* pin;
    const char* name;
    const FuriHalAdcChannel channel;
} GpioPinCtx;

static const GpioPinCtx js_gpio_pins[] = {
    {.pin = &gpio_ext_pa7, .name = "PA7", .channel = FuriHalAdcChannel12}, // 2
    {.pin = &gpio_ext_pa6, .name = "PA6", .channel = FuriHalAdcChannel11}, // 3
    {.pin = &gpio_ext_pa4, .name = "PA4", .channel = FuriHalAdcChannel9}, // 4
    {.pin = &gpio_ext_pb3, .name = "PB3", .channel = FuriHalAdcChannelNone}, // 5
    {.pin = &gpio_ext_pb2, .name = "PB2", .channel = FuriHalAdcChannelNone}, // 6
    {.pin = &gpio_ext_pc3, .name = "PC3", .channel = FuriHalAdcChannel4}, // 7
    {.pin = &gpio_swclk, .name = "PA14", .channel = FuriHalAdcChannelNone}, // 10
    {.pin = &gpio_swdio, .name = "PA13", .channel = FuriHalAdcChannelNone}, // 12
    {.pin = &gpio_usart_tx, .name = "PB6", .channel = FuriHalAdcChannelNone}, // 13
    {.pin = &gpio_usart_rx, .name = "PB7", .channel = FuriHalAdcChannelNone}, // 14
    {.pin = &gpio_ext_pc1, .name = "PC1", .channel = FuriHalAdcChannel2}, // 15
    {.pin = &gpio_ext_pc0, .name = "PC0", .channel = FuriHalAdcChannel1}, // 16
    {.pin = &gpio_ibutton, .name = "PB14", .channel = FuriHalAdcChannelNone}, // 17
};

bool js_gpio_get_gpio_pull(const char* pull, GpioPull* value) {
    if(strcmp(pull, "no") == 0) {
        *value = GpioPullNo;
        return true;
    } else if(strcmp(pull, "up") == 0) {
        *value = GpioPullUp;
        return true;
    } else if(strcmp(pull, "down") == 0) {
        *value = GpioPullDown;
        return true;
    } else {
        *value = GpioPullNo;
        return true;
    }
    return false;
}

bool js_gpio_get_gpio_mode(const char* mode, GpioMode* value) {
    if(strcmp(mode, "input") == 0) {
        *value = GpioModeInput;
        return true;
    } else if(strcmp(mode, "outputPushPull") == 0) {
        *value = GpioModeOutputPushPull;
        return true;
    } else if(strcmp(mode, "outputOpenDrain") == 0) {
        *value = GpioModeOutputOpenDrain;
        return true;
    } else if(strcmp(mode, "altFunctionPushPull") == 0) {
        *value = GpioModeAltFunctionPushPull;
        return true;
    } else if(strcmp(mode, "altFunctionOpenDrain") == 0) {
        *value = GpioModeAltFunctionOpenDrain;
        return true;
    } else if(strcmp(mode, "analog") == 0) {
        *value = GpioModeAnalog;
        return true;
    } else if(strcmp(mode, "interruptRise") == 0) {
        *value = GpioModeInterruptRise;
        return true;
    } else if(strcmp(mode, "interruptFall") == 0) {
        *value = GpioModeInterruptFall;
        return true;
    } else if(strcmp(mode, "interruptRiseFall") == 0) {
        *value = GpioModeInterruptRiseFall;
        return true;
    } else if(strcmp(mode, "eventRise") == 0) {
        *value = GpioModeEventRise;
        return true;
    } else if(strcmp(mode, "eventFall") == 0) {
        *value = GpioModeEventFall;
        return true;
    } else if(strcmp(mode, "eventRiseFall") == 0) {
        *value = GpioModeEventRiseFall;
        return true;
    } else {
        return false;
    }
}

const GpioPin* js_gpio_get_gpio_pin(const char* name) {
    for(size_t i = 0; i < COUNT_OF(js_gpio_pins); i++) {
        if(strcmp(js_gpio_pins[i].name, name) == 0) {
            return js_gpio_pins[i].pin;
        }
    }
    return NULL;
}

FuriHalAdcChannel js_gpio_get_gpio_channel(const char* name) {
    for(size_t i = 0; i < COUNT_OF(js_gpio_pins); i++) {
        if(strcmp(js_gpio_pins[i].name, name) == 0) {
            return js_gpio_pins[i].channel;
        }
    }
    return FuriHalAdcChannelNone;
}

static void js_gpio_init(struct mjs* mjs) {
    mjs_val_t pin_arg = mjs_arg(mjs, 0);
    mjs_val_t mode_arg = mjs_arg(mjs, 1);
    mjs_val_t pull_arg = mjs_arg(mjs, 2);

    if(!mjs_is_string(pin_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a string");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* pin_name = mjs_get_string(mjs, &pin_arg, NULL);
    if(!pin_name) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Failed to get pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    if(!mjs_is_string(mode_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a string");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* mode_name = mjs_get_string(mjs, &mode_arg, NULL);
    if(!mode_name) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Failed to get mode name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    if(!mjs_is_string(pull_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a string");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* pull_name = mjs_get_string(mjs, &pull_arg, NULL);
    if(!pull_name) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Failed to get pull name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const GpioPin* gpio_pin = js_gpio_get_gpio_pin(pin_name);
    if(gpio_pin == NULL) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Invalid pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    GpioMode gpio_mode;
    if(!js_gpio_get_gpio_mode(mode_name, &gpio_mode)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Invalid mode name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    GpioPull gpio_pull;
    if(!js_gpio_get_gpio_pull(pull_name, &gpio_pull)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Invalid pull name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    expansion_disable(furi_record_open(RECORD_EXPANSION));
    furi_record_close(RECORD_EXPANSION);

    furi_hal_gpio_init(gpio_pin, gpio_mode, gpio_pull, GpioSpeedVeryHigh);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_gpio_write(struct mjs* mjs) {
    mjs_val_t pin_arg = mjs_arg(mjs, 0);
    mjs_val_t value_arg = mjs_arg(mjs, 1);

    if(!mjs_is_string(pin_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a string");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* pin_name = mjs_get_string(mjs, &pin_arg, NULL);
    if(!pin_name) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Failed to get pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    if(!mjs_is_boolean(value_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a boolean");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool value = mjs_get_bool(mjs, value_arg);

    const GpioPin* gpio_pin = js_gpio_get_gpio_pin(pin_name);

    if(gpio_pin == NULL) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Invalid pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    furi_hal_gpio_write(gpio_pin, value);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_gpio_read(struct mjs* mjs) {
    mjs_val_t pin_arg = mjs_arg(mjs, 0);

    if(!mjs_is_string(pin_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a string");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* pin_name = mjs_get_string(mjs, &pin_arg, NULL);
    if(!pin_name) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Failed to get pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const GpioPin* gpio_pin = js_gpio_get_gpio_pin(pin_name);

    if(gpio_pin == NULL) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Invalid pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool value = furi_hal_gpio_read(gpio_pin);

    mjs_return(mjs, mjs_mk_boolean(mjs, value));
}

static void js_gpio_read_analog(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsGpioInst* gpio = mjs_get_ptr(mjs, obj_inst);
    furi_assert(gpio);

    if(gpio->handle == NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Analog mode not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_val_t pin_arg = mjs_arg(mjs, 0);

    if(!mjs_is_string(pin_arg)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a string");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* pin_name = mjs_get_string(mjs, &pin_arg, NULL);
    if(!pin_name) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Failed to get pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    FuriHalAdcChannel channel = js_gpio_get_gpio_channel(pin_name);
    if(channel == FuriHalAdcChannelNone) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Invalid pin name");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    uint16_t adc_value = furi_hal_adc_read(gpio->handle, channel);
    float adc_mv = furi_hal_adc_convert_to_voltage(gpio->handle, adc_value);

    mjs_return(mjs, mjs_mk_number(mjs, adc_mv));
}

static void js_gpio_start_analog(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsGpioInst* gpio = mjs_get_ptr(mjs, obj_inst);
    furi_assert(gpio);

    FuriHalAdcScale scale = FuriHalAdcScale2048;
    if(mjs_nargs(mjs) > 0) {
        mjs_val_t scale_arg = mjs_arg(mjs, 0);

        if(!mjs_is_number(scale_arg)) {
            mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a number");
            mjs_return(mjs, MJS_UNDEFINED);
            return;
        }

        int32_t scale_num = mjs_get_int32(mjs, scale_arg);
        if(scale_num == 2048 || scale_num == 2000) { // 2 volt reference
            scale = FuriHalAdcScale2048;
        } else if(scale_num == 2500) { // 2.5 volt reference
            scale = FuriHalAdcScale2500;
        } else {
            mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Invalid scale");
            mjs_return(mjs, MJS_UNDEFINED);
            return;
        }
    }

    if(gpio->handle != NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Analog mode already started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    gpio->handle = furi_hal_adc_acquire();
    furi_hal_adc_configure_ex(
        gpio->handle,
        scale,
        FuriHalAdcClockSync64,
        FuriHalAdcOversample64,
        FuriHalAdcSamplingtime247_5);
}

static void js_gpio_stop_analog(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsGpioInst* gpio = mjs_get_ptr(mjs, obj_inst);
    furi_assert(gpio);

    if(gpio->handle == NULL) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Analog mode not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    furi_hal_adc_release(gpio->handle);
    gpio->handle = NULL;
}

static void* js_gpio_create(struct mjs* mjs, mjs_val_t* object) {
    JsGpioInst* gpio = malloc(sizeof(JsGpioInst));
    gpio->handle = NULL;
    mjs_val_t gpio_obj = mjs_mk_object(mjs);
    mjs_set(mjs, gpio_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, gpio));
    mjs_set(mjs, gpio_obj, "init", ~0, MJS_MK_FN(js_gpio_init));
    mjs_set(mjs, gpio_obj, "write", ~0, MJS_MK_FN(js_gpio_write));
    mjs_set(mjs, gpio_obj, "read", ~0, MJS_MK_FN(js_gpio_read));
    mjs_set(mjs, gpio_obj, "readAnalog", ~0, MJS_MK_FN(js_gpio_read_analog));
    mjs_set(mjs, gpio_obj, "startAnalog", ~0, MJS_MK_FN(js_gpio_start_analog));
    mjs_set(mjs, gpio_obj, "stopAnalog", ~0, MJS_MK_FN(js_gpio_stop_analog));
    *object = gpio_obj;

    return (void*)gpio;
}

static void js_gpio_destroy(void* inst) {
    if(inst != NULL) {
        JsGpioInst* gpio = (JsGpioInst*)inst;
        if(gpio->handle != NULL) {
            furi_hal_adc_release(gpio->handle);
            gpio->handle = NULL;
        }
        free(gpio);
    }

    // loop through all pins and reset them to analog mode
    for(size_t i = 0; i < COUNT_OF(js_gpio_pins); i++) {
        furi_hal_gpio_write(js_gpio_pins[i].pin, false);
        furi_hal_gpio_init(js_gpio_pins[i].pin, GpioModeAnalog, GpioPullNo, GpioSpeedVeryHigh);
    }

    expansion_enable(furi_record_open(RECORD_EXPANSION));
    furi_record_close(RECORD_EXPANSION);
}

static const JsModuleDescriptor js_gpio_desc = {
    "gpio",
    js_gpio_create,
    js_gpio_destroy,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_gpio_desc,
};

const FlipperAppPluginDescriptor* js_gpio_ep(void) {
    return &plugin_descriptor;
}
