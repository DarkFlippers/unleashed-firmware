#pragma once
#include <gui/view.h>

class GenericViewModule {
public:
    GenericViewModule(){};
    virtual ~GenericViewModule(){};
    virtual View* get_view() = 0;
    virtual void clean() = 0;
};
