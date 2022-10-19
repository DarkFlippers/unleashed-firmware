#pragma once

typedef enum {
    //WSCustomEvent
    WSCustomEventStartId = 100,

    WSCustomEventSceneSettingLock,

    WSCustomEventViewReceiverOK,
    WSCustomEventViewReceiverConfig,
    WSCustomEventViewReceiverBack,
    WSCustomEventViewReceiverOffDisplay,
    WSCustomEventViewReceiverUnlock,
} WSCustomEvent;
