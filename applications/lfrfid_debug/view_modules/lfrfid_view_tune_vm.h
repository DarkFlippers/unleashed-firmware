#pragma once
#include <gui/view.h>
#include <view_modules/generic_view_module.h>

class LfRfidViewTuneVM : public GenericViewModule {
public:
    LfRfidViewTuneVM();
    ~LfRfidViewTuneVM() final;
    View* get_view() final;
    void clean() final;

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
