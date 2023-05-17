#pragma once

typedef enum {
    // SubRemCustomEventManagerNoSet = 0,
    // SubRemCustomEventManagerSet,
    // SubRemCustomEventManagerSetRAW,

    //SubmenuIndex
    SubmenuIndexSubRemOpenMapFile,
    SubmenuIndexSubRemRemoteView, // TODO: temp debug
    SubmenuIndexSubRemAbout,

    //SubRemCustomEvent
    SubRemCustomEventViewRemoteStartUP = 100,
    SubRemCustomEventViewRemoteStartDOWN,
    SubRemCustomEventViewRemoteStartLEFT,
    SubRemCustomEventViewRemoteStartRIGHT,
    SubRemCustomEventViewRemoteStartOK,
    SubRemCustomEventViewRemoteBack,
    SubRemCustomEventViewRemoteStop,
    SubRemCustomEventViewRemoteForcedStop,
} SubRemCustomEvent;