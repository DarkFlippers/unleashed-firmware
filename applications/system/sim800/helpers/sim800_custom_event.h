#pragma once

typedef enum {
    // Submenu items are sent as their own index, so stay clear of them
    Sim800CustomEventTextInputDone = 100,
    Sim800CustomEventButtonLeft,
    Sim800CustomEventButtonCenter,
    Sim800CustomEventButtonRight,
    Sim800CustomEventPopupBack,

    Sim800CustomEventTaskDone,

    // Unsolicited codes, posted from the UART worker
    Sim800CustomEventIncomingCall,
    Sim800CustomEventCallEnded,
    Sim800CustomEventNewSms,
    Sim800CustomEventPowerAlarm, //!< the module's own under/over-voltage warning
} Sim800CustomEvent;
