/**
 * @file expansion_protocol.h
 * @brief Flipper Expansion Protocol parser reference implementation.
 *
 * This file is licensed separately under The Unlicense.
 * See https://unlicense.org/ for more details.
 *
 * This parser is written with low-spec hardware in mind. It does not use
 * dynamic memory allocation or Flipper-specific libraries and can be
 * included directly into any module's firmware's sources.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default baud rate to start all communications at.
 */
#define EXPANSION_PROTOCOL_DEFAULT_BAUD_RATE (9600UL)

/**
 * @brief Maximum data size per frame, in bytes.
 */
#define EXPANSION_PROTOCOL_MAX_DATA_SIZE (64U)

/**
 * @brief Maximum allowed inactivity period, in milliseconds.
 */
#define EXPANSION_PROTOCOL_TIMEOUT_MS (250U)

/**
 * @brief Dead time after changing connection baud rate.
 */
#define EXPANSION_PROTOCOL_BAUD_CHANGE_DT_MS (25U)

/**
 * @brief Enumeration of supported frame types.
 */
typedef enum {
    ExpansionFrameTypeHeartbeat = 1, /**< Heartbeat frame. */
    ExpansionFrameTypeStatus = 2, /**< Status report frame. */
    ExpansionFrameTypeBaudRate = 3, /**< Baud rate negotiation frame. */
    ExpansionFrameTypeControl = 4, /**< Control frame. */
    ExpansionFrameTypeData = 5, /**< Data frame. */
    ExpansionFrameTypeReserved, /**< Special value. */
} ExpansionFrameType;

/**
 * @brief Enumeration of possible error types.
 */
typedef enum {
    ExpansionFrameErrorNone = 0x00, /**< No error occurred. */
    ExpansionFrameErrorUnknown = 0x01, /**< An unknown error has occurred (generic response). */
    ExpansionFrameErrorBaudRate = 0x02, /**< Requested baud rate is not supported. */
} ExpansionFrameError;

/**
 * @brief Enumeration of suported control commands.
 */
typedef enum {
    /** @brief Start an RPC session.
     *
     * Must only be used while the RPC session is NOT active.
     */
    ExpansionFrameControlCommandStartRpc = 0x00,
    /** @brief Stop an open RPC session.
      *
      * Must only be used while the RPC session IS active.
      */
    ExpansionFrameControlCommandStopRpc = 0x01,
    /** @brief Enable OTG (5V) on external GPIO.
      *
      * Must only be used while the RPC session is NOT active,
      * otherwise OTG is to be controlled via RPC messages.
      */
    ExpansionFrameControlCommandEnableOtg = 0x02,
    /** @brief Disable OTG (5V) on external GPIO.
      *
      * Must only be used while the RPC session is NOT active,
      * otherwise OTG is to be controlled via RPC messages.
      */
    ExpansionFrameControlCommandDisableOtg = 0x03,
} ExpansionFrameControlCommand;

#pragma pack(push, 1)

/**
 * @brief Frame header structure.
 */
typedef struct {
    uint8_t type; /**< Type of the frame. @see ExpansionFrameType. */
} ExpansionFrameHeader;

/**
 * @brief Heartbeat frame contents.
 */
typedef struct {
    /** Empty. */
} ExpansionFrameHeartbeat;

/**
 * @brief Status frame contents.
 */
typedef struct {
    uint8_t error; /**< Reported error code. @see ExpansionFrameError. */
} ExpansionFrameStatus;

/**
 * @brief Baud rate frame contents.
 */
typedef struct {
    uint32_t baud; /**< Requested baud rate. */
} ExpansionFrameBaudRate;

/**
 * @brief Control frame contents.
 */
typedef struct {
    uint8_t command; /**< Control command number. @see ExpansionFrameControlCommand. */
} ExpansionFrameControl;

/**
 * @brief Data frame contents.
 */
typedef struct {
    /** Size of the data. Must be less than EXPANSION_PROTOCOL_MAX_DATA_SIZE. */
    uint8_t size;
    /** Data bytes. Valid only up to ExpansionFrameData::size bytes. */
    uint8_t bytes[EXPANSION_PROTOCOL_MAX_DATA_SIZE];
} ExpansionFrameData;

