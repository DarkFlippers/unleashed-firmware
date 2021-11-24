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

    SubghzCustomEventViewTransmitterBack,
    SubghzCustomEventViewTransmitterSendStart,
    SubghzCustomEventViewTransmitterSendStop,
    SubghzCustomEventViewTransmitterError,
} SubghzCustomEvent;