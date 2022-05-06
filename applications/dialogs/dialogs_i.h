#pragma once
#include "dialogs.h"
#include "dialogs_message.h"
#include "view_holder.h"

#ifdef __cplusplus
extern "C" {
#endif
struct DialogsApp {
    osMessageQueueId_t message_queue;
};

#ifdef __cplusplus
}
#endif
