/**
 * @file example_adc.c
 * @brief ADC example.
 */
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>

const uint8_t font[] =
    "`\2\3\2\3\4\1\2\4\5\11\0\376\6\376\7\377\1M\2\263\3\370 \6\315\364\371\6!\12\315"
    "\364\201\260\35\312Q\0\42\11\315tJI\316\13\0#\14\315\264\223dP*\203R'\1$\15\315\264"
    "\262A\311\266D\251l\71\0%\15\315\264\7%\61)J\42\345 \0&\14\315\264\263$\13\223\266$"
    "\7\1'\10\315\364\201\60\347\10(\10\315\364\32[\313\0)\11\315\64\322b[\35\2*\12\315\264\263"
    "(\222j\71\15+\11\315\364I\331\226\23\1,\10\315\364\271\205Y\10-\10\315\364\31t\26\0.\10"
    "\315\364\71\346(\0/\14\315\364\221\60\13\263\60\13C\0\60\13\315\264\245Jb)E:\12\61\12\315"
    "\364\201Ll\333A\0\62\12\315\264\245bV\33r\20\63\13\315\264\245Z\232D\221\216\2\64\14\315\364"
    "\201LJ\242!\313v\20\65\14\315t\207$\134\223(\322Q\0\66\13\315\264\245p\252D\221\216\2\67"
    "\12\315t\207\60+\326a\0\70\13\315\264\245\222T\211\42\35\5\71\13\315\264\245J\24\215\221\216\2:"
    "\11\315\364i\71!G\1;\12\315\364I\71!\314B\0<\11\315\364\341\254Z\7\1=\12\315\364)"
    "C<\344$\0>\11\315\364\301\264V\207\1\77\12\315\264\245Z\35\312a\0@\14\315\264\245J\242$"
    "J\272\203\0A\15\315\264\245J\224\14I\224D\71\10B\13\315t\247\312T\211\222\35\5C\12\315\264"
    "\245JX\212t\24D\15\315t\247J\224DI\224\354(\0E\14\315t\207$\234\302p\310A\0F"
    "\12\315t\207$\234\302:\1G\14\315\264\245J\230(Q\244\243\0H\17\315t\243$J\206$J\242"
    "$\312A\0I\11\315\264\267\260m\7\1J\12\315\364\221\260%\212t\24K\14\315t\243\244\244iI"
    "T\7\1L\11\315t\303\216C\16\2M\17\315t\243dH\206$J\242$\312A\0N\16\315t\243"
    "D\251(Q\22%Q\16\2O\15\315\264\245J\224DI\24\351(\0P\12\315t\247J\224LaN"
    "Q\15\315\264\245J\224DI\42\251\61\0R\14\315t\247J\224L\225(\7\1S\13\315\264\245\222\232"
    "D\221\216\2T\10\315\264\267\260;\12U\16\315t\243$J\242$J\242HG\1V\15\315t\243$"
    "J\242$Jj\71\14W\17\315t\243$J\242dH\206$\312A\0X\15\315t\243$\212\64\251\22"
    "\345 \0Y\13\315t\243$Jja\35\6Z\12\315t\207\60k\34r\20[\10\315\264\264\260G\31"
    "\134\12\315\264\303\64L\303\64\14]\10\315t\304\276\351\0^\11\315\364\201,\311\271\1_\7\315\364y"
    "\35\4`\10\315t\322\234'\0a\14\315\364IK\224$R\222\203\0b\13\315t\303p\252D\311\216"
    "\2c\12\315\364IR%\335A\0d\14\315\364\221\60Z\242$\212v\20e\12\315\364I\322\220\244;"
    "\10f\12\315\364\221,\333\302:\12g\14\315\364IK\224D\321\30I\0h\14\315t\303p\252DI"
    "\224\203\0i\12\315\364\201\34\21k;\10j\12\315\364\201\34\21\273e\0k\13\315t\303J\244%Q"
    "\35\4l\10\315\264\305n;\10m\14\315\364)CRQ\22\245\216\1n\13\315\364)%\245\224D\71"
    "\10o\12\315\364IR%\212t\24p\13\315\364)S%J\246\60\4q\13\315\364IK\224D\321X"
    "\1r\11\315\364)%\245\230\23s\12\315\364I\313\232\354(\0t\13\315\364\201\60\333\302\64\7\1u"
    "\15\315\364)Q\22%\211\224\344 \0v\13\315\364)Q\22%\265\34\6w\13\315\364)\25%Q\272"
    "\203\0x\12\315\364)Q\244Iu\20y\15\315\364)Q\22%Q\64F\22\0z\12\315\364)CV"
    "\33r\20{\12\315\364\212\265\64\254&\0|\7\315\264\302~\7}\12\315t\322\260\232\205\265\14~\11"
    "\315\364II;\13\0\177\6\315\364\371\6\0\0\0\4\377\377\0";

