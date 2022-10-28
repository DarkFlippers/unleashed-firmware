#pragma once

typedef enum {
    TotpSceneNone,
    TotpSceneAuthentication,
    TotpSceneGenerateToken,
    TotpSceneAddNewToken,
    TotpSceneTokenMenu,
    TotpSceneAppSettings
} Scene;
