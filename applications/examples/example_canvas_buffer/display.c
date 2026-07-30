#include <furi.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>

#include <math.h>
#include <stdlib.h>

/* Bouncing-cube demo built around direct framebuffer access with
 * canvas_get_buffer().
 *
 * The returned pointer is the canvas backing store: a 128x64 1-bit
 * framebuffer laid out as 8 pages of vertical 8-pixel strips, byte =
 * (y / 8) * width + x, bit = y % 8, LSB on top. Everything the canvas API
 * draws earlier in the frame is already rasterized there, which enables:
 *
 * - reading pixels back (fb_hit): the canvas drawing API is write-only, so
 *   the buffer is the only way to test what is on screen; here it drives
 *   pixel-exact collision of the cube against the text glyphs;
 * - writing pixels directly (fb_put_pixel, fb_draw_line) to rasterize the
 *   cube in place, bypassing canvas_draw_line().
 *
 * The buffer is only touched inside the draw callback, where its contents
 * for the current frame are valid.
 */

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

#define FIXED_SHIFT 10
#define FIXED_ONE   (1 << FIXED_SHIFT)

#define SIN_LUT_SIZE 256

#define CAMERA_DISTANCE (4 * FIXED_ONE)
#define FOCAL_LENGTH    24

#define SPIN_MAX     700
#define TORQUE_SHIFT 6
#define SPIN_DAMPING 7
#define TUMBLE_SHIFT 6

#define POS_SHIFT    8
#define MOVE_SPEED_X 179
#define MOVE_SPEED_Y 133

typedef struct {
    uint16_t phase[3];
    int16_t speed[3];
    int32_t pos_x;
    int32_t pos_y;
    int32_t vel_x;
    int32_t vel_y;
} CubeState;

static int16_t sin_lut[SIN_LUT_SIZE];

static inline int16_t lut_sin(uint16_t phase) {
    return sin_lut[phase >> 8];
}

static inline int16_t lut_cos(uint16_t phase) {
    return sin_lut[((phase >> 8) + (SIN_LUT_SIZE / 4)) & (SIN_LUT_SIZE - 1)];
}

static const int8_t cube_vertex[8][3] = {
    {-1, -1, -1},
    {1, -1, -1},
    {1, 1, -1},
    {-1, 1, -1},
    {-1, -1, 1},
    {1, -1, 1},
    {1, 1, 1},
    {-1, 1, 1},
};

static const uint8_t cube_edge[12][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
    {4, 5},
    {5, 6},
    {6, 7},
    {7, 4},
    {0, 4},
    {1, 5},
    {2, 6},
    {3, 7},
};

static const int16_t inv_inertia[3] = {18, 16, 13};

static inline int16_t spin_clamp(int32_t value) {
    if(value > SPIN_MAX) return SPIN_MAX;
    if(value < -SPIN_MAX) return -SPIN_MAX;
    return (int16_t)value;
}

static void cube_animate(CubeState* state) {
    state->speed[0] += state->speed[2] >> TUMBLE_SHIFT;
    state->speed[2] -= state->speed[0] >> TUMBLE_SHIFT;
    state->speed[1] += state->speed[0] >> (TUMBLE_SHIFT + 1);
    state->speed[0] -= state->speed[1] >> (TUMBLE_SHIFT + 1);

    for(uint32_t i = 0; i < 3; i++) {
        state->speed[i] -= state->speed[i] >> SPIN_DAMPING;
        state->phase[i] += state->speed[i];
    }
}

/* Set one pixel directly in the canvas_get_buffer() framebuffer: one byte
 * holds a vertical 8-pixel strip, byte = (y / 8) * width + x, bit = y % 8.
 * Out-of-screen coordinates are ignored. */
static inline void fb_put_pixel(uint8_t* fb, int32_t x, int32_t y) {
    if((uint32_t)x < SCREEN_WIDTH && (uint32_t)y < SCREEN_HEIGHT) {
        fb[((y >> 3) * SCREEN_WIDTH) + x] |= 1 << (y & 7);
    }
}

/* Read one pixel back from the framebuffer. The canvas drawing API is
 * write-only, so this is the only way to test what has already been
 * rasterized this frame. Off-screen coordinates report as set, making the
 * screen edges solid walls for the collision pass. */
static inline bool fb_hit(const uint8_t* fb, int32_t x, int32_t y) {
    if((uint32_t)x >= SCREEN_WIDTH || (uint32_t)y >= SCREEN_HEIGHT) return true;
    return (fb[((y >> 3) * SCREEN_WIDTH) + x] & (1 << (y & 7))) != 0;
}

/* Bresenham line rasterized into the framebuffer pixel by pixel through
 * fb_put_pixel() instead of canvas_draw_line(). */
static void fb_draw_line(uint8_t* fb, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    int32_t dx = abs(x1 - x0);
    int32_t dy = -abs(y1 - y0);
    int32_t step_x = x0 < x1 ? 1 : -1;
    int32_t step_y = y0 < y1 ? 1 : -1;
    int32_t err = dx + dy;

    for(;;) {
        fb_put_pixel(fb, x0, y0);
        if(x0 == x1 && y0 == y1) break;
        int32_t e2 = 2 * err;
        if(e2 >= dy) {
            err += dy;
            x0 += step_x;
        }
        if(e2 <= dx) {
            err += dx;
            y0 += step_y;
        }
    }
}

