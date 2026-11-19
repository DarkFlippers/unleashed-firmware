#pragma once

#include "sim800_uart.h"

/** AT+CSQ units, 0..31 or 99 for none. */
uint8_t sim800_at_rssi(Sim800Uart* uart);

/** AT+COPS? carrier name, "--" when off network. */
void sim800_at_operator(Sim800Uart* uart, FuriString* out);

/** AT+CREG? as words. */
void sim800_at_registration(Sim800Uart* uart, FuriString* out);

/** AT+CPIN? as words. */
void sim800_at_sim_status(Sim800Uart* uart, FuriString* out);

/** AT+CBC as "4.05 V 87%". */
void sim800_at_power(Sim800Uart* uart, FuriString* out);

/** Run @p cmd, return the payload after @p prefix. */
bool sim800_at_query(Sim800Uart* uart, const char* cmd, const char* prefix, FuriString* out);

/** Run @p cmd, return its first response line (AT+GSN, AT+CGMM, ...). */
bool sim800_at_line(Sim800Uart* uart, const char* cmd, FuriString* out);
