#include <stdint.h>
#include "iso7816_t0_apdu.h"
#include "iso7816_response.h"

void iso7816_set_response(ISO7816_Response_APDU* responseAPDU, uint16_t responseCode) {
    responseAPDU->SW1 = (responseCode >> (8 * 1)) & 0xff;
    responseAPDU->SW2 = (responseCode >> (8 * 0)) & 0xff;
}
