#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#include "multi_converter_definitions.h"
#include "multi_converter_mode_display.h"
#include "multi_converter_mode_select.h"

static void multi_converter_render_callback(Canvas* const canvas, void* ctx) {

	const MultiConverterState* multi_converter_state = acquire_mutex((ValueMutex*)ctx, 25);
	if(multi_converter_state == NULL) {
		return;
	}

	if (multi_converter_state->mode == ModeDisplay) {
		multi_converter_mode_display_draw(canvas, multi_converter_state);
	} else {
		multi_converter_mode_select_draw(canvas, multi_converter_state);
	}

	release_mutex((ValueMutex*)ctx, multi_converter_state);
}

static void multi_converter_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
	furi_assert(event_queue); 

	MultiConverterEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void multi_converter_init(MultiConverterState* const multi_converter_state) {
	// initial default values

	multi_converter_state->buffer_orig[MULTI_CONVERTER_NUMBER_DIGITS] = '\0';
	multi_converter_state->buffer_dest[MULTI_CONVERTER_NUMBER_DIGITS] = '\0'; // null terminators

	multi_converter_state->unit_type_orig = UnitTypeDec;
	multi_converter_state->unit_type_dest = UnitTypeHex;

	multi_converter_state->keyboard_lock = 0;

	// init the display view
	multi_converter_mode_display_reset(multi_converter_state);

	// init the select view
	multi_converter_mode_select_reset(multi_converter_state);

	// set ModeDisplay as the current mode
	multi_converter_state->mode = ModeDisplay;
}

// main entry point
int32_t multi_converter_app(void* p) {
    UNUSED(p);

	// get event queue
	FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(MultiConverterEvent));

	// allocate state
	MultiConverterState* multi_converter_state = malloc(sizeof(MultiConverterState));

	// set mutex for plugin state (different threads can access it)
	ValueMutex state_mutex;
	if(!init_mutex(&state_mutex, multi_converter_state, sizeof(multi_converter_state))) {
		FURI_LOG_E("MultiConverter", "cannot create mutex\r\n");
		free(multi_converter_state);
		return 255;
	}

	// register callbacks for drawing and input processing
	ViewPort* view_port = view_port_alloc();
	view_port_draw_callback_set(view_port, multi_converter_render_callback, &state_mutex);
	view_port_input_callback_set(view_port, multi_converter_input_callback, event_queue);

	// open GUI and register view_port
	Gui* gui = furi_record_open("gui");
	gui_add_view_port(gui, view_port, GuiLayerFullscreen);

	multi_converter_init(multi_converter_state);

	// main loop
	MultiConverterEvent event;
	for (bool processing = true; processing;) {
		FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
		MultiConverterState* multi_converter_state = (MultiConverterState*)acquire_mutex_block(&state_mutex);

		if (event_status == FuriStatusOk) {
			// press events
			if (event.type == EventTypeKey && !multi_converter_state->keyboard_lock) {
				if (multi_converter_state->mode == ModeDisplay) {

					if (event.input.key == InputKeyBack) {
						if (event.input.type == InputTypePress) processing = false;
					} else if (event.input.key == InputKeyOk) { // the "ok" press can be short or long
						MultiConverterModeTrigger t = None;

						if (event.input.type == InputTypeLong) t = multi_converter_mode_display_ok(1, multi_converter_state);
						else if (event.input.type == InputTypeShort) t = multi_converter_mode_display_ok(0, multi_converter_state);

						if (t == Reset) {
							multi_converter_mode_select_reset(multi_converter_state);
							multi_converter_state->mode = ModeSelector;
						}
					} else {
						if (event.input.type == InputTypePress) multi_converter_mode_display_navigation(event.input.key, multi_converter_state);
					}
						
				} else { // ModeSelect 
					if (event.input.type == InputTypePress) {
						switch (event.input.key) {
							default:
								break;
							case InputKeyBack:
							case InputKeyOk: {
								MultiConverterModeTrigger t = multi_converter_mode_select_exit(event.input.key == InputKeyOk ? 1 : 0, multi_converter_state);

								if (t == Reset) {
									multi_converter_mode_display_reset(multi_converter_state);
								} else if (t == Convert) {
									multi_converter_mode_display_convert(multi_converter_state);
								}

								multi_converter_state->keyboard_lock = 1;
								multi_converter_state->mode = ModeDisplay;
								break;
							}
							case InputKeyLeft:
							case InputKeyRight:
								multi_converter_mode_select_switch(multi_converter_state);
								break;
							case InputKeyUp:
								multi_converter_mode_select_change_unit(-1, multi_converter_state);
								break;
							case InputKeyDown:
								multi_converter_mode_select_change_unit(1, multi_converter_state);
								break;
						}
					}
				}
			} else if (multi_converter_state->keyboard_lock) {
				multi_converter_state->keyboard_lock = 0;
			}
		} else {
			// event timeout
		}
	
		view_port_update(view_port);
		release_mutex(&state_mutex, multi_converter_state);
	}

	view_port_enabled_set(view_port, false);
	gui_remove_view_port(gui, view_port);
	furi_record_close("gui");
	view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    free(multi_converter_state);

    return 0;
}