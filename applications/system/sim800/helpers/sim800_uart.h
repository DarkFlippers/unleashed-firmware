/**
 * @file sim800_uart.h
 * @brief Line assembler for the GPIO serial link, with AT commands on top.
 *
 * Every assembled line goes to the line callback whether or not a command is waiting for it, so
 * unsolicited codes (RING, NO CARRIER, +CMTI) arrive even mid-command.
 *
 * @warning The callback runs on the worker thread and must not call back in here.
 */
#pragma once

#include <furi.h>
#include <furi_hal_serial.h>

typedef struct Sim800Uart Sim800Uart;

typedef enum {
    Sim800AtResultOk,
    Sim800AtResultError, //!< the module answered ERROR, +CME ERROR or +CMS ERROR
    Sim800AtResultTimeout, //!< nothing final came back in time
    Sim800AtResultNotOpen, //!< the port was never acquired
} Sim800AtResult;

typedef void (*Sim800UartLineCallback)(const char* line, void* context);

Sim800Uart* sim800_uart_alloc(void);
void sim800_uart_free(Sim800Uart* uart);

/** @note Set before ::sim800_uart_open, or lines slip past between the two. */
void sim800_uart_set_line_callback(Sim800Uart* uart, Sim800UartLineCallback callback, void* ctx);

/** Acquire the port and start the worker. Releases a port already held. */
bool sim800_uart_open(Sim800Uart* uart, FuriHalSerialId serial_id, uint32_t baudrate);
void sim800_uart_close(Sim800Uart* uart);
bool sim800_uart_is_open(const Sim800Uart* uart);

void sim800_uart_tx(Sim800Uart* uart, const uint8_t* data, size_t len);
void sim800_uart_tx_str(Sim800Uart* uart, const char* str);

/** Send @p cmd (CR appended); @p response takes the lines before the final code, or NULL. */
Sim800AtResult sim800_uart_command(
    Sim800Uart* uart,
    const char* cmd,
    uint32_t timeout_ms,
    FuriString* response);

/** For commands that answer with "> " and take a Ctrl-Z terminated body, like AT+CMGS. */
/** Dial or answer. Unlike ::sim800_uart_command, BUSY and NO CARRIER end this one. */
Sim800AtResult sim800_uart_command_call(
    Sim800Uart* uart,
    const char* cmd,
    uint32_t timeout_ms,
    FuriString* response);

Sim800AtResult sim800_uart_command_prompt(
    Sim800Uart* uart,
    const char* cmd,
    const char* body,
    uint32_t timeout_ms,
    FuriString* response);

/** AT, 300 ms. True when the module answers at the configured baud rate. */
/** Send "AT" until it is answered, for autobaud lock-on. */
bool sim800_uart_sync(Sim800Uart* uart, size_t attempts);

bool sim800_uart_probe(Sim800Uart* uart);

/** Payload of the first "<prefix>: <payload>" line, or false with @p out untouched. */
bool sim800_response_field(const FuriString* response, const char* prefix, FuriString* out);

/** Strip surrounding quotes in place. */
void sim800_unquote(FuriString* str);

/** Pick field @p index (0 based, comma separated) out of @p list into @p out. */
bool sim800_csv_field(const char* list, size_t index, FuriString* out);
