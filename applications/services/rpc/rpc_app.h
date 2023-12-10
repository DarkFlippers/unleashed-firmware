/**
 * @file rpc_app.h
 * @brief Application RPC subsystem interface.
 *
 * The application RPC subsystem provides facilities for interacting with applications,
 * such as starting/stopping, passing parameters, sending commands and exchanging arbitrary data.
 *
 * All commands are handled asynchronously via a user-settable callback.
 *
 * For a complete description of message types handled in this subsystem,
 * see https://github.com/flipperdevices/flipperzero-protobuf/blob/dev/application.proto
 */
#pragma once

#include "rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of possible event data types.
 */
typedef enum {
    RpcAppSystemEventDataTypeNone, /**< No data is provided by the event. */
    RpcAppSystemEventDataTypeString, /**< Event data contains a zero-terminated string. */
    RpcAppSystemEventDataTypeInt32, /**< Event data contains a signed 32-bit integer. */
    RpcAppSystemEventDataTypeBytes, /**< Event data contains zero or more bytes. */
} RpcAppSystemEventDataType;

/**
 * @brief Event data structure, containing the type and associated data.
 *
 * All below fields except for type are valid only if the respective type is set.
 */
typedef struct {
    RpcAppSystemEventDataType
        type; /**< Type of the data. The meaning of other fields depends on this one. */
    union {
        const char* string; /**< Pointer to a zero-terminated character string. */
        int32_t i32; /**< Signed 32-bit integer value. */
        struct {
            const uint8_t* ptr; /**< Pointer to the byte array data. */
            size_t size; /**< Size of the byte array, in bytes. */
        } bytes; /**< Byte array of arbitrary length. */
    };
} RpcAppSystemEventData;

/**
 * @brief Enumeration of possible event types.
 */
typedef enum {
    /**
     * @brief Denotes an invalid state.
     *
     * An event of this type shall never be passed into the callback.
     */
    RpcAppEventTypeInvalid,
    /**
     * @brief The client side has closed the session.
     *
     * After receiving this event, the RPC context is no more valid.
     */
    RpcAppEventTypeSessionClose,
    /**
     * @brief The client has requested the application to exit.
     *
     * The application must exit after receiving this command.
     */
    RpcAppEventTypeAppExit,
    /**
     * @brief The client has requested the application to load a file.
     *
     * This command's meaning is application-specific, i.e. the application might or
     * might not require additional commands after loading a file to do anything useful.
     */
    RpcAppEventTypeLoadFile,
    /**
     * @brief The client has informed the application that a button has been pressed.
     *
     * This command's meaning is application-specific, e.g. to select a part of the
     * previously loaded file or to invoke a particular function within the application.
     */
    RpcAppEventTypeButtonPress,
    /**
     * @brief The client has informed the application that a button has been released.
     *
     * This command's meaning is application-specific, e.g. to cease
     * all activities to be conducted while a button is being pressed.
     */
    RpcAppEventTypeButtonRelease,
    /**
     * @brief The client has sent a byte array of arbitrary size.
     *
     * This command's purpose is bi-directional exchange of arbitrary raw data.
     * Useful for implementing higher-level protocols while using the RPC as a transport layer.
     */
    RpcAppEventTypeDataExchange,
} RpcAppSystemEventType;

/**
 * @brief RPC application subsystem event structure.
 */
typedef struct {
    RpcAppSystemEventType type; /**< Type of the event. */
    RpcAppSystemEventData data; /**< Data associated with the event. */
} RpcAppSystemEvent;

/**
 * @brief Callback function type.
 *
 * A function of this type must be passed to rpc_system_app_set_callback() by the user code.
 *
 * @warning The event pointer is valid ONLY inside the callback function.
 *
 * @param[in] event pointer to the event object. Valid only inside the callback function.
 * @param[in,out] context pointer to the user-defined context object.
 */
typedef void (*RpcAppSystemCallback)(const RpcAppSystemEvent* event, void* context);

/**
 * @brief RPC application subsystem opaque type declaration.
 */
typedef struct RpcAppSystem RpcAppSystem;

/**
 * @brief Set the callback function for use by an RpcAppSystem instance.
 *
 * @param[in,out] rpc_app pointer to the instance to be configured.
 * @param[in] callback pointer to the function to be called upon message reception.
 * @param[in,out] context pointer to the user-defined context object. Will be passed to the callback.
 */
void rpc_system_app_set_callback(
    RpcAppSystem* rpc_app,
    RpcAppSystemCallback callback,
    void* context);

/**
 * @brief Send a notification that an RpcAppSystem instance has been started and is ready.
 *
 * Call this function once right after acquiring an RPC context and setting the callback.
 *
 * @param[in,out] rpc_app pointer to the instance to be used.
 */
void rpc_system_app_send_started(RpcAppSystem* rpc_app);

/**
 * @brief Send a notification that the application using an RpcAppSystem instance is about to exit.
 *
 * Call this function when the application is about to exit (usually in the *_free() function).
 *
 * @param[in,out] rpc_app pointer to the instance to be used.
 */
void rpc_system_app_send_exited(RpcAppSystem* rpc_app);

/**
 * @brief Send a confirmation that the application using an RpcAppSystem instance has handled the event.
 *
 * An explicit confirmation is required for the following event types:
 * - RpcAppEventTypeAppExit
 * - RpcAppEventTypeLoadFile
 * - RpcAppEventTypeButtonPress
 * - RpcAppEventTypeButtonRelease
 * - RpcAppEventTypeDataExchange
 *
 * Not confirming these events will result in a client-side timeout.
 *
 * @param[in,out] rpc_app pointer to the instance to be used.
 * @param[in] result whether the command was successfully handled or not (true for success).
 */
void rpc_system_app_confirm(RpcAppSystem* rpc_app, bool result);

/**
 * @brief Set the error code stored in an RpcAppSystem instance.
 *
 * The error code can be retrieved by the client at any time by using the GetError request.
 * The error code value has no meaning within the subsystem, i.e. it is only passed through to the client.
 *
 * @param[in,out] rpc_app pointer to the instance to be modified.
 * @param[in] error_code arbitrary error code to be set.
 */
void rpc_system_app_set_error_code(RpcAppSystem* rpc_app, uint32_t error_code);

/**
 * @brief Set the error text stored in an RpcAppSystem instance.
 *
 * The error text can be retrieved by the client at any time by using the GetError request.
 * The text has no meaning within the subsystem, i.e. it is only passed through to the client.
 *
 * @param[in,out] rpc_app pointer to the instance to be modified.
 * @param[in] error_text Pointer to a zero-terminated string containing the error text.
 */
void rpc_system_app_set_error_text(RpcAppSystem* rpc_app, const char* error_text);

/**
 * @brief Reset the error code and text stored in an RpcAppSystem instance.
 *
 * Resets the error code to 0 and error text to "" (empty string).
 *
 * @param[in,out] rpc_app pointer to the instance to be reset.
 */
void rpc_system_app_error_reset(RpcAppSystem* rpc_app);

/**
 * @brief Send a byte array of arbitrary data to the client using an RpcAppSystem instance.
 *
 * @param[in,out] rpc_app pointer to the instance to be used.
 * @param[in] data pointer to the data buffer to be sent.
 * @param[in] data_size size of the data buffer, in bytes.
 */
void rpc_system_app_exchange_data(RpcAppSystem* rpc_app, const uint8_t* data, size_t data_size);

#ifdef __cplusplus
}
#endif
