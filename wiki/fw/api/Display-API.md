All display operations based on [u8g2](https://github.com/olikraus/u8g2) library.

API available as `ValueComposer`.

Driver call render callback and pass API contains u8g2 functions, instance and fonts:

```C
typedef struct {
    u8g2_t* display;

    void (*u8g2_SetFont)(u8g2_t *u8g2, const uint8_t  *font);
    void (*u8g2_SetDrawColor)(u8g2_t *u8g2, uint8_t color);
    void (*u8g2_SetFontMode)(u8g2_t *u8g2, uint8_t is_transparent);
    u8g2_uint_t (*u8g2_DrawStr)(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, const char *str);

    Fonts fonts;
} DisplayApi;

typedef struct {
    const uint8_t* u8g2_font_6x10_mf;
} Fonts;
```

First of all you can open display API instance by calling `open_display`

```C
/// Get display instance and API
inline Display* open_display(const char* name) {
    return (Display*)furi_open(name);
}
```

Default display name is `/dev/display`.

For draw something to display you need to register new layer in display composer:

```C
typedef void (RenderCallback*)(void* ctx, DisplayApi* api);

inline ValueComposerHandle* init_display_composer(
    Display* api, RenderCallback render, void* ctx, uint32_t layer) {
    return add_compose_layer(api->composer, (ValueComposerCallback)render, ctx, layer);
}
```

And then call `request_compose` every time you need to redraw your image.

## Usage example

```C

void example_render(void* ctx, DisplayApi* api) {
    api->u8g2_SetFont(api->display, display_api->fonts.u8g2_font_6x10_mf);
    api->u8g2_SetDrawColor(api->display, 1);
    api->u8g2_SetFontMode(api->display, 1);
    api->u8g2_DrawStr(api->display, 2, 12, (char*)ctx); // ctx contains some static text
}

void u8g2_example(void* p) {
    Display* display_api = open_display("/dev/display");
    if(display_api == NULL) return; // display not available, critical error

    ValueComposerHandle display_handler = init_display_composer(
        display_api, example_render, (void*)"Hello world", UiLayerBelowNotify);

    request_compose(display_handler);
}
```