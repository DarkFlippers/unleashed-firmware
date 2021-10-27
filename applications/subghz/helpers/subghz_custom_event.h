#pragma once

typedef enum {
    SubghzCustomEventManagerNoSet = 0,
    SubghzCustomEventManagerSet,

    SubghzCustomEventSceneDeleteSuccess = 100,
    SubghzCustomEventSceneDelete,
    SubghzCustomEventSceneReceiverInfoTxStart,
    SubghzCustomEventSceneReceiverInfoTxStop,
    SubghzCustomEventSceneReceiverInfoSave,
    SubghzCustomEventSceneSaveName,
    SubghzCustomEventSceneSaveSuccess,
    SubghzCustomEventSceneShowError,
    SubghzCustomEventSceneShowOnlyRX,

    SubghzCustomEventSceneNeedSavingNo,
    SubghzCustomEventSceneNeedSavingYes,

    SubghzCustomEventViewReceverOK,
    SubghzCustomEventViewReceverConfig,
    SubghzCustomEventViewReceverBack,

    SubghzCustomEventViewReadRAWBack,
    SubghzCustomEventViewReadRAWIDLE,
    SubghzCustomEventViewReadRAWREC,
    SubghzCustomEventViewReadRAWConfig,
    SubghzCustomEventViewReadRAWMore,

    SubghzCustomEventViewTransmitterBack,
    SubghzCustomEventViewTransmitterSendStart,
    SubghzCustomEventViewTransmitterSendStop,
    SubghzCustomEventViewTransmitterError,
} SubghzCustomEvent;