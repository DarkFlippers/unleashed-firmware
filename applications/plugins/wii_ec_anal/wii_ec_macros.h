#ifndef WII_EC_MACROS_H_
#define WII_EC_MACROS_H_

//----------------------------------------------------------------------------- ----------------------------------------
// CHECK MACROS
//
// I don't generally like this style of coding - it just (generally) makes things nightmarish to debug
// However, on this occasion I think it's a good choice (to make adding controllers LESS bug-prone)
//

//if (furi_message_queue_get_count(queue) > 18)  WARN("queue high %d", furi_message_queue_get_count(queue));
#define MSGQ(lbl)                               \
    do {                                        \
        msg.wiiEc.in = lbl;                     \
        furi_message_queue_put(queue, &msg, 0); \
    } while(0)

// A 'standard' "button" is an independent SPST switch
//   Eg. Nunchuck 'Z' button
// The "value" will always be 0
#define BUTTON(btn, lbl)                                               \
    do {                                                               \
        if(new->btn != old->btn) {                                     \
            msg.wiiEc.type = (new->btn) ? WIIEC_PRESS : WIIEC_RELEASE; \
            msg.wiiEc.val = 0;                                         \
            MSGQ(lbl);                                                 \
        }                                                              \
    } while(0)

// An "analogue button" is an SPST coupled with an ananlogue 'switch'
//    Eg. The "bottom out" switches on the triggers of the classic controller
// The "value" will be the value of the associated analogue controller
#define ANABTN(btn, ana, lbl)                                          \
    do {                                                               \
        if(new->btn != old->btn) {                                     \
            msg.wiiEc.type = (new->btn) ? WIIEC_PRESS : WIIEC_RELEASE; \
            msg.wiiEc.val = new->ana;                                  \
            MSGQ(lbl);                                                 \
        }                                                              \
    } while(0)

#define ANALOG(ana, lbl)                   \
    do {                                   \
        if(new->ana != old->ana) {         \
            msg.wiiEc.type = WIIEC_ANALOG; \
            msg.wiiEc.val = new->ana;      \
            MSGQ(lbl);                     \
        }                                  \
    } while(0)

#define ACCEL(acc, lbl)                   \
    do {                                  \
        if(new->acc != old->acc) {        \
            msg.wiiEc.type = WIIEC_ACCEL; \
            msg.wiiEc.val = new->acc;     \
            MSGQ(lbl);                    \
        }                                 \
    } while(0)

//----------------------------------------------------------------------------- ----------------------------------------
// CALIBRATION MACROS
//
// Again ...I totally agree with anyone who says "MACRO coding" is (gernally) a poor choice of programming style
// But something about this code is making it soooo appealing
//
// ... v=variable, n=number
//
#define FACTORY_LO(v, n) \
    do {                 \
        (dst[1].v) = n;  \
    } while(0)
#define FACTORY_MID(v, n) \
    do {                  \
        (dst[2].v) = n;   \
    } while(0)
#define FACTORY_HI(v, n) \
    do {                 \
        (dst[3].v) = n;  \
    } while(0)

#define TRACK_LO(v)                                      \
    do {                                                 \
        if((src->v) < (dst[0].v)) (dst[0].v) = (src->v); \
    } while(0)
#define TRACK_HI(v)                                      \
    do {                                                 \
        if((src->v) > (dst[4].v)) (dst[4].v) = (src->v); \
    } while(0)
#define TRACK_LO_HI(v) \
    do {               \
        TRACK_LO(v);   \
        TRACK_HI(v);   \
    } while(0)

#define RESET_LO(v, b)                              \
    do {                                            \
        (dst[0].v) = (dst[1].v) = ((1 << (b)) - 1); \
    } while(0)
#define RESET_HI(v)                  \
    do {                             \
        (dst[4].v) = (dst[3].v) = 0; \
    } while(0)
#define RESET_MID(v)           \
    do {                       \
        (dst[2].v) = (src->v); \
    } while(0)
#define RESET_LO_HI(v, b) \
    do {                  \
        RESET_LO(v, b);   \
        RESET_HI(v);      \
    } while(0)
#define RESET_LO_MID_HI(v, b) \
    do {                      \
        RESET_LO(v, b);       \
        RESET_MID(v);         \
        RESET_HI(v);          \
    } while(0)

#define RANGE_LO(v)                                      \
    do {                                                 \
        if((src->v) < (dst[1].v)) (dst[1].v) = (src->v); \
    } while(0)
#define RANGE_HI(v)                                      \
    do {                                                 \
        if((src->v) > (dst[3].v)) (dst[3].v) = (src->v); \
    } while(0)
#define RANGE_LO_HI(v) \
    do {               \
        RANGE_LO(v);   \
        RANGE_HI(v);   \
    } while(0)

#define CENTRE(v)              \
    do {                       \
        (dst[2].v) = (src->v); \
    } while(0)

#endif //WII_EC_MACROS_H_
