#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BleGlueStatusUninitialized,
    BleGlueStatusStartup,
    BleGlueStatusBroken,
    BleGlueStatusStarted
} BleGlueStatus;

void APPE_Init();

BleGlueStatus APPE_Status();

#ifdef __cplusplus
} /* extern "C" */
#endif
