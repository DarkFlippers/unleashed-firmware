LED state describes by struct:

```C
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue; 
} Rgb;
```

LED API provided by struct:

```C
typedef struct {
    ValueComposer* composer; /// every app add its value to compose, <Rgb*>
    ValueManager* state; /// LED value state and changes <Rgb*>
} LedApi;
```

You can get API instance by calling `open_led`:

```C
/// Add new layer to LED:
inline LedApi* open_led(const char* name) {
    return (LedApi*)furi_open(name);
}
```

Default system led is `/dev/led`.

To read current led state you should use `read_led` function:

```C
/// return true if success, false otherwise
inline bool read_led(LedApi* led, Rgb* value, uint32_t timeout) {
    return read_mutex(led->state->value, (void*)value, sizeof(Rgb), timeout);
}
```

Also you can subscribe to led state changes:

Use `subscribe_led_changes` to register your callback:

```C
/// return true if success, false otherwise
inline bool subscribe_led_changes(LedApi* led, void(*cb)(Rgb*, void*), void* ctx) {
    return subscribe_pubsub(led->state->pubsub, void(*)(void*, void*)(cb), ctx);
}
```

Userspace helpers

```C
typedef struct {
    Rgb value;
    ValueMutex value_mutex;
    ValueComposerHandle* composer_handle;
} SystemLed;

inline bool init_led_composer(SystemLed* led, LedApi* api, uint32_t layer) {
    if(!init_mutex(&led->value_mutex, (void*)&led->value, sizeof(Rgb))) {
        return false;
    }
    led->composer_handle = add_compose_layer(
        api->composer, COPY_COMPOSE, &led->value_mutex, layer
    ); // just copy led state on update

    return led->composer_handle != NULL;
}

inline void write_led(SystemLed* led, Rgb* value) {
    write_mutex(&led->value_mutex, (void*)value, sizeof(Rgb), OsWaitForever);
    request_compose(led->composer_handle);
}
```


## Usage example

```C

void handle_led_state(Rgb* rgb, void* _ctx) {
    printf("led: #%02X%02X%02X\n", rgb->red, rgb->green, rgb->blue);
}

void led_example(void* p) {
    LedApi* led_api = open_led("/dev/led");
    if(led_api == NULL) return; // led not available, critical error

    // subscribe to led state updates
    subscribe_led_changes(led_api, handle_led_state, NULL);
    // get current led value
    Rgb led_value;
    if(read_led(led_api, &led_value, OsWaitForever)) {
        printf(
            "initial led: #%02X%02X%02X\n",
            led_value->red,
            led_value->green,
            led_value->blue
        );
    }

    // create compose to control led
    SystemLed system_led;
    if(!init_led_composer(&system_led, led_api, UiLayerBelowNotify)) return;

    // write RGB value
    write_led(&system_led, &(Rgb{.red = 0xFA, green = 0xCE, .blue = 0x8D}));
}
```
