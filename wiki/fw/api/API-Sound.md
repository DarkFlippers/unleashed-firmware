sound state describes by struct:

```C
typedef struct {
    float freq; /// frequency in Hz
    float width; /// pulse witdh 0...1
} Tone;
```

sound API provided by struct:

```C
typedef struct {
    LayeredReducer* source; /// every app add its layer to set value, LayeredReducer<Tone*>
    Subscriber* updates; /// sound value changes Supscriber<Tone*>
    ValueMutex* state; /// sound state, ValueMutex<Tone*>
} SoundApi;
```

You can get API instance by calling `open_sound`:

```C
/// Add new layer to sound:
inline SoundApi* open_sound(const char* name) {
    return (SoundApi*)furi_open(name);
}
```

Default system sound is `/dev/sound`.

Then add new layer to control sound by calling `add_sound_layer`:

```C
inline ValueManager* add_sound_layer(Tone* layer, uint8_t priority) {
    ValueManager* manager = register_valuemanager((void*)layer);
    if(manager == NULL) return NULL;

    if(!add_layered_reducer(manager, priority, layer_compose_default)) {
        unregister_valuemanager(manager);
        return NULL;
    }

    return manager;
}
```

For change sound you can get display instance pointer by calling `take_sound`, do something and commit your changes by calling `commit_sound`. Or you can call `write_sound`:

```C
/// return pointer in case off success, NULL otherwise
inline Tone* take_sound(ValueManager* sound, uint32_t timeout) {
    return (Tone*)take_mutex(sound->value, timeout);
}

inline void commit_sound(ValueManager* sound, Tone* value) {
    commit_valuemanager(sound, value);
}

/// return true if success, false otherwise
inline bool write_sound(ValueManager* sound, Tone* value, uint32_t timeout) {
    return write_valuemanager(state, (void*)value, sizeof(Tone), timeout);
}
```

To read current sound state you should use `read_sound` function:

```C
/// return true if success, false otherwise
inline bool read_sound(ValueManager* sound, Tone* value, uint32_t timeout) {
    return read_mutex(sound->value, (void*)value, sizeof(Tone), timeout);
}
```

Also you can subscribe to sound state changes:

Use `subscribe_sound_changes` to register your callback:

```C
/// return true if success, false otherwise
inline bool subscribe_sound_changes(Subscriber* updates, void(*cb)(Tone*, void*), void* ctx) {
    return subscribe_pubsub(events, void(*)(void*, void*)(cb), ctx);
}
```

## Usage example

```C

void handle_sound_state(Tone* tone, void* _ctx) {
    printf("sound: %d Hz, %d %%\n", (uint16_t)tone->freq, (uint8_t)(tone->witdh * 100));
}

void sound_example(void* p) {
    soundApi* sound_api = open_display("/dev/sound");
    if(sound_api == NULL) return; // sound not available, critical error

    // subscribe to sound state updates
    subscribe_sound_changes(sound_api->updates, handle_sound_state, NULL);

    Tone current_state;
    if(read_sound(sound_api->state, &current_state, OsWaitForever)) {
        printf(
            "sound: %d Hz, %d %%\n",
            (uint16_t)current_state->freq,
            (uint8_t)(current_state->witdh * 100)
        );
    }

    // add layer to control sound
    ValueManager* sound_manager = add_sound_layer(&current_state, UI_LAYER_APP);

    // write only freq by getting pointer
    Tone* tone = take_sound(sound_manager, OsWaitForever);
    if(tone != NULL) {
        tone->freq = 440;
    }
    commit_sound(sound_manager, tone);

    // write tone value
    write_sound(sound_manager, &(Tone{.freq = 110., witdh = 0.5}), OsWaitForever);
}
```
