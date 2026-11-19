#include "sim800_howto_view.h"

#include <gui/elements.h>
#include <furi.h>

#define SIM800_HOWTO_PAGES 5

struct Sim800HowtoView {
    View* view;
};

typedef struct {
    uint8_t page;
    uint8_t tx_pin;
    uint8_t rx_pin;
} Sim800HowtoViewModel;

static const char* const sim800_howto_titles[SIM800_HOWTO_PAGES] = {
    "Wiring",
    "5V dropper",
    "TX divider",
    "What it does",
    "Power & baud",
};

static const char* const sim800_howto_text[SIM800_HOWTO_PAGES] = {
    "",
    "",
    "",
    "Drives a SIM800 GSM module over the GPIO UART.\n"
    "Basic dials and texts by hand. Saved keeps numbers in /ext/phonebook and "
    "opens them from the Archive too. Advanced reaches the module itself: network, "
    "inbox, USSD, DTMF, audio, power and a raw AT terminal.",
    "SIM800C and SIM800L are the same modem here. A board with a regulator takes "
    "pin 1 (5 V, 1.2 A) directly. A bare module wants 3.4-4.4 V, so pin 1 kills it "
    "and 3V3 is too low to start it: drop pin 1 with the two diodes on page 2. The "
    "2 A figure is a 577us burst at 12.5% duty, not a load - the average is ~250 mA "
    "and the cap covers the burst. Plug the module in with the Flipper powered off.\n"
    "Set the same baud rate in Settings that the module runs at. 115200 is the "
    "factory default, but boards ship re-set as low as 2400 and keep whatever "
    "AT+IPR last gave them. If nothing answers, run Advanced > Detect baud.",
};

/** Wire with an arrowhead on the end the signal travels to. */
static void sim800_howto_wire(Canvas* canvas, int32_t y, bool to_right, bool to_left) {
    const int32_t x1 = 48;
    const int32_t x2 = 84;

    canvas_draw_line(canvas, x1, y, x2, y);
    if(to_right) {
        canvas_draw_line(canvas, x2 - 3, y - 2, x2, y);
        canvas_draw_line(canvas, x2 - 3, y + 2, x2, y);
    }
    if(to_left) {
        canvas_draw_line(canvas, x1 + 3, y - 2, x1, y);
        canvas_draw_line(canvas, x1 + 3, y + 2, x1, y);
    }
}

static void sim800_howto_draw_wiring(Canvas* canvas, const Sim800HowtoViewModel* model) {
    char label[12];

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 22, "Flipper");
    canvas_draw_str_aligned(canvas, 126, 22, AlignRight, AlignBottom, "SIM800L");

    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str(canvas, 2, 33, "11 GND");
    canvas_draw_str_aligned(canvas, 126, 33, AlignRight, AlignBottom, "GND");
    sim800_howto_wire(canvas, 30, false, false);

    snprintf(label, sizeof(label), "%u TX", model->tx_pin);
    canvas_draw_str(canvas, 2, 41, label);
    canvas_draw_str_aligned(canvas, 126, 41, AlignRight, AlignBottom, "RXD");
    sim800_howto_wire(canvas, 38, true, false);

    snprintf(label, sizeof(label), "%u RX", model->rx_pin);
    canvas_draw_str(canvas, 2, 49, label);
    canvas_draw_str_aligned(canvas, 126, 49, AlignRight, AlignBottom, "TXD");
    sim800_howto_wire(canvas, 46, false, true);

    // The one wire that must not come from the Flipper
    canvas_draw_str_aligned(canvas, 64, 61, AlignCenter, AlignBottom, "VCC: 3.4-4.4V, see p2");
}

/** Diode pointing right: triangle into a bar, at the left edge of a 12px cell. */
static void sim800_howto_diode(Canvas* canvas, int32_t x, int32_t y) {
    canvas_draw_line(canvas, x, y - 3, x, y + 3);
    canvas_draw_line(canvas, x, y - 3, x + 5, y);
    canvas_draw_line(canvas, x, y + 3, x + 5, y);
    canvas_draw_line(canvas, x + 5, y - 3, x + 5, y + 3);
}

/** Dropping pin 1 to the modem's VBAT with two rectifiers and a bulk cap. */
static void sim800_howto_draw_dropper(Canvas* canvas) {
    const int32_t top = 24; // the 5V chain
    const int32_t bot = 46; // the ground return
    const int32_t node = 80; // where the cap and VCC meet the chain

    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str(canvas, 2, top + 3, "Pin1");
    canvas_draw_line(canvas, 26, top, 30, top);
    sim800_howto_diode(canvas, 30, top);
    canvas_draw_line(canvas, 35, top, 41, top);
    sim800_howto_diode(canvas, 41, top);
    canvas_draw_line(canvas, 46, top, node, top);
    canvas_draw_str(canvas, node + 4, top + 3, "VCC");

    canvas_draw_str(canvas, 2, bot + 3, "Pin11");
    canvas_draw_line(canvas, 28, bot, node, bot);
    canvas_draw_str(canvas, node + 4, bot + 3, "GND");

    // Cap across the two rails, plates breaking the vertical run
    canvas_draw_line(canvas, node, top, node, 32);
    canvas_draw_line(canvas, 74, 32, 86, 32);
    canvas_draw_line(canvas, 74, 36, 86, 36);
    canvas_draw_line(canvas, node, 36, node, bot);
    canvas_draw_str(canvas, 88, 34, "+");

    // Two lines: one long enough to say all of it would not fit the width
    canvas_draw_str_aligned(canvas, 64, 55, AlignCenter, AlignBottom, "2x 1N5401 - 3A silicon");
    canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, "4700uF low-ESR 16V");
}

