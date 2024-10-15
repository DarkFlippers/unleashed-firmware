#include "../js_modules.h" // IWYU pragma: keep
#include "./js_event_loop/js_event_loop.h"
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <expansion/expansion.h>
#include <limits.h>
#include <mlib/m-array.h>

#define INTERRUPT_QUEUE_LEN 16

/**
 * Per-pin control structure
 */
typedef struct {
    const GpioPin* pin;
    bool had_interrupt;
    FuriSemaphore* interrupt_semaphore;
    JsEventLoopContract* interrupt_contract;
    FuriHalAdcChannel adc_channel;
    FuriHalAdcHandle* adc_handle;
} JsGpioPinInst;

ARRAY_DEF(ManagedPinsArray, JsGpioPinInst*, M_PTR_OPLIST); //-V575

/**
 * Per-module instance control structure
 */
typedef struct {
    FuriEventLoop* loop;
    ManagedPinsArray_t managed_pins;
    FuriHalAdcHandle* adc_handle;
} JsGpioInst;

/**
 * @brief Interrupt callback
 */
static void js_gpio_int_cb(void* arg) {
    furi_assert(arg);
    FuriSemaphore* semaphore = arg;
    furi_semaphore_release(semaphore);
}

/**
 * @brief Initializes a GPIO pin according to the provided mode object
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let led = gpio.get("pc3");
 * led.init({ direction: "out", outMode: "push_pull" });
 * ```
 */
static void js_gpio_init(struct mjs* mjs) {
    // deconstruct mode object
    mjs_val_t mode_arg;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_OBJ(&mode_arg));
    mjs_val_t direction_arg = mjs_get(mjs, mode_arg, "direction", ~0);
    mjs_val_t out_mode_arg = mjs_get(mjs, mode_arg, "outMode", ~0);
    mjs_val_t in_mode_arg = mjs_get(mjs, mode_arg, "inMode", ~0);
    mjs_val_t edge_arg = mjs_get(mjs, mode_arg, "edge", ~0);
    mjs_val_t pull_arg = mjs_get(mjs, mode_arg, "pull", ~0);

    // get strings
    const char* direction = mjs_get_string(mjs, &direction_arg, NULL);
    const char* out_mode = mjs_get_string(mjs, &out_mode_arg, NULL);
    const char* in_mode = mjs_get_string(mjs, &in_mode_arg, NULL);
    const char* edge = mjs_get_string(mjs, &edge_arg, NULL);
    const char* pull = mjs_get_string(mjs, &pull_arg, NULL);
    if(!direction)
        JS_ERROR_AND_RETURN(
            mjs, MJS_BAD_ARGS_ERROR, "Expected string in \"direction\" field of mode object");
    if(!out_mode) out_mode = "open_drain";
    if(!in_mode) in_mode = "plain_digital";
    if(!edge) edge = "rising";

    // convert strings to mode
    GpioMode mode;
    if(strcmp(direction, "out") == 0) {
        if(strcmp(out_mode, "push_pull") == 0)
            mode = GpioModeOutputPushPull;
        else if(strcmp(out_mode, "open_drain") == 0)
            mode = GpioModeOutputOpenDrain;
        else
            JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "invalid outMode");
    } else if(strcmp(direction, "in") == 0) {
        if(strcmp(in_mode, "analog") == 0) {
            mode = GpioModeAnalog;
        } else if(strcmp(in_mode, "plain_digital") == 0) {
            mode = GpioModeInput;
        } else if(strcmp(in_mode, "interrupt") == 0) {
            if(strcmp(edge, "rising") == 0)
                mode = GpioModeInterruptRise;
            else if(strcmp(edge, "falling") == 0)
                mode = GpioModeInterruptFall;
            else if(strcmp(edge, "both") == 0)
                mode = GpioModeInterruptRiseFall;
            else
                JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "invalid edge");
        } else if(strcmp(in_mode, "event") == 0) {
            if(strcmp(edge, "rising") == 0)
                mode = GpioModeEventRise;
            else if(strcmp(edge, "falling") == 0)
                mode = GpioModeEventFall;
            else if(strcmp(edge, "both") == 0)
                mode = GpioModeEventRiseFall;
            else
                JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "invalid edge");
        } else {
            JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "invalid inMode");
        }
    } else {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "invalid direction");
    }

    // convert pull
    GpioPull pull_mode;
    if(!pull) {
        pull_mode = GpioPullNo;
    } else if(strcmp(pull, "up") == 0) {
        pull_mode = GpioPullUp;
    } else if(strcmp(pull, "down") == 0) {
        pull_mode = GpioPullDown;
    } else {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "invalid pull");
    }

    // init GPIO
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    furi_hal_gpio_init(manager_data->pin, mode, pull_mode, GpioSpeedVeryHigh);
    mjs_return(mjs, MJS_UNDEFINED);
}

