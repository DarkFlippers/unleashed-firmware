#pragma once
#include <gui/gui.h>
#include <gui/view.h>

class GenericElement {
public:
    GenericElement(){};
    virtual ~GenericElement(){};
    virtual void draw(Canvas* canvas) = 0;
    virtual bool input(InputEvent* event) = 0;

    // TODO that must be accessible only to ContainerVMData
    void set_parent_view(View* view);

    // TODO that must be accessible only to inheritors
    void lock_model();
    void unlock_model(bool need_redraw);

private:
    View* view = nullptr;
};
