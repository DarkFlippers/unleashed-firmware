#pragma once
#include "generic_element.h"

typedef void (*ButtonElementCallback)(void* context);

class ButtonElement : public GenericElement {
public:
    ButtonElement();
    ~ButtonElement() final;
    void draw(Canvas* canvas) final;
    bool input(InputEvent* event) final;

    enum class Type : uint8_t {
        Left,
        Center,
        Right,
    };

    void set_type(Type type, const char* text);
    void set_callback(void* context, ButtonElementCallback callback);

private:
    Type type = Type::Left;
    const char* text = nullptr;

    void* context = nullptr;
    ButtonElementCallback callback = nullptr;
};