/**
 * @brief Writes a logic value to a GPIO pin 
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let led = gpio.get("pc3");
 * led.init({ direction: "out", outMode: "push_pull" });
 * led.write(true);
 * ```
 */
static void js_gpio_write(struct mjs* mjs) {
    bool level;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_BOOL(&level));
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    furi_hal_gpio_write(manager_data->pin, level);
    mjs_return(mjs, MJS_UNDEFINED);
}

/**
 * @brief Reads a logic value from a GPIO pin
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let button = gpio.get("pc1");
 * button.init({ direction: "in" });
 * if(button.read())
 *     print("hi button!!!!!");
 * ```
 */
static void js_gpio_read(struct mjs* mjs) {
    // get level
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    bool value = furi_hal_gpio_read(manager_data->pin);
    mjs_return(mjs, mjs_mk_boolean(mjs, value));
}

/**
 * @brief Returns a event loop contract that can be used to listen to interrupts
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let button = gpio.get("pc1");
 * let event_loop = require("event_loop");
 * button.init({ direction: "in", pull: "up", inMode: "interrupt", edge: "falling" });
 * event_loop.subscribe(button.interrupt(), function (_) { print("Hi!"); });
 * event_loop.run();
 * ```
 */
static void js_gpio_interrupt(struct mjs* mjs) {
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);

    // interrupt handling
    if(!manager_data->had_interrupt) {
        furi_hal_gpio_add_int_callback(
            manager_data->pin, js_gpio_int_cb, manager_data->interrupt_semaphore);
        furi_hal_gpio_enable_int_callback(manager_data->pin);
        manager_data->had_interrupt = true;
    }

    // make contract
    JsEventLoopContract* contract = malloc(sizeof(JsEventLoopContract));
    *contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object_type = JsEventLoopObjectTypeSemaphore,
        .object = manager_data->interrupt_semaphore,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
            },
    };
    manager_data->interrupt_contract = contract;
    mjs_return(mjs, mjs_mk_foreign(mjs, contract));
}

/**
 * @brief Reads a voltage from a GPIO pin in analog mode
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let pot = gpio.get("pc0");
 * pot.init({ direction: "in", inMode: "analog" });
 * print("voltage:" pot.read_analog(), "mV");
 * ```
 */
static void js_gpio_read_analog(struct mjs* mjs) {
    // get mV (ADC is configured for 12 bits and 2048 mV max)
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    uint16_t millivolts =
        furi_hal_adc_read(manager_data->adc_handle, manager_data->adc_channel) / 2;
    mjs_return(mjs, mjs_mk_number(mjs, (double)millivolts));
}

/**
 * @brief Returns an object that manages a specified pin.
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let led = gpio.get("pc3");
 * ```
 */
