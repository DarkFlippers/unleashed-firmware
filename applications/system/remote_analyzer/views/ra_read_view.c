#include "ra_read_view.h"

#include <furi.h>
#include "ra_icons.h"
#include <input/input.h>

/** Body lines that fit between the RSSI meter and the button row. */
#define RA_READ_VIEW_LINES    4
/** A display line is bounded by the width, not by the field: this only has to outlast it. */
#define RA_READ_VIEW_LINE_MAX 64
/** Width a body line has, at x = 2 with the same margin left on the right. */
#define RA_READ_VIEW_TEXT_W   124

/** Header baseline, the badge beside it, the separator under it, and the meter below that. */
#define RA_READ_VIEW_HEADER_Y 8
#define RA_READ_VIEW_BADGE_Y  1
#define RA_READ_VIEW_SEP_Y    9

/* elements_button_* is 12 rows tall. 11 still clears this font - haxrcorp4089 reaches 8 rows
 * above the baseline and drops 2 below it, so a 3 row inset leaves a row of air at the top and
 * the descender inside - and the row it gives back is what lets line 0 be FontPrimary. */
#define RA_READ_VIEW_BTN_H   11
#define RA_READ_VIEW_BTN_PAD 3

/**
 * Baseline per body line rather than a fixed step: line 0 is FontPrimary, whose 8 rows of
 * ascent and 2 of descender need more room than the FontSecondary lines under it. Line 0 clears
 * the meter above and line 1 below; the rest sit at FontSecondary's minimum leading of 9.
 */
static const uint8_t ra_read_view_line_y[RA_READ_VIEW_LINES] = {23, 33, 42, 50};

struct RaReadView {
    View* view;
    RaReadViewCallback callback;
    void* context;
};

typedef struct {
    FuriString* text; // empty while nothing has been decoded
    FuriString* radio; // "433.92 AM650"
    uint32_t count; // signals seen since the last reset
    uint8_t scroll; // first body line drawn
    uint8_t lines; // display lines text wraps to, measured at draw, so input can clamp scroll
    uint8_t kind; // RaReadViewKind of the signal on screen
    float rssi; // dBm, for the meter under the separator
    FuriString* status_line; // empty unless the sweep has something to say
    FuriString* status_detail;
} RaReadViewModel;

/* Same span the SubGhz app's meter covers, stretched over the screen instead of its 50px. */
#define RA_READ_VIEW_RSSI_MIN (-90.0f)
#define RA_READ_VIEW_RSSI_MAX (-40.0f)

#define RA_READ_VIEW_RSSI_X 2
#define RA_READ_VIEW_RSSI_Y 11
/** Edge to edge, less the same 2px margin the separator above it does not take. */
#define RA_READ_VIEW_RSSI_W 124

/** elements_button_left/right, redrawn at RA_READ_VIEW_BTN_H with the 3x5 arrow. */
static void ra_read_view_draw_button(Canvas* canvas, const char* str, bool left) {
    const Icon* icon = left ? &I_ButtonLeftSmall_3x5 : &I_ButtonRightSmall_3x5;
    const uint8_t icon_w = (uint8_t)icon_get_width(icon);
    const uint8_t icon_h = (uint8_t)icon_get_height(icon);
    const uint8_t str_w = (uint8_t)canvas_string_width(canvas, str);
    const uint8_t w = RA_READ_VIEW_BTN_PAD * 2 + icon_w + RA_READ_VIEW_BTN_PAD + str_w;

    const uint8_t bottom = (uint8_t)canvas_height(canvas);
    const uint8_t top = bottom - RA_READ_VIEW_BTN_H;
    const uint8_t x = left ? 0 : (uint8_t)(canvas_width(canvas) - w);

    canvas_draw_box(canvas, x, top, w, RA_READ_VIEW_BTN_H);
    // The stock hint tapers away from the screen edge, so these keep that shape
    if(left) {
        canvas_draw_line(canvas, x + w, bottom, x + w, top);
        canvas_draw_line(canvas, x + w + 1, bottom, x + w + 1, top + 1);
    } else {
        canvas_draw_line(canvas, x - 1, bottom, x - 1, top);
        canvas_draw_line(canvas, x - 2, bottom, x - 2, top + 1);
    }

    const uint8_t icon_y = top + (RA_READ_VIEW_BTN_H - icon_h) / 2;
    canvas_invert_color(canvas);
    if(left) {
        canvas_draw_icon(canvas, x + RA_READ_VIEW_BTN_PAD, icon_y, icon);
        canvas_draw_str(
            canvas, x + RA_READ_VIEW_BTN_PAD * 2 + icon_w, bottom - RA_READ_VIEW_BTN_PAD, str);
    } else {
        canvas_draw_str(canvas, x + RA_READ_VIEW_BTN_PAD, bottom - RA_READ_VIEW_BTN_PAD, str);
        canvas_draw_icon(canvas, x + RA_READ_VIEW_BTN_PAD * 2 + str_w, icon_y, icon);
    }
    canvas_invert_color(canvas);
}

