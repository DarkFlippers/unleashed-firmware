#pragma once

#include "../config/app/config.h"

typedef uint8_t Scene;

/**
 * @brief TOTP application scenes
 */
enum Scenes {
    /**
     * @brief Empty scene which does nothing 
     */
    TotpSceneNone,

    /**
     * @brief Scene where user have to enter PIN to authenticate 
     */
    TotpSceneAuthentication,

    /**
     * @brief Scene where actual TOTP token is getting generated and displayed to the user 
     */
    TotpSceneGenerateToken,

#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
    /**
     * @brief Scene where user can add new token 
     */
    TotpSceneAddNewToken,
#endif

    /**
     * @brief Scene with a menu for given token, allowing user to do multiple actions
     */
    TotpSceneTokenMenu,

    /**
     * @brief Scene where user can change application settings 
     */
    TotpSceneAppSettings,

    /**
     * @brief Scene which informs user that CLI command is running
     */
    TotpSceneStandby
};
