#pragma once

#include <gui/view.h>

typedef struct SubghzTestPacket SubghzTestPacket;

SubghzTestPacket* subghz_test_packet_alloc();

void subghz_test_packet_free(SubghzTestPacket* subghz_test_packet);

View* subghz_test_packet_get_view(SubghzTestPacket* subghz_test_packet);
