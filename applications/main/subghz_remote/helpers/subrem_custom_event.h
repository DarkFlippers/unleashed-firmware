#pragma once

typedef enum {
    //SubmenuIndex
    SubmenuIndexSubRemOpenMapFile,
    SubmenuIndexSubRemRemoteView,
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