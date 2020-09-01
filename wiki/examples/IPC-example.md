In this example we show how to interact data between different applications.
As we already know, we can use FURI Record for this purpose: one application create named record and other application opens it by name.

We will simulate the display. The first application (called `application_ipc_display`) will be display driver, and the second app (called `application_ipc_widget`) will be draw such simple demo on the screen.

# Dsiplay definition

For work with the display we create simple framebuffer and write some "pixels" into it.

```C
#define FB_WIDTH 10
#define FB_HEIGHT 3
#define FB_SIZE (FB_WIDTH * FB_HEIGHT)

char _framebuffer[FB_SIZE];

for(size_t i = 0; i < FB_SIZE; i++) {
    _framebuffer[i] = ' ';
}
```

On local target we just draw framebuffer content like this:
```
    +==========+
    |          |
    |          |
    |          |
    +==========+
```

```C
fuprintf(log, "+==========+\n");
for(uint8_t i = 0; i < FB_HEIGHT; i++) {
    strncpy(row_buffer, &fb[FB_WIDTH * i], FB_WIDTH);
    fuprintf(log, "|%s|\n", row_buffer);
}
fuprintf(log, "+==========+\n");
```

_Notice: after creating display emulator this example should be changed to work with real 128×64 display using u8g2_

# Demo "widget" application

The application opens record with framebuffer:

```C
FuriRecordSubscriber* fb_record = furi_open(
    "test_fb", false, false, NULL, NULL, NULL
);
```

Then it clear display and draw "pixel" every 120 ms, and pixel move across the screen.

For do that we should tale framebuffer:

`char* fb = (char*)furi_take(fb_record);`

Write some data:

```C
if(fb == NULL) furiac_exit(NULL);

for(size_t i = 0; i < FB_SIZE; i++) {
    fb[i] = ' ';
}

fb[counter % FB_SIZE] = '#';
```
And give framebuffer (use `furi_commit` to notify another apps that record was changed):

`furi_commit(fb_record);`

`counter` is increasing on every iteration to make "pixel" moving.

# Display driver

The driver application creates framebuffer after start (see [Display definition](#Display-definition)) and creates new FURI record with framebuffer pointer:

`furi_create("test_fb", (void*)_framebuffer, FB_SIZE)`

Next it opens this record and subscribe to its changing:

```
FuriRecordSubscriber* fb_record = furi_open(
    "test_fb", false, false, handle_fb_change, NULL, &ctx
);
```

The handler is called any time when some app writes to framebuffer record (by calling `furi_commit`):

```C
static void handle_fb_change(const void* fb, size_t fb_size, void* raw_ctx) {
    IpcCtx* ctx = (IpcCtx*)raw_ctx; // make right type

    fuprintf(ctx->log, "[cb] framebuffer updated\n");

    // send event to app thread
    xSemaphoreGive(ctx->events);

    // Attention! Please, do not make blocking operation like IO and waits inside callback
    // Remember that callback execute in calling thread/context
}
```

That callback execute in calling thread/context, so handler pass control flow to app thread by semaphore. App thread wait for semaphore and then do the "rendering":

```C
if(xSemaphoreTake(events, portMAX_DELAY) == pdTRUE) {
    fuprintf(log, "[display] get fb update\n\n");

    #ifdef HW_DISPLAY
    // on Flipper target draw the screen
    #else
    // on local target just print
    {
        void* fb = furi_take(fb_record);
        print_fb((char*)fb, log);
        furi_give(fb_record);
    }
    #endif
}
```

A structure containing the context is used so that the callback can access the semaphore. This structure is passed as an argument to `furi_open` and goes to the handler:

```C
typedef struct {
    SemaphoreHandle_t events; // queue to pass events from callback to app thread
    FuriRecordSubscriber* log; // app logger
} IpcCtx;
```

We just have to create a semaphore and define a context:

```C
StaticSemaphore_t event_descriptor;
// create stack-based counting semaphore
SemaphoreHandle_t events = xSemaphoreCreateCountingStatic(255, 0, &event_descriptor);

IpcCtx ctx = {.events = events, .log = log};
```

You can find full example code in `applications/examples/ipc.c`, and run it by `docker-compose exec dev make -C target_lo example_ipc`.

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/application_examples/example_ipc.gif)
