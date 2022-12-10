#pragma once

#include <gui/view.h>
#include "../helpers/pocsag_pager_types.h"
#include "../helpers/pocsag_pager_event.h"
#include <lib/flipper_format/flipper_format.h>

typedef struct PCSGReceiverInfo PCSGReceiverInfo;

void pcsg_view_receiver_info_update(PCSGReceiverInfo* pcsg_receiver_info, FlipperFormat* fff);

PCSGReceiverInfo* pcsg_view_receiver_info_alloc();

void pcsg_view_receiver_info_free(PCSGReceiverInfo* pcsg_receiver_info);

View* pcsg_view_receiver_info_get_view(PCSGReceiverInfo* pcsg_receiver_info);