/** "More below", in the gap between the two hints, where no reading line reaches. */
static void ra_read_view_draw_more(Canvas* canvas) {
    canvas_draw_line(canvas, 61, 57, 67, 57);
    canvas_draw_line(canvas, 62, 58, 66, 58);
    canvas_draw_line(canvas, 63, 59, 65, 59);
    canvas_draw_dot(canvas, 64, 60);
}

static const Icon* ra_read_view_kind_icon(uint8_t kind) {
    switch(kind) {
    case RaReadViewKindStatic:
        return &I_Static_9x7;
    case RaReadViewKindDynamic:
        return &I_Dynamic_9x7;
    case RaReadViewKindUnknown:
        return &I_Quest_7x8;
    default:
        return NULL;
    }
}

/** Where the next display line comes from, as the text is laid out one line at a time. */
typedef struct {
    const char* p; /**< next byte to lay out */
    size_t logical; /**< which of the decoder's own lines p is inside */
} RaReadViewCursor;

/**
 * Step over the separator the previous line stopped on - CRLF, or either half of it - leaving
 * cursor->logical naming the decoder line the next display line comes from.
 *
 * Separate from taking the line because the caller has to know which logical line is next
 * before it can set the font, and the font is what decides where that line breaks.
 */
static void ra_read_view_step_break(RaReadViewCursor* cursor) {
    if((*cursor->p != '\r') && (*cursor->p != '\n')) return;
    while((*cursor->p == '\r') || (*cursor->p == '\n'))
        cursor->p++;
    cursor->logical++;
}

/**
 * Take the next display line off @p cursor into @p out.
 *
 * A decoder writes one field per line, and a long one - a 64 bit key, say - is wider than the
 * screen. Rather than running off the right edge it is carried onto the next line, at the last
 * space that fits, or mid-value when the field has no space in it to break at.
 *
 * The font must already be the one the line will be drawn in. Widths are accumulated a glyph
 * at a time, so the whole layout is one pass over the text rather than a measurement per
 * candidate break.
 */
static void
    ra_read_view_next_line(Canvas* canvas, RaReadViewCursor* cursor, char* out, size_t out_size) {
    const char* p = cursor->p;

    size_t avail = 0;
    while(p[avail] && (p[avail] != '\r') && (p[avail] != '\n'))
        avail++;

    size_t take = 0;
    size_t width = 0;
    size_t space = 0; // last space that still fits, 0 for none
    while((take < avail) && (take + 1 < out_size)) {
        const size_t glyph = canvas_glyph_width(canvas, (uint16_t)(uint8_t)p[take]);
        if((width + glyph) > RA_READ_VIEW_TEXT_W) break;
        if(p[take] == ' ') space = take;
        width += glyph;
        take++;
    }

    size_t skip = 0;
    if(take < avail) {
        if(space > 0) {
            // The space is where the line ends, so it is dropped rather than drawn
            take = space;
            skip = 1;
        } else if(take == 0) {
            take = 1; // a glyph wider than the whole line would otherwise never advance
        }
    }

    memcpy(out, p, take);
    out[take] = '\0';
    cursor->p = p + take + skip;
}

