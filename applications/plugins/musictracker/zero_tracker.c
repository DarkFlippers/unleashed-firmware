#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <notification/notification_messages.h>
#include "zero_tracker.h"
#include "tracker_engine/tracker.h"
#include "view/tracker_view.h"

// Channel p_0_channels[] = {
//     {
//         .rows =
//             {
//                 // 1/4
//                 ROW_MAKE(NOTE_C3, EffectArpeggio, EFFECT_DATA_2(4, 7)),
//                 ROW_MAKE(0, EffectArpeggio, EFFECT_DATA_2(4, 7)),
//                 ROW_MAKE(NOTE_C4, EffectSlideToNote, 0x20),
//                 ROW_MAKE(0, EffectSlideToNote, 0x20),
//                 //
//                 ROW_MAKE(0, EffectSlideToNote, 0x20),
//                 ROW_MAKE(0, EffectSlideToNote, 0x20),
//                 ROW_MAKE(0, EffectSlideToNote, 0x20),
//                 ROW_MAKE(0, EffectSlideToNote, 0x20),
//                 //
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
//                 //
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
//                 // 2/4
//                 ROW_MAKE(NOTE_C3, EffectSlideDown, 0x20),
//                 ROW_MAKE(0, EffectSlideDown, 0x20),
//                 ROW_MAKE(NOTE_C4, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 //
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 //
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 //
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 ROW_MAKE(NOTE_OFF, EffectVibrato, EFFECT_DATA_2(3, 3)),
//                 // 3/4
//                 ROW_MAKE(NOTE_C3, EffectArpeggio, EFFECT_DATA_2(4, 7)),
//                 ROW_MAKE(0, EffectArpeggio, EFFECT_DATA_2(4, 7)),
//                 ROW_MAKE(NOTE_OFF, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 //
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 //
//                 ROW_MAKE(NOTE_C2, EffectPWM, 60),
//                 ROW_MAKE(0, EffectPWM, 32),
//                 ROW_MAKE(0, EffectPWM, 12),
//                 ROW_MAKE(NOTE_OFF, 0, 0),
//                 //
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 // 4/4
//                 ROW_MAKE(NOTE_C3, EffectSlideDown, 0x20),
//                 ROW_MAKE(0, EffectSlideDown, 0x20),
//                 ROW_MAKE(0, EffectSlideDown, 0x20),
//                 ROW_MAKE(NOTE_OFF, 0, 0),
//                 //
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 //
//                 ROW_MAKE(NOTE_C2, EffectPWM, 60),
//                 ROW_MAKE(0, EffectPWM, 32),
//                 ROW_MAKE(0, EffectPWM, 12),
//                 ROW_MAKE(NOTE_OFF, 0, 0),
//                 //
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//                 ROW_MAKE(0, 0, 0),
//             },
//     },
// };

Channel p_0_channels[] = {
    {
        .rows =
            {
                //
                ROW_MAKE(NOTE_A4, EffectArpeggio, EFFECT_DATA_2(4, 7)),
                ROW_MAKE(NOTE_C3, 0, 0),
                ROW_MAKE(NOTE_F2, 0, 0),
                ROW_MAKE(NOTE_C3, 0, 0),
                //
                ROW_MAKE(NOTE_E4, 0, 0),
                ROW_MAKE(NOTE_C3, 0, 0),
                ROW_MAKE(NOTE_E4, EffectPWM, 50),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_E5, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_C3, EffectSlideDown, 0x30),
                ROW_MAKE(NOTE_F2, 0, 0),
                ROW_MAKE(NOTE_C3, 0, 0),
                //
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_C3, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                //
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_B4, EffectArpeggio, EFFECT_DATA_2(4, 7)),
                ROW_MAKE(NOTE_D3, 0, 0),
                ROW_MAKE(NOTE_G2, 0, 0),
                ROW_MAKE(NOTE_D3, 0, 0),
                //
                ROW_MAKE(NOTE_E4, 0, 0),
                ROW_MAKE(NOTE_D3, 0, 0),
                ROW_MAKE(NOTE_E4, EffectPWM, 50),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_E5, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_D3, EffectSlideDown, 0x3F),
                ROW_MAKE(NOTE_G2, 0, 0),
                ROW_MAKE(NOTE_D3, 0, 0),
                //
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_D3, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                //
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(NOTE_OFF, 0, 0),
            },
    },
};