/**
 * @brief Expansion protocol frame structure.
 */
typedef struct {
    ExpansionFrameHeader header; /**< Header of the frame. Required. */
    union {
        ExpansionFrameHeartbeat heartbeat; /**< Heartbeat frame contents. */
        ExpansionFrameStatus status; /**< Status frame contents. */
        ExpansionFrameBaudRate baud_rate; /**< Baud rate frame contents. */
        ExpansionFrameControl control; /**< Control frame contents. */
        ExpansionFrameData data; /**< Data frame contents. */
    } content; /**< Contents of the frame. */
} ExpansionFrame;

#pragma pack(pop)

/**
 * @brief Expansion checksum type.
 */
typedef uint8_t ExpansionFrameChecksum;

/**
 * @brief Receive function type declaration.
 *
 * @see expansion_frame_decode().
 *
 * @param[out] data pointer to the buffer to reveive the data into.
 * @param[in] data_size maximum output buffer capacity, in bytes.
 * @param[in,out] context pointer to a user-defined context object.
 * @returns number of bytes written into the output buffer.
 */
typedef size_t (*ExpansionFrameReceiveCallback)(uint8_t* data, size_t data_size, void* context);

/**
 * @brief Send function type declaration.
 *
 * @see expansion_frame_encode().
 *
 * @param[in] data pointer to the buffer containing the data to be sent.
 * @param[in] data_size size of the data to send, in bytes.
 * @param[in,out] context pointer to a user-defined context object.
 * @returns number of bytes actually sent.
 */
typedef size_t (*ExpansionFrameSendCallback)(const uint8_t* data, size_t data_size, void* context);

/**
 * @brief Get encoded frame size.
 *
 * The frame MUST be complete and properly formed.
 *
 * @param[in] frame pointer to the frame to be evaluated.
 * @returns encoded frame size, in bytes.
 */
static inline size_t expansion_frame_get_encoded_size(const ExpansionFrame* frame) {
    switch(frame->header.type) {
    case ExpansionFrameTypeHeartbeat:
        return sizeof(frame->header);
    case ExpansionFrameTypeStatus:
        return sizeof(frame->header) + sizeof(frame->content.status);
    case ExpansionFrameTypeBaudRate:
        return sizeof(frame->header) + sizeof(frame->content.baud_rate);
    case ExpansionFrameTypeControl:
        return sizeof(frame->header) + sizeof(frame->content.control);
    case ExpansionFrameTypeData:
        return sizeof(frame->header) + sizeof(frame->content.data.size) + frame->content.data.size;
    default:
        return 0;
    }
}

/**
 * @brief Get remaining number of bytes needed to properly decode a frame.
 *
 * The return value will vary depending on the received_size parameter value.
 * The frame is considered complete when the function returns 0.
 *
 * @param[in] frame pointer to the frame to be evaluated.
 * @param[in] received_size number of bytes currently availabe for evaluation.
 * @param[out] remaining_size pointer to the variable to contain the number of bytes needed for a complete frame.
 * @returns true if the remaining size could be calculated, false on error.
 */
static inline bool expansion_frame_get_remaining_size(
    const ExpansionFrame* frame,
    size_t received_size,
    size_t* remaining_size) {
    if(received_size < sizeof(ExpansionFrameHeader)) {
        // Frame type is unknown as of now
        *remaining_size = sizeof(ExpansionFrameHeader);
        return true;
    }

    const size_t received_content_size = received_size - sizeof(ExpansionFrameHeader);
    size_t content_size;

    switch(frame->header.type) {
    case ExpansionFrameTypeHeartbeat:
        content_size = 0;
        break;
    case ExpansionFrameTypeStatus:
        content_size = sizeof(frame->content.status);
        break;
    case ExpansionFrameTypeBaudRate:
        content_size = sizeof(frame->content.baud_rate);
        break;
    case ExpansionFrameTypeControl:
        content_size = sizeof(frame->content.control);
        break;
    case ExpansionFrameTypeData:
        if(received_content_size < sizeof(frame->content.data.size)) {
            // Data size is unknown as of now
            content_size = sizeof(frame->content.data.size);
        } else if(frame->content.data.size > sizeof(frame->content.data.bytes)) {
            // Malformed frame or garbage input
            return false;
        } else {
            content_size = sizeof(frame->content.data.size) + frame->content.data.size;
        }
        break;
    default:
        return false;
    }

    if(content_size > received_content_size) {
        *remaining_size = content_size - received_content_size;
    } else {
        *remaining_size = 0;
    }

    return true;
}

