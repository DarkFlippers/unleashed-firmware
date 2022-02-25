/**
  * @file infrared_app_event.h
  * Infrared: Scene events description
  */
#pragma once
#include <infrared.h>
#include <gui/modules/dialog_ex.h>

/** Infrared events class */
class InfraredAppEvent {
public:
    /** Type of event enum */
    enum class Type : uint8_t {
        /** Tick event come after no other events came in 100 ms */
        Tick,
        /** Exit application event */
        Exit,
        /** Back event */
        Back,
        /** Menu selected event type. Provided with payload value. */
        MenuSelected,
        /** Button press event. Need for continuous signal sending. */
        MenuSelectedPress,
        /** Button release event. Need for continuous signal sending. */
        MenuSelectedRelease,
        /** Events from DialogEx view module */
        DialogExSelected,
        /** Infrared signal received event */
        InfraredMessageReceived,
        /** Text edit done event */
        TextEditDone,
        /** Popup timer finished event */
        PopupTimer,
        /** Button panel pressed event */
        ButtonPanelPressed,
    };

    union {
        /** Menu selected event type payload. Selected index. */
        int32_t menu_index;
        /** DialogEx view module event type payload */
        DialogExResult dialog_ex_result;
    } payload;

    /** Type of event */
    Type type;
};
