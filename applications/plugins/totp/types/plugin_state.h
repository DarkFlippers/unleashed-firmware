#pragma once

#include <notification/notification.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include "../services/list/list.h"
#include "../scenes/totp_scenes_enum.h"

#define TOTP_IV_SIZE 16

/**
 * @brief Application state structure
 */
typedef struct {
    /**
     * @brief Application current scene
     */
    Scene current_scene;

    /**
     * @brief Application current scene state
     */
    void* current_scene_state;

    /**
     * @brief Whether scene is changing now 
     */
    bool changing_scene;

    /**
     * @brief Reference to the firmware notification subsystem
     */
    NotificationApp* notification;

    /**
     * @brief Reference to the firmware dialogs subsystem 
     */
    DialogsApp* dialogs;

    /**
     * @brief Reference to the firmware GUI subsystem
     */
    Gui* gui;

    /**
     * @brief Timezone UTC offset in hours 
     */
    float timezone_offset;

    /**
     * @brief Token list head node 
     */
    ListNode* tokens_list;

    /**
     * @brief Whether token list is loaded or not 
     */
    bool token_list_loaded;

    /**
     * @brief Tokens list length 
     */
    uint16_t tokens_count;

    /**
     * @brief Encrypted well-known string data
     */
    uint8_t* crypto_verify_data;

    /**
     * @brief Encrypted well-known string data length
     */
    size_t crypto_verify_data_length;

    /**
     * @brief Whether PIN is set by user or not 
     */
    bool pin_set;

    /**
     * @brief Initialization vector (IV) to be used for encryption\decryption 
     */
    uint8_t iv[TOTP_IV_SIZE];

    /**
     * @brief Basic randomly-generated initialization vector (IV)
     */
    uint8_t base_iv[TOTP_IV_SIZE];
} PluginState;
