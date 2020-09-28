All input API available by struct:

```C
typedef struct {
    Subscriber* events; /// debounced keyboards events: press/release, Subscriber<InputEvent*>
    Subscriber* raw_events; /// raw keyboards events: press/release, Subscriber<InputEvent*>
    ValueMutex* state; /// current keyboard state, ValueMutex<InputState*>
} Input;
```

You can get API instance by calling `open_input`:

```C
/// Get input struct
inline Input* open_input(const char* name) {
    return furi_open(name);
}
```

Default (system) input name is `/dev/kb`.

Buttons state store as struct:

```C
/// Current state of buttons
typedef struct {
    bool up;
    bool down;
    bool right;
    bool left;
    bool ok;
    bool back;
} InputState;
```

To read buttons state you should use `read_state` function:

```C
/// read current state of all buttons. Return true if success, false otherwise
inline bool read_state(ValueMutex* state, InputState* value, uint32_t timeout) {
    return read_mutex(state, (void*)value, sizeof(InputState), timeout);
}
```

Also you can subscribe to input events:

```C
/// used to pass button press/release evens
typedef struct {
    Inputs input; /// what button
    bool state; /// true = press, false = release
} InputEvent;

/// List of buttons
typedef enum {
    InputsUp = 0,
    InputsDown,
    InputsRight,
    InputsLeft,
    InputsOk,
    InputsBack,
    InputsSize
} Inputs;
```

Use `subscribe_input_events` to register your callback:

```C
/// subscribe on button press/release events. Return true if success, false otherwise
inline bool subscribe_input_events(Subscriber* events, void(*cb)(InputEvent*, void*), void* ctx) {
    return subscribe_pubsub(events, void(*)(void*, void*)(cb), ctx);
}
```

## Usage example

```C
// function used to handle keyboard events
void handle_keyboard(InputEvent* event, void* _ctx) {
    if(event->state) {
        printf("you press %d", event->input);
    } else {
        printf("you release %d", event->input);
    }
}

void input_example(void* p) {
    Input* input = open_input("/dev/kb");
    if(input == NULL) return; // keyboard not available, critical error

    // async way
    subscribe_input_events(input->events, handle_keyboard, NULL);

    // blocking way
    InputState state;
    while(1) {
        if(read_state(input->state, &state, OsWaitForever)) {
            if(state.up) {
                printf("up is pressed");
                delay(1000);
            }
        }

        delay(10);
    }
}
```
