#pragma once
#include "dialogs.h"
#include "dialogs-message.h"
#include "view-holder.h"

#ifdef __cplusplus
extern "C" {
#endif
struct DialogsApp {
    osMessageQueueId_t message_queue;
};

#ifdef __cplusplus
}
#endif