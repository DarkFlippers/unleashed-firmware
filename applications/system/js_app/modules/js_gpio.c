#include "../js_modules.h" // IWYU pragma: keep
#include "./js_event_loop/js_event_loop.h"
#include <furi_hal_gpio.h>
#include <furi_hal_pwm.h>
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
    FuriHalPwmOutputId pwm_output;
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
    // direction variants
    typedef enum {
        JsGpioDirectionIn,
        JsGpioDirectionOut,
    } JsGpioDirection;
    static const JsValueEnumVariant js_gpio_direction_variants[] = {
        {"in", JsGpioDirectionIn},
        {"out", JsGpioDirectionOut},
    };
    static const JsValueDeclaration js_gpio_direction =
        JS_VALUE_ENUM(JsGpioDirection, js_gpio_direction_variants);

    // inMode variants
    typedef enum {
        JsGpioInModeAnalog = (0 << 0),
        JsGpioInModePlainDigital = (1 << 0),
        JsGpioInModeInterrupt = (2 << 0),
        JsGpioInModeEvent = (3 << 0),
    } JsGpioInMode;
    static const JsValueEnumVariant js_gpio_in_mode_variants[] = {
        {"analog", JsGpioInModeAnalog},
        {"plain_digital", JsGpioInModePlainDigital},
        {"interrupt", JsGpioInModeInterrupt},
        {"event", JsGpioInModeEvent},
    };
    static const JsValueDeclaration js_gpio_in_mode =
        JS_VALUE_ENUM_W_DEFAULT(JsGpioInMode, js_gpio_in_mode_variants, JsGpioInModePlainDigital);

    // outMode variants
    typedef enum {
        JsGpioOutModePushPull,
        JsGpioOutModeOpenDrain,
    } JsGpioOutMode;
    static const JsValueEnumVariant js_gpio_out_mode_variants[] = {
        {"push_pull", JsGpioOutModePushPull},
        {"open_drain", JsGpioOutModeOpenDrain},
    };
    static const JsValueDeclaration js_gpio_out_mode =
        JS_VALUE_ENUM_W_DEFAULT(JsGpioOutMode, js_gpio_out_mode_variants, JsGpioOutModeOpenDrain);

    // edge variants
    typedef enum {
        JsGpioEdgeRising = (0 << 2),
        JsGpioEdgeFalling = (1 << 2),
        JsGpioEdgeBoth = (2 << 2),
    } JsGpioEdge;
    static const JsValueEnumVariant js_gpio_edge_variants[] = {
        {"rising", JsGpioEdgeRising},
        {"falling", JsGpioEdgeFalling},
        {"both", JsGpioEdgeBoth},
    };
    static const JsValueDeclaration js_gpio_edge =
        JS_VALUE_ENUM_W_DEFAULT(JsGpioEdge, js_gpio_edge_variants, JsGpioEdgeRising);

    // pull variants
    static const JsValueEnumVariant js_gpio_pull_variants[] = {
        {"up", GpioPullUp},
        {"down", GpioPullDown},
    };
    static const JsValueDeclaration js_gpio_pull =
        JS_VALUE_ENUM_W_DEFAULT(GpioPull, js_gpio_pull_variants, GpioPullNo);

    // complete mode object
    static const JsValueObjectField js_gpio_mode_object_fields[] = {
        {"direction", &js_gpio_direction},
        {"inMode", &js_gpio_in_mode},
        {"outMode", &js_gpio_out_mode},
        {"edge", &js_gpio_edge},
        {"pull", &js_gpio_pull},
    };

    // function args
    static const JsValueDeclaration js_gpio_init_arg_list[] = {
        JS_VALUE_OBJECT_W_DEFAULTS(js_gpio_mode_object_fields),
    };
    static const JsValueArguments js_gpio_init_args = JS_VALUE_ARGS(js_gpio_init_arg_list);

    JsGpioDirection direction;
    JsGpioInMode in_mode;
    JsGpioOutMode out_mode;
    JsGpioEdge edge;
    GpioPull pull;
    JS_VALUE_PARSE_ARGS_OR_RETURN(
        mjs, &js_gpio_init_args, &direction, &in_mode, &out_mode, &edge, &pull);

    GpioMode mode;
    if(direction == JsGpioDirectionOut) {
        static const GpioMode js_gpio_out_mode_lut[] = {
            [JsGpioOutModePushPull] = GpioModeOutputPushPull,
            [JsGpioOutModeOpenDrain] = GpioModeOutputOpenDrain,
        };
        mode = js_gpio_out_mode_lut[out_mode];
    } else {
        static const GpioMode js_gpio_in_mode_lut[] = {
            [JsGpioInModeAnalog] = GpioModeAnalog,
            [JsGpioInModePlainDigital] = GpioModeInput,
            [JsGpioInModeInterrupt | JsGpioEdgeRising] = GpioModeInterruptRise,
            [JsGpioInModeInterrupt | JsGpioEdgeFalling] = GpioModeInterruptFall,
            [JsGpioInModeInterrupt | JsGpioEdgeBoth] = GpioModeInterruptRiseFall,
            [JsGpioInModeEvent | JsGpioEdgeRising] = GpioModeEventRise,
            [JsGpioInModeEvent | JsGpioEdgeFalling] = GpioModeEventFall,
            [JsGpioInModeEvent | JsGpioEdgeBoth] = GpioModeEventRiseFall,
        };
        mode = js_gpio_in_mode_lut[in_mode | edge];
    }

    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    furi_hal_gpio_init(manager_data->pin, mode, pull, GpioSpeedVeryHigh);
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
    static const JsValueDeclaration js_gpio_write_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeBool),
    };
    static const JsValueArguments js_gpio_write_args = JS_VALUE_ARGS(js_gpio_write_arg_list);
    bool level;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gpio_write_args, &level);

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
 * print("voltage:" pot.readAnalog(), "mV");
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
 * @brief Determines whether this pin supports PWM
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * assert_eq(true, gpio.get("pa4").isPwmSupported());
 * assert_eq(false, gpio.get("pa5").isPwmSupported());
 * ```
 */
static void js_gpio_is_pwm_supported(struct mjs* mjs) {
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, manager_data->pwm_output != FuriHalPwmOutputIdNone));
}

/**
 * @brief Sets PWM parameters and starts the PWM
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let pa4 = gpio.get("pa4");
 * pa4.pwmWrite(10000, 50);
 * ```
 */
