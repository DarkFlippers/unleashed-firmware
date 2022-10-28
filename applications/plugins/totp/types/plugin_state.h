#pragma once

#include <notification/notification.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include "../services/list/list.h"
#include "../scenes/totp_scenes_enum.h"

#define TOTP_IV_SIZE 16

typedef struct {
    Scene current_scene;
    void* current_scene_state;
    bool changing_scene;
    NotificationApp* notification;
    DialogsApp* dialogs;
    Gui* gui;

    float timezone_offset;
    ListNode* tokens_list;
    bool token_list_loaded;
    uint8_t tokens_count;

    uint8_t* crypto_verify_data;
    size_t crypto_verify_data_length;
    bool pin_set;
    uint8_t iv[TOTP_IV_SIZE];
    uint8_t base_iv[TOTP_IV_SIZE];
} PluginState;
