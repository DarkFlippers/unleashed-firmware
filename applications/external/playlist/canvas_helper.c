#include <gui/gui.h>

#define WIDTH 128
#define HEIGHT 64

void draw_centered_boxed_str(Canvas* canvas, int x, int y, int height, int pad, const char* text) {
    // get width of text
    int w = canvas_string_width(canvas, text);
    canvas_draw_rframe(canvas, x, y, w + pad, height, 2);
    canvas_draw_str_aligned(canvas, x + pad / 2, y + height / 2, AlignLeft, AlignCenter, text);
}

void draw_corner_aligned(Canvas* canvas, int width, int height, Align horizontal, Align vertical) {
    canvas_set_color(canvas, ColorBlack);
    switch(horizontal) {
    case AlignLeft:
        switch(vertical) {
        case AlignTop:
            canvas_draw_rbox(canvas, 0, 0, width, height, 3);
            canvas_draw_box(canvas, 0, 0, width, 3);
            canvas_draw_box(canvas, 0, 0, 3, height);
            break;
        case AlignCenter:
            canvas_draw_rbox(canvas, 0, HEIGHT - height / 2, width, height, 3);
            canvas_draw_box(canvas, 0, HEIGHT - height / 2, 3, height);
            break;
        case AlignBottom:
            canvas_draw_rbox(canvas, 0, HEIGHT - height, width, height, 3);
            canvas_draw_box(canvas, 0, HEIGHT - height, 3, height);
            canvas_draw_box(canvas, 0, HEIGHT - 3, width, 3);
            break;
        default:
            break;
        }
        break;
    case AlignRight:
        switch(vertical) {
        case AlignTop:
            canvas_draw_rbox(canvas, WIDTH - width, 0, width, height, 3);
            canvas_draw_box(canvas, WIDTH - width, 0, width, 3); // bottom corner
            canvas_draw_box(canvas, WIDTH - 3, 0, 3, height); // right corner
            break;
        case AlignCenter:
            canvas_draw_rbox(canvas, WIDTH - width, HEIGHT / 2 - height / 2, width, height, 3);
            canvas_draw_box(canvas, WIDTH - 3, HEIGHT / 2 - height / 2, 3, height); // right corner
            break;
        case AlignBottom:
            canvas_draw_rbox(canvas, WIDTH - width, HEIGHT - height, width, height, 3);
            canvas_draw_box(canvas, WIDTH - 3, HEIGHT - height, 3, height); // right corner
            canvas_draw_box(canvas, WIDTH - width, HEIGHT - 3, width, 3); // bottom corner
            break;
        default:
            break;
        }
        break;
    case AlignCenter:
        switch(vertical) {
        case AlignTop:
            canvas_draw_rbox(canvas, WIDTH / 2 - width / 2, 0, width, height, 3);
            canvas_draw_box(canvas, WIDTH / 2 - width / 2, 0, width, 3); // bottom corner
            canvas_draw_box(canvas, WIDTH / 2 - 3, 0, 3, height); // right corner
            break;
        case AlignCenter:
            canvas_draw_rbox(
                canvas, WIDTH / 2 - width / 2, HEIGHT / 2 - height / 2, width, height, 3);
            canvas_draw_box(
                canvas, WIDTH / 2 - 3, HEIGHT / 2 - height / 2, 3, height); // right corner
            break;
        case AlignBottom:
            canvas_draw_rbox(canvas, WIDTH / 2 - width / 2, HEIGHT - height, width, height, 3);
            canvas_draw_box(canvas, WIDTH / 2 - 3, HEIGHT - height, 3, height); // right corner
            canvas_draw_box(canvas, WIDTH / 2 - width / 2, HEIGHT - 3, width, 3); // bottom corner
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}