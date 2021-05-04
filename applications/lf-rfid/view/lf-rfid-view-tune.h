#pragma once
#include <gui/view.h>

class LfRfidViewTune {
public:
    LfRfidViewTune();
    ~LfRfidViewTune();

    View* get_view();

    bool is_dirty();
    uint32_t get_ARR();
    uint32_t get_CCR();

private:
    View* view;
    void view_draw_callback(Canvas* canvas, void* _model);
    bool view_input_callback(InputEvent* event, void* context);

    void button_up();
    void button_down();
    void button_left();
    void button_right();
    void button_ok();
};
