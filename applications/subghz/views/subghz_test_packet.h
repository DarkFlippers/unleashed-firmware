#pragma once

#include <gui/view.h>

typedef enum {
    SubghzTestPacketEventOnlyRx,
} SubghzTestPacketEvent;

typedef struct SubghzTestPacket SubghzTestPacket;

typedef void (*SubghzTestPacketCallback)(SubghzTestPacketEvent event, void* context);

void subghz_test_packet_set_callback(
    SubghzTestPacket* subghz_test_packet,
    SubghzTestPacketCallback callback,
    void* context);

SubghzTestPacket* subghz_test_packet_alloc();

void subghz_test_packet_free(SubghzTestPacket* subghz_test_packet);

View* subghz_test_packet_get_view(SubghzTestPacket* subghz_test_packet);
