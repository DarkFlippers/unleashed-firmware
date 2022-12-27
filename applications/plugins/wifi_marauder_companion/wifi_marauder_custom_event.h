#pragma once

typedef enum {
    WifiMarauderEventRefreshConsoleOutput = 0,
    WifiMarauderEventStartConsole,
    WifiMarauderEventStartKeyboard,
    WifiMarauderEventSaveSourceMac,
    WifiMarauderEventSaveDestinationMac
} WifiMarauderCustomEvent;
