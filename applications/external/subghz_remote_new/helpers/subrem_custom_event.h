#pragma once

typedef enum {
    // SubRemCustomEventManagerNoSet = 0,
    // SubRemCustomEventManagerSet,
    // SubRemCustomEventManagerSetRAW,

    //SubmenuIndex
    SubmenuIndexOpenMapFile,
    SubmenuIndexOpenView, // TODO: temp debug

    //SubRemCustomEvent
    SubRemCustomEventViewRemoteBack = 100,
    SubRemCustomEventViewRemoteStartUP,
    SubRemCustomEventViewRemoteStartDOWN,
    SubRemCustomEventViewRemoteStartLEFT,
    SubRemCustomEventViewRemoteStartRIGHT,
    SubRemCustomEventViewRemoteStartOK,
    SubRemCustomEventViewRemoteStop,
    SubRemCustomEventViewRemoteForceStop,

    // SubRemCustomEventSceneDeleteSuccess = 100,
    // SubRemCustomEventSceneDelete,
    // SubRemCustomEventSceneDeleteRAW,
    // SubRemCustomEventSceneDeleteRAWBack,

    // SubRemCustomEventSceneReceiverInfoTxStart,
    // SubRemCustomEventSceneReceiverInfoTxStop,
    // SubRemCustomEventSceneReceiverInfoSave,
    // SubRemCustomEventSceneSaveName,
    // SubRemCustomEventSceneSaveSuccess,
    // SubRemCustomEventSceneShowErrorBack,
    // SubRemCustomEventSceneShowErrorOk,
    // SubRemCustomEventSceneShowErrorSub,
    // SubRemCustomEventSceneShowOnlyRX,
    // SubRemCustomEventSceneAnalyzerLock,
    // SubRemCustomEventSceneAnalyzerUnlock,
    // SubRemCustomEventSceneSettingLock,

    // SubRemCustomEventSceneExit,
    // SubRemCustomEventSceneStay,

    // SubRemCustomEventSceneRpcLoad,
    // SubRemCustomEventSceneRpcButtonPress,
    // SubRemCustomEventSceneRpcButtonRelease,
    // SubRemCustomEventSceneRpcSessionClose,

    // SubRemCustomEventViewReceiverOK,
    // SubRemCustomEventViewReceiverConfig,
    // SubRemCustomEventViewReceiverBack,
    // SubRemCustomEventViewReceiverOffDisplay,
    // SubRemCustomEventViewReceiverUnlock,
    // SubRemCustomEventViewReceiverDeleteItem,

    // SubRemCustomEventViewReadRAWBack,
    // SubRemCustomEventViewReadRAWIDLE,
    // SubRemCustomEventViewReadRAWREC,
    // SubRemCustomEventViewReadRAWConfig,
    // SubRemCustomEventViewReadRAWErase,
    // SubRemCustomEventViewReadRAWSendStart,
    // SubRemCustomEventViewReadRAWSendStop,
    // SubRemCustomEventViewReadRAWSave,
    // SubRemCustomEventViewReadRAWTXRXStop,
    // SubRemCustomEventViewReadRAWMore,

    // SubRemCustomEventViewTransmitterBack,
    // SubRemCustomEventViewTransmitterSendStart,
    // SubRemCustomEventViewTransmitterSendStop,
    // SubRemCustomEventViewTransmitterError,

    // SubRemCustomEventViewFreqAnalOkShort,
    // SubRemCustomEventViewFreqAnalOkLong,

    // SubRemCustomEventByteInputDone,
} SubRemCustomEvent;
