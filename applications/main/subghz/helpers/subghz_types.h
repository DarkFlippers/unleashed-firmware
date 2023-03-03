#pragma once

#include <furi.h>
#include <furi_hal.h>

/** SubGhzNotification state */
typedef enum {
    SubGhzNotificationStateStarting,
    SubGhzNotificationStateIDLE,
    SubGhzNotificationStateTx,
    SubGhzNotificationStateRx,
    SubGhzNotificationStateRxDone,
} SubGhzNotificationState;

/** SubGhzTxRx state */
typedef enum {
    SubGhzTxRxStateIDLE,
    SubGhzTxRxStateRx,
    SubGhzTxRxStateTx,
    SubGhzTxRxStateSleep,
} SubGhzTxRxState;

/** SubGhzHopperState state */
typedef enum {
    SubGhzHopperStateOFF,
    SubGhzHopperStateRunnig,
    SubGhzHopperStatePause,
    SubGhzHopperStateRSSITimeOut,
} SubGhzHopperState;

/** SubGhzSpeakerState state */
typedef enum {
    SubGhzSpeakerStateDisable,
    SubGhzSpeakerStateShutdown,
    SubGhzSpeakerStateEnable,
} SubGhzSpeakerState;

/** SubGhzRxKeyState state */
typedef enum {
    SubGhzRxKeyStateIDLE,
    SubGhzRxKeyStateNoSave,
    SubGhzRxKeyStateNeedSave,
    SubGhzRxKeyStateBack,
    SubGhzRxKeyStateStart,
    SubGhzRxKeyStateAddKey,
    SubGhzRxKeyStateExit,
    SubGhzRxKeyStateRAWLoad,
    SubGhzRxKeyStateRAWSave,
} SubGhzRxKeyState;

/** SubGhzLoadKeyState state */
typedef enum {
    SubGhzLoadKeyStateUnknown,
    SubGhzLoadKeyStateOK,
    SubGhzLoadKeyStateParseErr,
    SubGhzLoadKeyStateProtocolDescriptionErr,
} SubGhzLoadKeyState;

/** SubGhzLock */
typedef enum {
    SubGhzLockOff,
    SubGhzLockOn,
} SubGhzLock;

typedef enum {
    SubGhzViewIdMenu,
    SubGhzViewIdReceiver,
    SubGhzViewIdPopup,
    SubGhzViewIdTextInput,
    SubGhzViewIdWidget,
    SubGhzViewIdTransmitter,
    SubGhzViewIdVariableItemList,
    SubGhzViewIdFrequencyAnalyzer,
    SubGhzViewIdReadRAW,

    SubGhzViewIdStatic,
    SubGhzViewIdTestCarrier,
    SubGhzViewIdTestPacket,
} SubGhzViewId;