static void js_gpio_get(struct mjs* mjs) {
    mjs_val_t name_arg;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_ANY(&name_arg));
    const char* name_string = mjs_get_string(mjs, &name_arg, NULL);
    const GpioPinRecord* pin_record = NULL;

    // parse input argument to a pin pointer
    if(name_string) {
        pin_record = furi_hal_resources_pin_by_name(name_string);
    } else if(mjs_is_number(name_arg)) {
        int name_int = mjs_get_int(mjs, name_arg);
        pin_record = furi_hal_resources_pin_by_number(name_int);
    } else {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "Must be either a string or a number");
    }

    if(!pin_record) JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "Pin not found on device");
    if(pin_record->debug)
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "Pin is used for debugging");

    // return pin manager object
    JsGpioInst* module = JS_GET_CONTEXT(mjs);
    mjs_val_t manager = mjs_mk_object(mjs);
    JsGpioPinInst* manager_data = malloc(sizeof(JsGpioPinInst));
    manager_data->pin = pin_record->pin;
    manager_data->interrupt_semaphore = furi_semaphore_alloc(UINT32_MAX, 0);
    manager_data->adc_handle = module->adc_handle;
    manager_data->adc_channel = pin_record->channel;
    mjs_own(mjs, &manager);
    mjs_set(mjs, manager, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, manager_data));
    mjs_set(mjs, manager, "init", ~0, MJS_MK_FN(js_gpio_init));
    mjs_set(mjs, manager, "write", ~0, MJS_MK_FN(js_gpio_write));
    mjs_set(mjs, manager, "read", ~0, MJS_MK_FN(js_gpio_read));
    mjs_set(mjs, manager, "read_analog", ~0, MJS_MK_FN(js_gpio_read_analog));
    mjs_set(mjs, manager, "interrupt", ~0, MJS_MK_FN(js_gpio_interrupt));
    mjs_return(mjs, manager);

    // remember pin
    ManagedPinsArray_push_back(module->managed_pins, manager_data);
}

static void* js_gpio_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    JsEventLoop* js_loop = js_module_get(modules, "event_loop");
    if(M_UNLIKELY(!js_loop)) return NULL;
    FuriEventLoop* loop = js_event_loop_get_loop(js_loop);

    JsGpioInst* module = malloc(sizeof(JsGpioInst));
    ManagedPinsArray_init(module->managed_pins);
    module->adc_handle = furi_hal_adc_acquire();
    module->loop = loop;
    furi_hal_adc_configure(module->adc_handle);

    mjs_val_t gpio_obj = mjs_mk_object(mjs);
    mjs_set(mjs, gpio_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, module));
    mjs_set(mjs, gpio_obj, "get", ~0, MJS_MK_FN(js_gpio_get));
    *object = gpio_obj;

    return (void*)module;
}

static void js_gpio_destroy(void* inst) {
    furi_assert(inst);
    JsGpioInst* module = (JsGpioInst*)inst;

    // reset pins
    ManagedPinsArray_it_t iterator;
    for(ManagedPinsArray_it(iterator, module->managed_pins); !ManagedPinsArray_end_p(iterator);
        ManagedPinsArray_next(iterator)) {
        JsGpioPinInst* manager_data = *ManagedPinsArray_cref(iterator);
        if(manager_data->had_interrupt) {
            furi_hal_gpio_disable_int_callback(manager_data->pin);
            furi_hal_gpio_remove_int_callback(manager_data->pin);
        }
        furi_hal_gpio_init(manager_data->pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_event_loop_maybe_unsubscribe(module->loop, manager_data->interrupt_semaphore);
        furi_semaphore_free(manager_data->interrupt_semaphore);
        free(manager_data->interrupt_contract);
        free(manager_data);
    }

    // free buffers
    furi_hal_adc_release(module->adc_handle);
    ManagedPinsArray_clear(module->managed_pins);
    free(module);
}

static const JsModuleDescriptor js_gpio_desc = {
    "gpio",
    js_gpio_create,
    js_gpio_destroy,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_gpio_desc,
};

const FlipperAppPluginDescriptor* js_gpio_ep(void) {
    return &plugin_descriptor;
}
