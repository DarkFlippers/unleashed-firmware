#pragma once

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

    /**
     * @brief Scene where user can add new token 
     */
    TotpSceneAddNewToken,

    /**
     * @brief Scene with a menu for given token, allowing user to do multiple actions
     */
    TotpSceneTokenMenu,

    /**
     * @brief Scene where user can change application settings 
     */
    TotpSceneAppSettings
};
