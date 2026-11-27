#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Apple Media Service client: track info and media control from a paired iPhone.
 * Needs the Full BLE stack. The client starts on its own when a link is encrypted.
 */

typedef enum {
    AmsStateIdle,
    AmsStateDiscService,
    AmsStateDiscChars,
    AmsStateDiscDesc,
    AmsStateSubscribe,
    AmsStateRequestTrack,
    AmsStateRequestPlayer,
    AmsStateRequestQueue,
    AmsStateReady,
    AmsStateCommand,
    AmsStateNotFound,
    AmsStateError,
    AmsStateUnsupported, /**< Radio stack has no GATT client (Light stack) */
} AmsState;

typedef enum {
    AmsCommandPlay = 0,
    AmsCommandPause = 1,
    AmsCommandTogglePlayPause = 2,
    AmsCommandNextTrack = 3,
    AmsCommandPreviousTrack = 4,
    AmsCommandVolumeUp = 5,
    AmsCommandVolumeDown = 6,
} AmsCommand;

typedef struct {
    AmsState state;
    uint8_t error;
    uint16_t updates;
    char title[64];
    char artist[64];
    char player[32];
    char playback[32];
    uint16_t queue_index;
    uint16_t queue_count;
} AmsInfo;

/** Copy the latest state, track and player info */
void ams_client_get(AmsInfo* info);

/** Send a remote command to the phone's active media app. False if not ready */
bool ams_client_command(AmsCommand command);

const char* ams_client_state_name(AmsState state);

#ifdef __cplusplus
}
#endif