Channel p_1_channels[] = {
    {
        .rows =
            {
                //
                ROW_MAKE(NOTE_C5, EffectArpeggio, EFFECT_DATA_2(4, 7)),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 50),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_G4, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_C6, 0, 0),
                ROW_MAKE(NOTE_E3, EffectSlideDown, 0x30),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 50),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_G4, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                //
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_C5, EffectArpeggio, EFFECT_DATA_2(4, 7)),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 50),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_G4, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(0, EffectPWM, 55),
                ROW_MAKE(0, EffectPWM, 45),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_C6, 0, 0),
                ROW_MAKE(NOTE_E3, EffectSlideDown, 0x30),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 50),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_G4, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, 0, 0),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                //
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(1, 1)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(0, EffectVibrato, EFFECT_DATA_2(2, 2)),
                ROW_MAKE(NOTE_OFF, 0, 0),
            },
    },
};

Channel p_2_channels[] = {
    {
        .rows =
            {
                //
                ROW_MAKE(NOTE_C5, EffectArpeggio, EFFECT_DATA_2(4, 7)),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_A4, 0, 0),
                //
                ROW_MAKE(NOTE_C5, EffectPWM, 55),
                ROW_MAKE(NOTE_A4, EffectPWM, 45),
                ROW_MAKE(NOTE_C5, EffectPWM, 35),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(NOTE_C5, EffectPWM, 55),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_E3, EffectSlideDown, 0x30),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_OFF, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 55),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_D5, EffectPWM, 55),
                ROW_MAKE(NOTE_B4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_D5, EffectPWM, 45),
                ROW_MAKE(NOTE_B4, EffectPWM, 45),
                ROW_MAKE(NOTE_D5, EffectPWM, 35),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, EffectArpeggio, EFFECT_DATA_2(4, 7)),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_E5, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_E5, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                //
                ROW_MAKE(NOTE_E5, EffectPWM, 55),
                ROW_MAKE(NOTE_C5, EffectPWM, 45),
                ROW_MAKE(NOTE_E5, EffectPWM, 35),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_E5, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_E5, EffectPWM, 55),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_E3, EffectSlideDown, 0x30),
                ROW_MAKE(NOTE_A2, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                //
                ROW_MAKE(NOTE_OFF, 0, 0),
                ROW_MAKE(NOTE_E3, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 55),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_D5, EffectPWM, 55),
                ROW_MAKE(NOTE_B4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_D5, EffectPWM, 45),
                ROW_MAKE(NOTE_B4, EffectPWM, 45),
                ROW_MAKE(NOTE_D5, EffectPWM, 35),
                ROW_MAKE(NOTE_OFF, 0, 0),
            },
    },
};

