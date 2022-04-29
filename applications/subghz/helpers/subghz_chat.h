#pragma once
#include "../subghz_i.h"
#include <cli/cli.h>

typedef struct SubGhzChatWorker SubGhzChatWorker;

typedef enum {
    SubGhzChatEventNoEvent,
    SubGhzChatEventUserEntrance,
    SubGhzChatEventUserExit,
    SubGhzChatEventInputData,
    SubGhzChatEventRXData,
    SubGhzChatEventNewMessage,
} SubGhzChatEventType;

typedef struct {
    SubGhzChatEventType event;
    char c;
} SubGhzChatEvent;

SubGhzChatWorker* subghz_chat_worker_alloc(Cli* cli);
void subghz_chat_worker_free(SubGhzChatWorker* instance);
bool subghz_chat_worker_start(SubGhzChatWorker* instance, uint32_t frequency);
void subghz_chat_worker_stop(SubGhzChatWorker* instance);
bool subghz_chat_worker_is_running(SubGhzChatWorker* instance);
SubGhzChatEvent subghz_chat_worker_get_event_chat(SubGhzChatWorker* instance);
void subghz_chat_worker_put_event_chat(SubGhzChatWorker* instance, SubGhzChatEvent* event);
size_t subghz_chat_worker_available(SubGhzChatWorker* instance);
size_t subghz_chat_worker_read(SubGhzChatWorker* instance, uint8_t* data, size_t size);
bool subghz_chat_worker_write(SubGhzChatWorker* instance, uint8_t* data, size_t size);
