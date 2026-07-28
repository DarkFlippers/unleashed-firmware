# Canvas Buffer Example

Demonstrates `canvas_get_buffer()` — direct read **and** write access to the raw
canvas framebuffer from a regular external application (FAP).

A wireframe cube flies across the screen, bouncing off the screen borders and
off the text. Its spin is pure collision physics: every impact applies a torque
(r × F for each vertex that hit), the inertia is slightly different per axis,
and the spin axis slowly precesses like an asymmetric top — so the tumble stays
truly three-dimensional instead of settling into a flat, 2D-looking rotation. The text is rendered by the regular canvas API
(`canvas_draw_str()`), and the collision works by reading the glyph pixels back
from the framebuffer — which is exactly what makes this example impossible to
reproduce without `canvas_get_buffer()`.

## What it shows

- `canvas_get_buffer()` — pointer to the live framebuffer of the canvas.
- Buffer layout: one byte covers a vertical 8-pixel strip;
  `byte = (y / 8) * width + x`, `bit = y % 8`, LSB on top.
- **Write path**: Bresenham line rasterizer that puts pixels straight into the
  buffer, bypassing the per-primitive canvas calls.
- **Read path**: per-pixel collision test against glyphs that the canvas API
  itself rendered a moment earlier in the same frame.

## Why this cannot be done without buffer access

The public canvas API is strictly write-only: there is no `canvas_get_pixel()`.
Fonts are rasterized inside u8g2, which is private to the firmware — an
external app cannot obtain glyph bitmaps any other way. So "bounce off the
letters" requires reading the framebuffer, and the only sanctioned way to read
it is `canvas_get_buffer()`.

## History: how full-framebuffer rendering was done before this API

Approaches used in real projects over the years, in the order they were tried.
FPS numbers are the author's measurements of full-screen animation on real
hardware.

1. **Canvas primitives only.** Often simply not enough: no pixel read-back,
   and full-screen software rendering through per-primitive calls is slow.

2. **Own framebuffer + `canvas_draw_xbm()`.** Keep a private buffer in XBM
   layout and blit it every frame. Works, fully official, but the bitmap path
   tops out at about **8 FPS** for full-screen animation. This method (along
   with an even older one) is preserved in the history of
   [FlipperCatacombs](https://git.aperturefox.ru/FlipperZero/FlipperCatacombs.git) —
   the author's oldest game.

3. **`gui_add_framebuffer_callback()`.** Writing into the framebuffer from the
   GUI commit callback. Fast — **67+ FPS** measured — but fundamentally wrong:
   the hook is a *post-send capture* intended for RPC screen streaming. Inside
   `canvas_commit()` it fires **after** `u8g2_SendBuffer()`, so writes are one
   frame late and get wiped by `canvas_reset()` on the next redraw. Worse, the
   app never owns GUI focus: input events are not captured, and view ports and
   the desktop keep fighting for the screen. The historical implementation
   lives in [ardulib](https://git.aperturefox.ru/FlipperZero/ardulib.git). See
   the anti-example below.

4. **`gui_direct_draw_acquire()` / `gui_direct_draw_release()`.** Also part of
   the same historical code. Solves the takeover problem legitimately — the GUI
   suspends view-port composition and hands the canvas over, input is taken via
   the `RECORD_INPUT_EVENTS` pubsub — but it still exposes only the write-only
   drawing primitives, not the raw buffer.

5. **`u8g2_GetBufferPtr(&canvas->fb)` hack.** Reaching into canvas internals
   through the u8g2 struct. It compiled locally, but a FAP release was
   impossible: the API check rejects the symbol — it is only reachable when
   building in-tree with `./fbt`. Preserved in
   [ardulib](https://git.aperturefox.ru/FlipperZero/ardulib.git) as the
   semi-final variant that immediately preceded the public API.

6. **`canvas_get_buffer()` / `canvas_get_buffer_size()` public API** — this
   example. Full-speed read/write access from a regular FAP, with normal
   ViewPort input handling and compositing. The newest project built on top of
   it is [Flipcraft](https://git.aperturefox.ru/FlipperZero/Flipcraft.git).

### Historical anti-example (kept for reference — do not use)

```c
void rt_framebuffer_commit_callback(
    uint8_t* data,
    size_t size,
    CanvasOrientation orientation,
    void* context) {
    ArduboyRuntimeState* state = (ArduboyRuntimeState*)context;
    if(!state || !data) return;
    if(size < RuntimeBufferSize) return;
    (void)orientation;

    const uint8_t* src = state->screen_buffer;
    bool inverted = __atomic_load_n((bool*)&state->screen_inverted, __ATOMIC_ACQUIRE);

    for(size_t i = 0; i < RuntimeBufferSize; i++) {
        if(inverted) {
            data[i] = src[i];
        } else {
            data[i] = (uint8_t)(src[i] ^ 0xFF);
        }
    }

    if(state->pending_clear) {
        memset(state->screen_buffer, 0x00, RuntimeBufferSize);
        state->pending_clear = false;
    }
}
```

This shipped in early Arduboy runtime builds and reached 67+ FPS on the GUI
internals of that era. On current firmware it cannot work correctly: the
callback runs after the frame has already been sent to the display, the app
receives no input, and view ports break. It is documented here as historically
working but not correct.

## Building

```
./fbt fap_example_canvas_buffer
```
