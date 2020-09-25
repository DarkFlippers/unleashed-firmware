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
    LayeredReducer* source; /// every app add its layer to set value, LayeredReducer<Rgb*>
    Subscriber* updates; /// LED value changes Supscriber<Rgb*>
    ValueMutex* state; /// LED state, ValueMutex<Rgb*>
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

Then add new layer to control LED by calling `add_led_layer`:

```C
inline ValueManager* add_led_layer(Rgb* layer, uint8_t priority) {
    ValueManager* manager = register_valuemanager((void*)layer);
    if(manager == NULL) return NULL;

    if(!add_layered_reducer(manager, priority, layer_compose_default)) {
        unregister_valuemanager(manager);
        return NULL;
    }

    return manager;
}
```

For change led you can get display instance pointer by calling `take_led`, do something and commit your changes by calling `commit_led`. Or you can call `write_led`:

```C
/// return pointer in case off success, NULL otherwise
inline Rgb* take_led(ValueManager* led, uint32_t timeout) {
    return (Rgb*)take_mutex(led->value, timeout);
}

inline void commit_led(ValueManager* led, Rgb* value) {
    commit_valuemanager(led, value);
}

/// return true if success, false otherwise
inline bool write_led(ValueManager* led, Rgb* value, uint32_t timeout) {
    return write_valuemanager(state, (void*)value, sizeof(Rgb), timeout);
}
```

To read current led state you should use `read_led` function:

```C
/// return true if success, false otherwise
inline bool read_led(ValueManager* led, Rgb* value, uint32_t timeout) {
    return read_mutex(led->value, (void*)value, sizeof(Rgb), timeout);
}
```

Also you can subscribe to led state changes:

Use `subscribe_led_changes` to register your callback:

```C
/// return true if success, false otherwise
inline bool subscribe_led_changes(Subscriber* updates, void(*cb)(Rgb*, void*), void* ctx) {
    return subscribe_pubsub(events, void(*)(void*, void*)(cb), ctx);
}
```

## Usage example

```C

void handle_led_state(Rgb* rgb, void* _ctx) {
    printf("led: #%02X%02X%02X\n", rgb->red, rgb->green, rgb->blue);
}

void led_example(void* p) {
    LedApi* led_api = open_display("/dev/led");
    if(led_api == NULL) return; // led not available, critical error

    // subscribe to led state updates
    subscribe_led_changes(led_api->updates, handle_led_state, NULL);

    Rgb current_state;
    if(read_led(led_api->state, &current_state, OsWaitForever)) {
        printf(
            "initial led: #%02X%02X%02X\n",
            current_state->red,
            current_state->green,
            current_state->blue
        );
    }

    // add layer to control led
    ValueManager* led_manager = add_led_layer(&current_state, UI_LAYER_APP);

    // write only blue by getting pointer
    Rgb* rgb = take_led(led_manager, OsWaitForever);
    if(rgb != NULL) {
        rgb->blue = 0;
    }
    commit_led(led_manager, rgb);

    // write RGB value
    write_led(led_manager, &(Rgb{.red = 0xFA, green = 0xCE, .blue = 0x8D}), OsWaitForever);
}
```