#define FONT_HEIGHT (8u)

typedef float (*ValueConverter)(FuriHalAdcHandle* handle, uint16_t value);

typedef struct {
    const GpioPinRecord* pin;
    float value;
    ValueConverter converter;
    const char* suffix;
} DataItem;

typedef struct {
    size_t count;
    DataItem* items;
} Data;

const GpioPinRecord item_vref = {.name = "VREF", .channel = FuriHalAdcChannelVREFINT};
const GpioPinRecord item_temp = {.name = "TEMP", .channel = FuriHalAdcChannelTEMPSENSOR};
const GpioPinRecord item_vbat = {.name = "VBAT", .channel = FuriHalAdcChannelVBAT};

static void app_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    Data* data = ctx;

    canvas_set_custom_u8g2_font(canvas, font);
    char buffer[64];
    int32_t x = 0, y = FONT_HEIGHT;
    for(size_t i = 0; i < data->count; i++) {
        if(i == canvas_height(canvas) / FONT_HEIGHT) {
            x = 64;
            y = FONT_HEIGHT;
        }

        snprintf(
            buffer,
            sizeof(buffer),
            "%4s: %4.0f%s\n",
            data->items[i].pin->name,
            (double)data->items[i].value,
            data->items[i].suffix);
        canvas_draw_str(canvas, x, y, buffer);
        y += FONT_HEIGHT;
    }
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t example_adc_main(void* p) {
    UNUSED(p);

    // Data
    Data data = {};
    for(size_t i = 0; i < gpio_pins_count; i++) {
        if(gpio_pins[i].channel != FuriHalAdcChannelNone) {
            data.count++;
        }
    }
    data.count += 3; // Special channels
    data.items = malloc(data.count * sizeof(DataItem));
    size_t item_pos = 0;
    for(size_t i = 0; i < gpio_pins_count; i++) {
        if(gpio_pins[i].channel != FuriHalAdcChannelNone) {
            furi_hal_gpio_init(gpio_pins[i].pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
            data.items[item_pos].pin = &gpio_pins[i];
            data.items[item_pos].converter = furi_hal_adc_convert_to_voltage;
            data.items[item_pos].suffix = "mV";
            item_pos++;
        }
    }
    data.items[item_pos].pin = &item_vref;
    data.items[item_pos].converter = furi_hal_adc_convert_vref;
    data.items[item_pos].suffix = "mV";
    item_pos++;
    data.items[item_pos].pin = &item_temp;
    data.items[item_pos].converter = furi_hal_adc_convert_temp;
    data.items[item_pos].suffix = "C";
    item_pos++;
    data.items[item_pos].pin = &item_vbat;
    data.items[item_pos].converter = furi_hal_adc_convert_vbat;
    data.items[item_pos].suffix = "mV";
    item_pos++;
    furi_assert(item_pos == data.count);

    // Alloc message queue
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, &data);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Initialize ADC
    FuriHalAdcHandle* adc_handle = furi_hal_adc_acquire();
    furi_hal_adc_configure(adc_handle);

    // Process events
    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypePress && event.key == InputKeyBack) {
                running = false;
            }
        } else {
            for(size_t i = 0; i < data.count; i++) {
                data.items[i].value = data.items[i].converter(
                    adc_handle, furi_hal_adc_read(adc_handle, data.items[i].pin->channel));
            }
            view_port_update(view_port);
        }
    }

    furi_hal_adc_release(adc_handle);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
    free(data.items);

    return 0;
}
