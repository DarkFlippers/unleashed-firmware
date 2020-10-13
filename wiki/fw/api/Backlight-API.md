Backlight state describes by `uint8_t level;` brightness level.

LED API provided by struct:

```C
typedef struct {
    ValueComposer* composer; /// every app add its value to compose, <uint8_t*>
    ValueManager* state; /// value state and changes <uint8_t*>
} BacklightApi;
```

You can get API instance by calling `open_backlight`:

```C
/// Add new layer to LED:
inline BacklightApi* open_backlight(const char* name) {
    return (BacklightApi*)furi_open(name);
}
```

Default system led is `/dev/backlight`.

To read current backlight state you should use `read_backlight` function:

```C
/// return true if success, false otherwise
inline bool read_backlight(BacklightApi* api, uint8_t* value, uint32_t timeout) {
    return read_mutex(api->state->value, (void*)value, sizeof(uint8_t), timeout);
}
```

Also you can subscribe to backlight state changes:

Use `subscribe_backlight_changes` to register your callback:

```C
/// return true if success, false otherwise
inline bool subscribe_backlight_changes(LedApi* led, void(*cb)(uint8_t*, void*), void* ctx) {
    return subscribe_pubsub(led->state->pubsub, void(*)(void*, void*)(cb), ctx);
}
```

Userspace helpers

```C
typedef struct {
    uint8_t value;
    ValueMutex value_mutex;
    ValueComposerHandle* composer_handle;
} Backlight;

inline bool init_backlight_composer(Backlight* backlight, BacklightApi* api, uint32_t layer) {
    if(!init_mutex(&backlight->value_mutex, (void*)&backlight->value, sizeof(uint8_t))) {
        return false;
    }
    backlight->composer_handle = add_compose_layer(
        api->composer, COPY_COMPOSE, &backlight->value_mutex, layer
    ); // just copy backlight state on update

    return backlight->composer_handle != NULL;
}

inline void write_backlight(Backlight* backlight, uint8_t value) {
    write_mutex(&backlight->value_mutex, (void*)&value, sizeof(uint8_t), OsWaitForever);
    request_compose(backlight->composer_handle);
}
```


## Usage example

```C

void handle_backlight_state(uint8_t* value, void* _ctx) {
    printf("backlight: %d %%\n", (*value * 100) / 256);
}

void backlight_example(void* p) {
    BacklightApi* backlight_api = open_backlight("/dev/backlight");
    if(backlight_api == NULL) return; // backlight not available, critical error

    // subscribe to led state updates
    subscribe_backlight_changes(backlight_api, handle_backlight_state, NULL);
    // get current backlight value
    uint8_t backlight_value;
    if(read_backlight(backlight_api, &backlight_value, OsWaitForever)) {
        printf(
            "initial backlight: %d %%\n",
            backlight_value * 100 / 256
        );
    }

    // create compose to control led
    Backlight backlight;
    if(!init_led_composer(&backlight, backlight_api, UiLayerBelowNotify)) return;

    // write RGB value
    write_backlight(&backlight, 127);
}
```
