#pragma once

#include <notification/notification.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include "../config/app/config.h"
#include "../ui/totp_scenes_enum.h"
#include "../services/config/config_file_context.h"
#include "../services/idle_timeout/idle_timeout.h"
#include "notification_method.h"
#include "automation_method.h"
#include "automation_kb_layout.h"
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
#include "../workers/bt_type_code/bt_type_code.h"
#endif
#include "crypto_settings.h"

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
     * @brief Reference to the firmware dialogs subsystem 
     */
    DialogsApp* dialogs_app;

    /**
     * @brief Reference to the firmware GUI subsystem
     */
    Gui* gui;

    /**
     * @brief Timezone UTC offset in hours 
     */
    float timezone_offset;

    /**
     * @brief Config file context
     */
    ConfigFileContext* config_file_context;

    /**
     * @brief Notification method
     */
    NotificationMethod notification_method;

    /**
     * @brief Automation method
     */
    AutomationMethod automation_method;

    /**
     * @brief Automation keyboard layout to be used
     */
    AutomationKeyboardLayout automation_kb_layout;

#ifdef TOTP_BADBT_AUTOMATION_ENABLED
    /**
     * @brief Bad-Bluetooth worker context
     */
    TotpBtTypeCodeWorkerContext* bt_type_code_worker_context;
#endif

    /**
     * @brief IDLE timeout context
     */
    IdleTimeoutContext* idle_timeout_context;

    /**
     * @brief Font index to be used to draw TOTP token
     */
    uint8_t active_font_index;

    /**
     * @brief Application even queue
     */
    FuriMessageQueue* event_queue;

    /**
     * @brief Crypto settings
     */
    CryptoSettings crypto_settings;
} PluginState;