static void js_gpio_pwm_write(struct mjs* mjs) {
    static const JsValueDeclaration js_gpio_pwm_write_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeInt32),
        JS_VALUE_SIMPLE(JsValueTypeInt32),
    };
    static const JsValueArguments js_gpio_pwm_write_args =
        JS_VALUE_ARGS(js_gpio_pwm_write_arg_list);
    int32_t frequency, duty;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gpio_pwm_write_args, &frequency, &duty);

    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    if(manager_data->pwm_output == FuriHalPwmOutputIdNone) {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "PWM is not supported on this pin");
    }

    if(furi_hal_pwm_is_running(manager_data->pwm_output)) {
        furi_hal_pwm_set_params(manager_data->pwm_output, frequency, duty);
    } else {
        furi_hal_pwm_start(manager_data->pwm_output, frequency, duty);
    }
}

/**
 * @brief Determines whether PWM is running
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * assert_eq(false, gpio.get("pa4").isPwmRunning());
 * ```
 */
static void js_gpio_is_pwm_running(struct mjs* mjs) {
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    if(manager_data->pwm_output == FuriHalPwmOutputIdNone) {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "PWM is not supported on this pin");
    }

    mjs_return(mjs, mjs_mk_boolean(mjs, furi_hal_pwm_is_running(manager_data->pwm_output)));
}

/**
 * @brief Stops PWM
 * 
 * Example usage:
 * 
 * ```js
 * let gpio = require("gpio");
 * let pa4 = gpio.get("pa4");
 * pa4.pwmWrite(10000, 50);
 * pa4.pwmStop();
 * ```
 */
static void js_gpio_pwm_stop(struct mjs* mjs) {
    JsGpioPinInst* manager_data = JS_GET_CONTEXT(mjs);
    if(manager_data->pwm_output == FuriHalPwmOutputIdNone) {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "PWM is not supported on this pin");
    }

    furi_hal_pwm_stop(manager_data->pwm_output);
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
    static const JsValueDeclaration js_gpio_get_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeAny),
    };
    static const JsValueArguments js_gpio_get_args = JS_VALUE_ARGS(js_gpio_get_arg_list);
    mjs_val_t name_arg;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gpio_get_args, &name_arg);

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
    manager_data->pwm_output = pin_record->pwm_output;
    JS_ASSIGN_MULTI(mjs, manager) {
        JS_FIELD(INST_PROP_NAME, mjs_mk_foreign(mjs, manager_data));
        JS_FIELD("init", MJS_MK_FN(js_gpio_init));
        JS_FIELD("write", MJS_MK_FN(js_gpio_write));
        JS_FIELD("read", MJS_MK_FN(js_gpio_read));
        JS_FIELD("readAnalog", MJS_MK_FN(js_gpio_read_analog));
        JS_FIELD("interrupt", MJS_MK_FN(js_gpio_interrupt));
        JS_FIELD("isPwmSupported", MJS_MK_FN(js_gpio_is_pwm_supported));
        JS_FIELD("pwmWrite", MJS_MK_FN(js_gpio_pwm_write));
        JS_FIELD("isPwmRunning", MJS_MK_FN(js_gpio_is_pwm_running));
        JS_FIELD("pwmStop", MJS_MK_FN(js_gpio_pwm_stop));
    }
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
