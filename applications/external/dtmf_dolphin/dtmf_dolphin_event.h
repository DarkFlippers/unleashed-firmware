#pragma once

typedef enum {
    DTMFDolphinEventVolumeUp = 0,
    DTMFDolphinEventVolumeDown,
    DTMFDolphinDialerOkCB,
    DTMFDolphinEventStartDialer,
    DTMFDolphinEventStartBluebox,
    DTMFDolphinEventStartRedboxUS,
    DTMFDolphinEventStartRedboxUK,
    DTMFDolphinEventStartRedboxCA,
    DTMFDolphinEventStartMisc,
    DTMFDolphinEventPlayTones,
    DTMFDolphinEventStopTones,
    DTMFDolphinEventDMAHalfTransfer,
    DTMFDolphinEventDMAFullTransfer,
} DTMFDolphinEvent;

typedef struct {
    DTMFDolphinEvent type;
} DTMFDolphinCustomEvent;