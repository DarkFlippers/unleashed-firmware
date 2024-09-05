#pragma once

/**
 * @brief Enumeration of possible error codes for application which can be started through rpc
 */
typedef enum {
    RpcAppSystemErrorCodeNone, /** There are no errors */
    RpcAppSystemErrorCodeParseFile, /** File parsing error, or wrong file structure, or missing required parameters. more accurate data can be obtained through the debug port */
    RpcAppSystemErrorCodeRegionLock, /** Requested function is blocked by regional settings */
    RpcAppSystemErrorCodeInternalParse, /** Error in protocol parameters description, or some data in opened file are unsupported */
} RpcAppSystemErrorCode;
