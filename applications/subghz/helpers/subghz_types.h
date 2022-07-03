#pragma once

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
    SubGhzLoadKeyStateOnlyRx,
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
    SubGhzViewIdByteInput,
    SubGhzViewIdWidget,
    SubGhzViewIdTransmitter,
    SubGhzViewIdVariableItemList,
    SubGhzViewIdFrequencyAnalyzer,
    SubGhzViewIdReadRAW,

    SubGhzViewIdStatic,
    SubGhzViewIdTestCarrier,
    SubGhzViewIdTestPacket,
} SubGhzViewId;