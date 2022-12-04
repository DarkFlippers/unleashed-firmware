#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <stdbool.h> // Header-file for boolean data-type.

typedef struct selected_position {
    int x;
    int y;
} selected_position;

typedef struct {
    selected_position selected;
    bool board[32][16];
    bool isDrawing;
} PaintData;

void paint_draw_callback(Canvas* canvas, void* ctx) {
    const PaintData* paint_state = acquire_mutex((ValueMutex*)ctx, 25);
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    //draw the canvas(32x16) on screen(144x64) using 4x4 tiles
    for(int y = 0; y < 16; y++) {
        for(int x = 0; x < 32; x++) {
            if(paint_state->board[x][y]) {
                canvas_draw_box(canvas, x * 4, y * 4, 4, 4);
            }
        }
    }

    //draw cursor as a 4x4 black box with a 2x2 white box inside
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, paint_state->selected.x * 4, paint_state->selected.y * 4, 4, 4);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(
        canvas, paint_state->selected.x * 4 + 1, paint_state->selected.y * 4 + 1, 2, 2);

    //release the mutex
    release_mutex((ValueMutex*)ctx, paint_state);
}

void paint_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t paint_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    PaintData* paint_state = malloc(sizeof(PaintData));
    ValueMutex paint_state_mutex;
    if(!init_mutex(&paint_state_mutex, paint_state, sizeof(PaintData))) {
        FURI_LOG_E("paint", "cannot create mutex\r\n");
        free(paint_state);
        return -1;
    }

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, paint_draw_callback, &paint_state_mutex);
    view_port_input_callback_set(view_port, paint_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    //NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    InputEvent event;

    while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        //break out of the loop if the back key is pressed
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            break;
        }

        //check the key pressed and change x and y accordingly
        if(event.type == InputTypeShort) {
            switch(event.key) {
            case InputKeyUp:
                paint_state->selected.y -= 1;
                break;
            case InputKeyDown:
                paint_state->selected.y += 1;
                break;
            case InputKeyLeft:
                paint_state->selected.x -= 1;
                break;
            case InputKeyRight:
                paint_state->selected.x += 1;
                break;
            case InputKeyOk:
                paint_state->board[paint_state->selected.x][paint_state->selected.y] =
                    !paint_state->board[paint_state->selected.x][paint_state->selected.y];
                break;

            default:
                break;
            }

            //check if cursor position is out of bounds and reset it to the closest position
            if(paint_state->selected.x < 0) {
                paint_state->selected.x = 0;
            }
            if(paint_state->selected.x > 31) {
                paint_state->selected.x = 31;
            }
            if(paint_state->selected.y < 0) {
                paint_state->selected.y = 0;
            }
            if(paint_state->selected.y > 15) {
                paint_state->selected.y = 15;
            }
            if(paint_state->isDrawing == true) {
                paint_state->board[paint_state->selected.x][paint_state->selected.y] = true;
            }
            view_port_update(view_port);
        }
        if(event.key == InputKeyBack && event.type == InputTypeLong) {
            paint_state->board[1][1] = true;
            for(int y = 0; y < 16; y++) {
                for(int x = 0; x < 32; x++) {
                    paint_state->board[x][y] = false;
                }
            }
            view_port_update(view_port);
        }
        if(event.key == InputKeyOk && event.type == InputTypeLong) {
            paint_state->isDrawing = !paint_state->isDrawing;
            paint_state->board[paint_state->selected.x][paint_state->selected.y] = true;
            view_port_update(view_port);
        }
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    free(paint_state);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    return 0;
}