/** Dropping the Flipper's 3.3V TX into the modem's 2.8V logic domain. */
static void sim800_howto_draw_divider(Canvas* canvas) {
    const int32_t rail = 24; // the TX run across the top
    const int32_t node = 78; // where the shunt taps the divider

    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str(canvas, 2, rail + 3, "13 TX");
    canvas_draw_line(canvas, 26, rail, 38, rail);

    // Series leg, with the value inside the body so it cannot collide with the rail
    canvas_draw_frame(canvas, 38, rail - 5, 22, 10);
    canvas_draw_str_aligned(canvas, 49, rail + 3, AlignCenter, AlignBottom, "1k");

    canvas_draw_line(canvas, 60, rail, 96, rail);
    canvas_draw_disc(canvas, node, rail, 1);
    canvas_draw_str(canvas, 98, rail + 3, "RXD");

    // Shunt leg down to ground
    canvas_draw_line(canvas, node, rail, node, 34);
    canvas_draw_frame(canvas, node - 5, 34, 10, 16);
    canvas_draw_str(canvas, node + 8, 44, "2.2k");
    canvas_draw_line(canvas, node, 50, node, 55);

    canvas_draw_line(canvas, node - 7, 55, node + 7, 55);
    canvas_draw_line(canvas, node - 4, 58, node + 4, 58);
    canvas_draw_line(canvas, node - 1, 61, node + 1, 61);

    canvas_draw_str(canvas, 2, 38, "3.3V -> 2.3V");
    canvas_draw_str(canvas, 2, 46, "TX side only");
    canvas_draw_str(canvas, 2, 54, "Direct works");
}

static void sim800_howto_view_draw(Canvas* canvas, void* ctx) {
    Sim800HowtoViewModel* model = ctx;
    char counter[16];

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, sim800_howto_titles[model->page]);

    // Paging lives in the title bar: framed buttons would eat the bottom of every page
    snprintf(
        counter,
        sizeof(counter),
        "%s%u/%u%s",
        (model->page > 0) ? "< " : "",
        model->page + 1,
        SIM800_HOWTO_PAGES,
        (model->page + 1 < SIM800_HOWTO_PAGES) ? " >" : "");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, counter);
    canvas_draw_line(canvas, 0, 12, 127, 12);

    if(model->page == 0) {
        sim800_howto_draw_wiring(canvas, model);
    } else if(model->page == 1) {
        sim800_howto_draw_dropper(canvas);
    } else if(model->page == 2) {
        sim800_howto_draw_divider(canvas);
    } else {
        elements_text_box(
            canvas, 2, 15, 124, 48, AlignLeft, AlignTop, sim800_howto_text[model->page], false);
    }
}

static bool sim800_howto_view_input(InputEvent* event, void* ctx) {
    Sim800HowtoView* instance = ctx;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;
    if(event->key != InputKeyLeft && event->key != InputKeyRight) return false;

    with_view_model(
        instance->view,
        Sim800HowtoViewModel * model,
        {
            if(event->key == InputKeyLeft && model->page > 0) {
                model->page--;
            } else if(event->key == InputKeyRight && model->page + 1 < SIM800_HOWTO_PAGES) {
                model->page++;
            }
        },
        true);

    return true;
}

Sim800HowtoView* sim800_howto_view_alloc(void) {
    Sim800HowtoView* instance = malloc(sizeof(Sim800HowtoView));

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(Sim800HowtoViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, sim800_howto_view_draw);
    view_set_input_callback(instance->view, sim800_howto_view_input);

    with_view_model(
        instance->view,
        Sim800HowtoViewModel * model,
        {
            model->page = 0;
            model->tx_pin = 13;
            model->rx_pin = 14;
        },
        false);

    return instance;
}

void sim800_howto_view_free(Sim800HowtoView* instance) {
    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* sim800_howto_view_get_view(Sim800HowtoView* instance) {
    furi_assert(instance);
    return instance->view;
}

void sim800_howto_view_set_pins(Sim800HowtoView* instance, uint8_t tx_pin, uint8_t rx_pin) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800HowtoViewModel * model,
        {
            model->tx_pin = tx_pin;
            model->rx_pin = rx_pin;
        },
        true);
}

void sim800_howto_view_reset(Sim800HowtoView* instance) {
    furi_assert(instance);
    with_view_model(instance->view, Sim800HowtoViewModel * model, { model->page = 0; }, true);
}
