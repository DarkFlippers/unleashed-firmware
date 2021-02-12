#include <furi.h>
#include "app-template.h"

/*
To use this example you need to rename
AppExampleState - class to hold app state
AppExampleEvent - class to hold app event
AppExample      - app class
app_cpp_example - function that launch app
*/

// event enumeration type
typedef uint8_t event_t;

// app state class
class AppExampleState {
public:
    // state data
    uint8_t example_data;

    // state initializer
    AppExampleState() {
        example_data = 0;
    }
};

// app events class
class AppExampleEvent {
public:
    // events enum
    static const event_t EventTypeTick = 0;
    static const event_t EventTypeKey = 1;

    // payload
    union {
        InputEvent input;
    } value;

    // event type
    event_t type;
};

// our app derived from base AppTemplate class
// with template variables <state, events>
class AppExample : public AppTemplate<AppExampleState, AppExampleEvent> {
public:
    uint8_t run();
    void render(Canvas* canvas);
};

// start app
uint8_t AppExample::run() {
    // here we dont need to acquire or release state
    // because before we call app_ready our application is "single threaded"
    state.example_data = 12;

    // signal that we ready to render and ipc
    app_ready();

    // from here, any data that pass in render function must be guarded
    // by calling acquire_state and release_state

    AppExampleEvent event;
    while(1) {
        if(get_event(&event, 1000)) {
            if(event.type == AppExampleEvent::EventTypeKey) {
                // press events
                if(event.value.input.type == InputTypeShort && event.value.input.key == InputKeyBack) {
                    printf("bye!\n");
                    return exit();
                }

                if(event.value.input.type == InputTypeShort && event.value.input.key == InputKeyUp) {
                    // to read or write state you need to execute
                    // acquire modify release state
                    acquire_state();
                    state.example_data = 24;
                    release_state();
                }
            }
        }

        // signal to force gui update
        update_gui();
    };
}

// render app
void AppExample::render(Canvas* canvas) {
    // here you dont need to call acquire_state or release_state
    // to read or write app state, that already handled by caller
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, state.example_data, "Example app");
}

// app enter function
extern "C" uint8_t app_cpp_example(void* p) {
    AppExample* app = new AppExample();
    return app->run();
}