Channel p_3_channels[] = {
    {
        .rows =
            {
                //
                ROW_MAKE(NOTE_Ds5, EffectArpeggio, EFFECT_DATA_2(4, 6)),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_Ds5, 0, 0),
                ROW_MAKE(NOTE_C5, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_Ds5, EffectPWM, 45),
                ROW_MAKE(NOTE_C5, EffectPWM, 35),
                ROW_MAKE(NOTE_Ds5, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_D5, EffectPWM, 45),
                ROW_MAKE(NOTE_B4, EffectPWM, 35),
                ROW_MAKE(NOTE_D5, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_Cs5, EffectArpeggio, EFFECT_DATA_2(4, 6)),
                ROW_MAKE(NOTE_As4, 0, 0),
                ROW_MAKE(NOTE_Cs5, 0, 0),
                ROW_MAKE(NOTE_As4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_Cs5, EffectPWM, 45),
                ROW_MAKE(NOTE_As4, EffectPWM, 35),
                ROW_MAKE(NOTE_Cs5, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_A4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_C5, EffectPWM, 45),
                ROW_MAKE(NOTE_A4, EffectPWM, 35),
                ROW_MAKE(NOTE_C5, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_B4, EffectArpeggio, EFFECT_DATA_2(4, 6)),
                ROW_MAKE(NOTE_Gs4, 0, 0),
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_Gs4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_B4, EffectPWM, 45),
                ROW_MAKE(NOTE_Gs4, EffectPWM, 35),
                ROW_MAKE(NOTE_B4, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_A4, 0, 0),
                ROW_MAKE(NOTE_C5, 0, 0),
                ROW_MAKE(NOTE_A4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_C5, EffectPWM, 45),
                ROW_MAKE(NOTE_A4, EffectPWM, 35),
                ROW_MAKE(NOTE_C5, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_Cs5, EffectArpeggio, EFFECT_DATA_2(4, 6)),
                ROW_MAKE(NOTE_As4, 0, 0),
                ROW_MAKE(NOTE_Cs5, 0, 0),
                ROW_MAKE(NOTE_As4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_Cs5, EffectPWM, 45),
                ROW_MAKE(NOTE_As4, EffectPWM, 35),
                ROW_MAKE(NOTE_Cs5, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
                //
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_B4, 0, 0),
                ROW_MAKE(NOTE_D5, 0, 0),
                ROW_MAKE(NOTE_B4, EffectPWM, 55),
                //
                ROW_MAKE(NOTE_D5, EffectPWM, 45),
                ROW_MAKE(NOTE_B4, EffectPWM, 35),
                ROW_MAKE(NOTE_D5, EffectPWM, 30),
                ROW_MAKE(NOTE_OFF, 0, 0),
            },
    },
};
Pattern patterns[] = {
    {.channels = p_0_channels},
    {.channels = p_1_channels},
    {.channels = p_2_channels},
    {.channels = p_3_channels},
};

uint8_t order_list[] = {
    0,
    1,
    0,
    2,
    0,
    1,
    0,
    3,
};

Song song = {
    .channels_count = 1,
    .patterns_count = sizeof(patterns) / sizeof(patterns[0]),
    .patterns = patterns,

    .order_list_size = sizeof(order_list) / sizeof(order_list[0]),
    .order_list = order_list,

    .ticks_per_second = 60,
};

void tracker_message(TrackerMessage message, void* context) {
    FuriMessageQueue* queue = context;
    furi_assert(queue);
    furi_message_queue_put(queue, &message, 0);
}

int32_t zero_tracker_app(void* p) {
    UNUSED(p);

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_display_backlight_enforce_on);

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewDispatcher* view_dispatcher = view_dispatcher_alloc();
    TrackerView* tracker_view = tracker_view_alloc();
    tracker_view_set_song(tracker_view, &song);
    view_dispatcher_add_view(view_dispatcher, 0, tracker_view_get_view(tracker_view));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(view_dispatcher, 0);

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(TrackerMessage));
    Tracker* tracker = tracker_alloc();
    tracker_set_message_callback(tracker, tracker_message, queue);
    tracker_set_song(tracker, &song);
    tracker_start(tracker);

    while(1) {
        TrackerMessage message;
        FuriStatus status = furi_message_queue_get(queue, &message, portMAX_DELAY);
        if(status == FuriStatusOk) {
            if(message.type == TrackerPositionChanged) {
                uint8_t order_list_index = message.data.position.order_list_index;
                uint8_t row = message.data.position.row;
                uint8_t pattern = song.order_list[order_list_index];
                tracker_view_set_position(tracker_view, order_list_index, row);
                FURI_LOG_I("Tracker", "O:%d P:%d R:%d", order_list_index, pattern, row);
            } else if(message.type == TrackerEndOfSong) {
                FURI_LOG_I("Tracker", "End of song");
                break;
            }
        }
    }

    tracker_stop(tracker);
    tracker_free(tracker);
    furi_message_queue_free(queue);

    furi_delay_ms(500);

    view_dispatcher_remove_view(view_dispatcher, 0);
    tracker_view_free(tracker_view);
    view_dispatcher_free(view_dispatcher);

    notification_message(notification, &sequence_display_backlight_enforce_auto);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    return 0;
}