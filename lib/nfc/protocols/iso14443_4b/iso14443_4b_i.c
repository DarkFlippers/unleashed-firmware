#include "iso14443_4b_i.h"

Iso14443_4bError iso14443_4b_process_error(Iso14443_3bError error) {
    switch(error) {
    case Iso14443_3bErrorNone:
        return Iso14443_4bErrorNone;
    case Iso14443_3bErrorNotPresent:
        return Iso14443_4bErrorNotPresent;
    case Iso14443_3bErrorColResFailed:
    case Iso14443_3bErrorCommunication:
    case Iso14443_3bErrorWrongCrc:
        return Iso14443_4bErrorProtocol;
    case Iso14443_3bErrorTimeout:
        return Iso14443_4bErrorTimeout;
    default:
        return Iso14443_4bErrorProtocol;
    }
}
