#pragma once

enum iButtonCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    iButtonCustomEventReserved = 100,

    iButtonCustomEventBack,
    iButtonCustomEventTextEditResult,
    iButtonCustomEventByteEditChanged,
    iButtonCustomEventByteEditResult,
    iButtonCustomEventWorkerEmulated,
    iButtonCustomEventWorkerRead,

    iButtonCustomEventRpcLoad,
    iButtonCustomEventRpcExit,
    iButtonCustomEventRpcSessionClose,
};
