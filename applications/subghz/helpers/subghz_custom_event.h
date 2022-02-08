#pragma once

typedef enum {
    SubghzCustomEventManagerNoSet = 0,
    SubghzCustomEventManagerSet,
    SubghzCustomEventManagerSetRAW,

    SubghzCustomEventSceneDeleteSuccess = 100,
    SubghzCustomEventSceneDelete,
    SubghzCustomEventSceneDeleteRAW,
    SubghzCustomEventSceneDeleteRAWBack,

    SubghzCustomEventSceneReceiverInfoTxStart,
    SubghzCustomEventSceneReceiverInfoTxStop,
    SubghzCustomEventSceneReceiverInfoSave,
    SubghzCustomEventSceneSaveName,
    SubghzCustomEventSceneSaveSuccess,
    SubghzCustomEventSceneShowErrorBack,
    SubghzCustomEventSceneShowErrorOk,
    SubghzCustomEventSceneShowErrorSub,
    SubghzCustomEventSceneShowOnlyRX,

    SubghzCustomEventSceneExit,
    SubghzCustomEventSceneStay,

    SubghzCustomEventViewReceverOK,
    SubghzCustomEventViewReceverConfig,
    SubghzCustomEventViewReceverBack,

    SubghzCustomEventViewReadRAWBack,
    SubghzCustomEventViewReadRAWIDLE,
    SubghzCustomEventViewReadRAWREC,
    SubghzCustomEventViewReadRAWConfig,
    SubghzCustomEventViewReadRAWErase,
    SubghzCustomEventViewReadRAWSendStart,
    SubghzCustomEventViewReadRAWSendStop,
    SubghzCustomEventViewReadRAWSave,
    SubghzCustomEventViewReadRAWVibro,
    SubghzCustomEventViewReadRAWTXRXStop,
    SubghzCustomEventViewReadRAWMore,

    SubghzCustomEventViewTransmitterBack,
    SubghzCustomEventViewTransmitterSendStart,
    SubghzCustomEventViewTransmitterSendStop,
    SubghzCustomEventViewTransmitterError,
} SubghzCustomEvent;