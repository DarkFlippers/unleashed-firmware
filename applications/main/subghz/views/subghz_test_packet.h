#pragma once

#include <gui/view.h>

typedef enum {
    SubGhzTestPacketEventOnlyRx,
} SubGhzTestPacketEvent;

typedef struct SubGhzTestPacket SubGhzTestPacket;

typedef void (*SubGhzTestPacketCallback)(SubGhzTestPacketEvent event, void* context);

void subghz_test_packet_set_callback(
    SubGhzTestPacket* subghz_test_packet,
    SubGhzTestPacketCallback callback,
    void* context);

SubGhzTestPacket* subghz_test_packet_alloc();

void subghz_test_packet_free(SubGhzTestPacket* subghz_test_packet);

View* subghz_test_packet_get_view(SubGhzTestPacket* subghz_test_packet);
