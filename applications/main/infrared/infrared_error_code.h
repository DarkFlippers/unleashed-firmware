#pragma once

typedef enum {
    InfraredErrorCodeNone = 0,
    InfraredErrorCodeFileOperationFailed = 0x800000,
    InfraredErrorCodeWrongFileType = 0x80000100,
    InfraredErrorCodeWrongFileVersion = 0x80000200,

    //Common signal errors
    InfraredErrorCodeSignalTypeUnknown = 0x80000300,
    InfraredErrorCodeSignalNameNotFound = 0x80000400,
    InfraredErrorCodeSignalUnableToReadType = 0x80000500,
    InfraredErrorCodeSignalUnableToWriteType = 0x80000600,

    //Raw signal errors
    InfraredErrorCodeSignalRawUnableToReadFrequency = 0x80000700,
    InfraredErrorCodeSignalRawUnableToReadDutyCycle = 0x80000800,
    InfraredErrorCodeSignalRawUnableToReadTimingsSize = 0x80000900,
    InfraredErrorCodeSignalRawUnableToReadTooLongData = 0x80000A00,
    InfraredErrorCodeSignalRawUnableToReadData = 0x80000B00,

    InfraredErrorCodeSignalRawUnableToWriteFrequency = 0x80000C00,
    InfraredErrorCodeSignalRawUnableToWriteDutyCycle = 0x80000D00,
    InfraredErrorCodeSignalRawUnableToWriteData = 0x80000E00,

    //Message signal errors
    InfraredErrorCodeSignalMessageUnableToReadProtocol = 0x80000F00,
    InfraredErrorCodeSignalMessageUnableToReadAddress = 0x80001000,
    InfraredErrorCodeSignalMessageUnableToReadCommand = 0x80001100,
    InfraredErrorCodeSignalMessageIsInvalid = 0x80001200,

    InfraredErrorCodeSignalMessageUnableToWriteProtocol = 0x80001300,
    InfraredErrorCodeSignalMessageUnableToWriteAddress = 0x80001400,
    InfraredErrorCodeSignalMessageUnableToWriteCommand = 0x80001500,
} InfraredErrorCode;

#define INFRARED_ERROR_CODE_MASK  (0xFFFFFF00)
#define INFRARED_ERROR_INDEX_MASK (0x000000FF)

#define INFRARED_ERROR_GET_CODE(error)        ((error) & INFRARED_ERROR_CODE_MASK)
#define INFRARED_ERROR_GET_INDEX(error)       ((error) & INFRARED_ERROR_INDEX_MASK)
#define INFRARED_ERROR_SET_INDEX(code, index) ((code) |= ((index) & INFRARED_ERROR_INDEX_MASK))

#define INFRARED_ERROR_PRESENT(error)          (INFRARED_ERROR_GET_CODE(error) != InfraredErrorCodeNone)
#define INFRARED_ERROR_CHECK(error, test_code) (INFRARED_ERROR_GET_CODE(error) == (test_code))