static uint8_t ra_read_view_rssi_fill(float rssi) {
    const float span = RA_READ_VIEW_RSSI_MAX - RA_READ_VIEW_RSSI_MIN;
    float part = (rssi - RA_READ_VIEW_RSSI_MIN) / span;
    if(part < 0.0f) part = 0.0f;
    if(part > 1.0f) part = 1.0f;
    return (uint8_t)(part * RA_READ_VIEW_RSSI_W + 0.5f);
}

/* The SubGhz app's texture: three rows with the middle one shifted right, and every fifth
 * pixel left out so the length stays readable at a glance. */
static void ra_read_view_draw_rssi(Canvas* canvas, float rssi) {
    const uint8_t fill = ra_read_view_rssi_fill(rssi);
    for(uint8_t i = 1; i < fill; i++) {
        if(i % 5) {
            canvas_draw_dot(canvas, RA_READ_VIEW_RSSI_X + i, RA_READ_VIEW_RSSI_Y);
            canvas_draw_dot(canvas, RA_READ_VIEW_RSSI_X + i + 1, RA_READ_VIEW_RSSI_Y + 1);
            canvas_draw_dot(canvas, RA_READ_VIEW_RSSI_X + i, RA_READ_VIEW_RSSI_Y + 2);
        }
    }
}

static void ra_read_view_draw(Canvas* canvas, RaReadViewModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    // Badge on the left and the counter on the right, so the radio text keeps the middle
    const Icon* badge = ra_read_view_kind_icon(model->kind);
    if(badge) canvas_draw_icon(canvas, 2, RA_READ_VIEW_BADGE_Y, badge);
    canvas_draw_str_aligned(
        canvas,
        64,
        RA_READ_VIEW_HEADER_Y,
        AlignCenter,
        AlignBottom,
        furi_string_get_cstr(model->radio));
    if(model->count > 1) {
        char buf[8];
        // The screen has room for three digits, and a press count past that reads the same
        snprintf(
            buf, sizeof(buf), "x%lu", model->count > 999 ? 999UL : (unsigned long)model->count);
        canvas_draw_str_aligned(canvas, 126, RA_READ_VIEW_HEADER_Y, AlignRight, AlignBottom, buf);
    }
    canvas_draw_line(canvas, 0, RA_READ_VIEW_SEP_Y, 127, RA_READ_VIEW_SEP_Y);
    ra_read_view_draw_rssi(canvas, model->rssi);

    bool more = false;
    if(!furi_string_empty(model->status_line)) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 64, 28, AlignCenter, AlignCenter, furi_string_get_cstr(model->status_line));
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas, 64, 42, AlignCenter, AlignCenter, furi_string_get_cstr(model->status_detail));
    } else if(furi_string_empty(model->text)) {
        canvas_draw_str_aligned(canvas, 64, 33, AlignCenter, AlignCenter, "Waiting for a signal");
    } else {
        /* Laid out here rather than when the signal arrives: where a line breaks depends on
         * the font, which only exists on a canvas. The pass walks every display line so the
         * total is known, and draws the ones inside the window. */
        RaReadViewCursor cursor = {.p = furi_string_get_cstr(model->text), .logical = 0};
        char line[RA_READ_VIEW_LINE_MAX];
        uint8_t index = 0;

        for(;;) {
            /* The break is stepped before the font is chosen: the font decides where the
             * line wraps, so it has to be the one the line about to be taken belongs to. */
            ra_read_view_step_break(&cursor);
            if(!*cursor.p) break;

            // The decoder's first line is the protocol name and bit count: the headline
            canvas_set_font(canvas, (cursor.logical == 0) ? FontPrimary : FontSecondary);
            ra_read_view_next_line(canvas, &cursor, line, sizeof(line));

            if((index >= model->scroll) && (index < model->scroll + RA_READ_VIEW_LINES)) {
                canvas_draw_str(canvas, 2, ra_read_view_line_y[index - model->scroll], line);
            }
            index++;
        }

        canvas_set_font(canvas, FontSecondary);
        model->lines = index;
        more = index > model->scroll + RA_READ_VIEW_LINES;
    }

    ra_read_view_draw_button(canvas, "Config", true);
    ra_read_view_draw_button(canvas, "Reset", false);
    if(more) ra_read_view_draw_more(canvas);
}

