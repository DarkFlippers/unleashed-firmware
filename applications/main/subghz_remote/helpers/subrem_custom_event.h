#pragma once

typedef enum {
    // SubmenuIndex
    SubmenuIndexSubRemOpenMapFile = 0,
#if FURI_DEBUG
    SubmenuIndexSubRemRemoteView,
#endif
    // SubmenuIndexSubRemAbout,

    // SubRemCustomEvent
    SubRemCustomEventViewRemoteStartUP = 100,
    SubRemCustomEventViewRemoteStartDOWN,
    SubRemCustomEventViewRemoteStartLEFT,
    SubRemCustomEventViewRemoteStartRIGHT,
    SubRemCustomEventViewRemoteStartOK,
    SubRemCustomEventViewRemoteBack,
    SubRemCustomEventViewRemoteStop,
    SubRemCustomEventViewRemoteForcedStop,
} SubRemCustomEvent;