#pragma once
#include <view-modules/generic-view-module.h>

class ContainerVM : public GenericViewModule {
public:
    ContainerVM();
    ~ContainerVM() final;
    View* get_view() final;
    void clean() final;

    template <typename T> T* add();

private:
    View* view;
    static void view_draw_callback(Canvas* canvas, void* model);
    static bool view_input_callback(InputEvent* event, void* context);
};