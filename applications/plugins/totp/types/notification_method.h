#pragma once

typedef uint8_t NotificationMethod;

enum NotificationMethods {
    NotificationMethodNone = 0b00,
    NotificationMethodSound = 0b01,
    NotificationMethodVibro = 0b10,
};