/**
 * @brief Enumeration of protocol parser statuses.
 */
typedef enum {
    ExpansionProtocolStatusOk, /**< No error has occurred. */
    ExpansionProtocolStatusErrorFormat, /**< Invalid frame type. */
    ExpansionProtocolStatusErrorChecksum, /**< Checksum mismatch. */
    ExpansionProtocolStatusErrorCommunication, /**< Input/output error. */
} ExpansionProtocolStatus;

/**
 * @brief Get the checksum byte corresponding to the frame
 *
 * Lightweight XOR checksum algorithm for basic error detection.
 *
 * @param[in] data pointer to a byte buffer containing the data.
 * @param[in] data_size size of the data buffer.
 * @returns checksum byte of the frame.
 */
static inline ExpansionFrameChecksum
    expansion_protocol_get_checksum(const uint8_t* data, size_t data_size) {
    ExpansionFrameChecksum checksum = 0;
    for(size_t i = 0; i < data_size; ++i) {
        checksum ^= data[i];
    }
    return checksum;
}

/**
 * @brief Receive and decode a frame.
 *
 * Will repeatedly call the receive callback function until enough data is received.
 *
 * @param[out] frame pointer to the frame to contain decoded data.
 * @param[in] receive pointer to the function used to receive data.
 * @param[in,out] context pointer to a user-defined context object. Will be passed to the receive callback function.
 * @returns ExpansionProtocolStatusOk on success, any other error code on failure.
 */
static inline ExpansionProtocolStatus expansion_protocol_decode(
    ExpansionFrame* frame,
    ExpansionFrameReceiveCallback receive,
    void* context) {
    size_t total_size = 0;
    size_t remaining_size;

    while(true) {
        if(!expansion_frame_get_remaining_size(frame, total_size, &remaining_size)) {
            return ExpansionProtocolStatusErrorFormat;
        } else if(remaining_size == 0) {
            break;
        }

        const size_t received_size =
            receive((uint8_t*)frame + total_size, remaining_size, context);

        if(received_size == 0) {
            return ExpansionProtocolStatusErrorCommunication;
        }

        total_size += received_size;
    }

    ExpansionFrameChecksum checksum;
    const size_t received_size = receive(&checksum, sizeof(checksum), context);

    if(received_size != sizeof(checksum)) {
        return ExpansionProtocolStatusErrorCommunication;
    } else if(checksum != expansion_protocol_get_checksum((const uint8_t*)frame, total_size)) {
        return ExpansionProtocolStatusErrorChecksum;
    } else {
        return ExpansionProtocolStatusOk;
    }
}

/**
 * @brief Encode and send a frame.
 *
 * @param[in] frame pointer to the frame to be encoded and sent.
 * @param[in] send pointer to the function used to send data.
 * @param[in,out] context pointer to a user-defined context object. Will be passed to the send callback function.
 * @returns ExpansionProtocolStatusOk on success, any other error code on failure.
 */
static inline ExpansionProtocolStatus expansion_protocol_encode(
    const ExpansionFrame* frame,
    ExpansionFrameSendCallback send,
    void* context) {
    const size_t encoded_size = expansion_frame_get_encoded_size(frame);
    if(encoded_size == 0) {
        return ExpansionProtocolStatusErrorFormat;
    }

    const ExpansionFrameChecksum checksum =
        expansion_protocol_get_checksum((const uint8_t*)frame, encoded_size);

    if((send((const uint8_t*)frame, encoded_size, context) != encoded_size) ||
       (send(&checksum, sizeof(checksum), context) != sizeof(checksum))) {
        return ExpansionProtocolStatusErrorCommunication;
    } else {
        return ExpansionProtocolStatusOk;
    }
}

#ifdef __cplusplus
}
#endif
