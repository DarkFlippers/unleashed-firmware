#pragma once

typedef enum {
    // Reserve first 100 events for button types and indexes, starting from 0
    iButtonCustomEventReserved = 100,

    iButtonCustomEventBack,
    iButtonCustomEventTextEditResult,
    iButtonCustomEventByteEditChanged,
    iButtonCustomEventByteEditResult,
    iButtonCustomEventWorkerEmulated,
    iButtonCustomEventWorkerRead,
    iButtonCustomEventWorkerWriteOK,
    iButtonCustomEventWorkerWriteSameKey,
    iButtonCustomEventWorkerWriteNoDetect,
    iButtonCustomEventWorkerWriteCannotWrite,

    iButtonCustomEventRpcLoadFile,
    iButtonCustomEventRpcExit,
    iButtonCustomEventRpcSessionClose,
} iButtonCustomEvent;