static void cube_draw_callback(Canvas* canvas, void* context) {
    CubeState* state = context;
    cube_animate(state);

    canvas_draw_str(canvas, 2, 9, "Angry pixels");
    canvas_draw_str(canvas, 92, 9, "run!");
    canvas_draw_str(canvas, 4, 38, "bonk!");
    canvas_draw_str(canvas, 99, 38, "ouch!");
    canvas_draw_str(canvas, 20, 62, "the floor is lava");

    int32_t sin_x = lut_sin(state->phase[0]), cos_x = lut_cos(state->phase[0]);
    int32_t sin_y = lut_sin(state->phase[1]), cos_y = lut_cos(state->phase[1]);
    int32_t sin_z = lut_sin(state->phase[2]), cos_z = lut_cos(state->phase[2]);

    int32_t rx[8];
    int32_t ry[8];
    int32_t rz[8];
    int32_t dx[8];
    int32_t dy[8];

    for(uint32_t i = 0; i < 8; i++) {
        int32_t x = cube_vertex[i][0] * FIXED_ONE;
        int32_t y = cube_vertex[i][1] * FIXED_ONE;
        int32_t z = cube_vertex[i][2] * FIXED_ONE;

        int32_t t = (y * cos_x - z * sin_x) >> FIXED_SHIFT;
        z = (y * sin_x + z * cos_x) >> FIXED_SHIFT;
        y = t;

        t = (x * cos_y + z * sin_y) >> FIXED_SHIFT;
        z = (z * cos_y - x * sin_y) >> FIXED_SHIFT;
        x = t;

        t = (x * cos_z - y * sin_z) >> FIXED_SHIFT;
        y = (x * sin_z + y * cos_z) >> FIXED_SHIFT;
        x = t;

        rx[i] = x;
        ry[i] = y;
        rz[i] = z;

        int32_t depth = z + CAMERA_DISTANCE;
        dx[i] = (x * FOCAL_LENGTH) / depth;
        dy[i] = (y * FOCAL_LENGTH) / depth;
    }

    /* canvas_get_buffer() returns the canvas backing framebuffer directly.
     * At this point it already holds the glyphs rasterized by
     * canvas_draw_str() above: the same buffer is first read back for
     * pixel-exact collision, then written to draw the cube in place. */
    uint8_t* fb = canvas_get_buffer(canvas);

    int32_t cur_x = state->pos_x >> POS_SHIFT;
    int32_t cur_y = state->pos_y >> POS_SHIFT;
    int32_t next_x = (state->pos_x + state->vel_x) >> POS_SHIFT;
    int32_t next_y = (state->pos_y + state->vel_y) >> POS_SHIFT;

    /* Read pass: probe the framebuffer at every projected cube vertex,
     * separately for the advanced X and the advanced Y position, to find out
     * along which axis the cube would run into already-drawn pixels. */
    int32_t torque[3] = {0, 0, 0};
    bool hit_x = false;
    bool hit_y = false;
    for(uint32_t i = 0; i < 8; i++) {
        if(fb_hit(fb, next_x + dx[i], cur_y + dy[i])) {
            hit_x = true;
            int32_t push = (state->vel_x > 0) ? -1 : 1;
            torque[1] += rz[i] * push;
            torque[2] -= ry[i] * push;
        }
        if(fb_hit(fb, cur_x + dx[i], next_y + dy[i])) {
            hit_y = true;
            int32_t push = (state->vel_y > 0) ? -1 : 1;
            torque[0] -= rz[i] * push;
            torque[2] += rx[i] * push;
        }
    }
    for(uint32_t i = 0; i < 3; i++) {
        state->speed[i] =
            spin_clamp(state->speed[i] + ((torque[i] * inv_inertia[i]) >> TORQUE_SHIFT));
    }
    if(hit_x) state->vel_x = -state->vel_x;
    if(hit_y) state->vel_y = -state->vel_y;
    state->pos_x += state->vel_x;
    state->pos_y += state->vel_y;

    int32_t center_x = state->pos_x >> POS_SHIFT;
    int32_t center_y = state->pos_y >> POS_SHIFT;

    /* Write pass: rasterize the cube edges straight into the framebuffer,
     * on top of everything the canvas API drew this frame. */
    for(uint32_t i = 0; i < 12; i++) {
        fb_draw_line(
            fb,
            center_x + dx[cube_edge[i][0]],
            center_y + dy[cube_edge[i][0]],
            center_x + dx[cube_edge[i][1]],
            center_y + dy[cube_edge[i][1]]);
    }
}

static void cube_input_callback(InputEvent* event, void* context) {
    FuriMessageQueue* queue = context;
    furi_message_queue_put(queue, event, 0);
}

int32_t example_canvas_buffer_app(void* arg) {
    UNUSED(arg);

    for(uint32_t i = 0; i < SIN_LUT_SIZE; i++) {
        sin_lut[i] = (int16_t)lroundf(sinf(i * (6.2831853f / SIN_LUT_SIZE)) * FIXED_ONE);
    }

    CubeState state = {0};
    state.speed[0] = 240;
    state.speed[1] = -180;
    state.speed[2] = 130;
    state.pos_x = (SCREEN_WIDTH / 2) << POS_SHIFT;
    state.pos_y = (SCREEN_HEIGHT / 2) << POS_SHIFT;
    state.vel_x = (furi_hal_random_get() & 1) ? MOVE_SPEED_X : -MOVE_SPEED_X;
    state.vel_y = (furi_hal_random_get() & 1) ? MOVE_SPEED_Y : -MOVE_SPEED_Y;

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, cube_draw_callback, &state);
    view_port_input_callback_set(view_port, cube_input_callback, queue);

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_display_backlight_enforce_on);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    bool running = true;
    while(running) {
        InputEvent event;
        if(furi_message_queue_get(queue, &event, 16) == FuriStatusOk) {
            if(event.type == InputTypePress && event.key == InputKeyBack) {
                running = false;
            }
        }
        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);

    notification_message(notification, &sequence_display_backlight_enforce_auto);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_free(view_port);
    furi_message_queue_free(queue);

    return 0;
}
