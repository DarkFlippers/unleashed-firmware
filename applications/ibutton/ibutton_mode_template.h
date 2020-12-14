#pragma once

// template for modes
template <class TState, class TEvents> class AppTemplateMode {
public:
    const char* name;
    virtual void event(TEvents* event, TState* state) = 0;
    virtual void render(Canvas* canvas, TState* state) = 0;
    virtual void acquire() = 0;
    virtual void release() = 0;
};
