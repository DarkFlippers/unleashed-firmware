#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BleGlueStatusUninitialized,
    BleGlueStatusStartup,
    BleGlueStatusStarted
} BleGlueStatus;

void APPE_Init();

BleGlueStatus APPE_Status();

#ifdef __cplusplus
} /* extern "C" */
#endif