static bool ra_read_view_input(InputEvent* event, void* context) {
    furi_assert(context);
    RaReadView* instance = context;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    switch(event->key) {
    case InputKeyLeft:
        if(instance->callback) instance->callback(RaReadViewEventConfig, instance->context);
        return true;
    case InputKeyRight:
        if(instance->callback) instance->callback(RaReadViewEventReset, instance->context);
        return true;
    case InputKeyUp:
    case InputKeyDown: {
        const bool down = event->key == InputKeyDown;
        bool moved = false;
        with_view_model(
            instance->view,
            RaReadViewModel * model,
            {
                if(down) {
                    if(model->lines > model->scroll + RA_READ_VIEW_LINES) {
                        model->scroll++;
                        moved = true;
                    }
                } else if(model->scroll > 0) {
                    model->scroll--;
                    moved = true;
                }
            },
            moved);
        return true;
    }
    default:
        return false;
    }
}

RaReadView* ra_read_view_alloc(void) {
    RaReadView* instance = malloc(sizeof(RaReadView));

    instance->callback = NULL;
    instance->context = NULL;

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(RaReadViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)ra_read_view_draw);
    view_set_input_callback(instance->view, ra_read_view_input);

    with_view_model(
        instance->view,
        RaReadViewModel * model,
        {
            model->text = furi_string_alloc();
            model->radio = furi_string_alloc();
            model->status_line = furi_string_alloc();
            model->status_detail = furi_string_alloc();
            model->count = 0;
            model->scroll = 0;
            model->lines = 0;
            model->kind = RaReadViewKindNone;
            model->rssi = RA_READ_VIEW_RSSI_MIN;
        },
        false);

    return instance;
}

void ra_read_view_free(RaReadView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        RaReadViewModel * model,
        {
            furi_string_free(model->text);
            furi_string_free(model->radio);
            furi_string_free(model->status_line);
            furi_string_free(model->status_detail);
        },
        false);
    view_free(instance->view);
    free(instance);
}

View* ra_read_view_get_view(RaReadView* instance) {
    furi_assert(instance);
    return instance->view;
}

void ra_read_view_set_callback(RaReadView* instance, RaReadViewCallback callback, void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->context = context;
}

void ra_read_view_set_signal(RaReadView* instance, const char* text, RaReadViewKind kind) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RaReadViewModel * model,
        {
            furi_string_set_str(model->text, text);
            // Filled in by the next draw, which is where the wrapping is worked out
            model->lines = 0;
            model->scroll = 0;
            model->count++;
            model->kind = kind;
            furi_string_reset(model->status_line);
            furi_string_reset(model->status_detail);
        },
        true);
}

void ra_read_view_set_rssi(RaReadView* instance, float rssi) {
    furi_assert(instance);
    bool changed = false;
    with_view_model(
        instance->view,
        RaReadViewModel * model,
        {
            // A redraw per tick for a bar that has not moved a pixel is not worth it
            changed = ra_read_view_rssi_fill(rssi) != ra_read_view_rssi_fill(model->rssi);
            model->rssi = rssi;
        },
        changed);
}

void ra_read_view_set_status(RaReadView* instance, const char* status, const char* detail) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RaReadViewModel * model,
        {
            furi_string_set_str(model->status_line, status ? status : "");
            furi_string_set_str(model->status_detail, detail ? detail : "");
        },
        true);
}

void ra_read_view_clear(RaReadView* instance) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RaReadViewModel * model,
        {
            furi_string_reset(model->text);
            model->lines = 0;
            model->scroll = 0;
            model->count = 0;
            model->kind = RaReadViewKindNone;
            furi_string_reset(model->status_line);
            furi_string_reset(model->status_detail);
        },
        true);
}

void ra_read_view_set_radio(RaReadView* instance, const char* frequency, const char* modulation) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RaReadViewModel * model,
        { furi_string_printf(model->radio, "%s %s", frequency, modulation); },
        true);
}
