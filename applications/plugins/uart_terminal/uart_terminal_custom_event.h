#pragma once

typedef enum {
    UART_TerminalEventRefreshConsoleOutput = 0,
    UART_TerminalEventStartConsole,
    UART_TerminalEventStartKeyboard,
} UART_TerminalCustomEvent;
