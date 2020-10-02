#ifndef __INPUT_H
#define __INPUT_H

#include <stdbool.h>

#define INPUT_COUNT 7

typedef enum {
    InputUp = 0,
    InputDown,
    InputRight,
    InputLeft,
    InputOk,
    InputBack,
    InputCharging,
} Input;

typedef struct {
    Input input;
    bool state;
} InputEvent;

typedef struct {
    bool up : 1;
    bool down : 1;
    bool right : 1;
    bool left : 1;
    bool ok : 1;
    bool back : 1;
    bool charging : 1;
} __attribute__((packed)) InputState;

#define _BITS2STATE(bits)                                                                        \
    {                                                                                            \
        .up = (((bits)&0x01) != 0), .down = (((bits)&0x02) != 0), .right = (((bits)&0x04) != 0), \
        .left = (((bits)&0x08) != 0), .ok = (((bits)&0x10) != 0), .back = (((bits)&0x20) != 0),  \
        .charging = (((bits)&0x40) != 0)                                                         \
    }

#endif /* __INPUT_H */
