#include "iso7816.h"

// ISO7816-5
// Simple-TLV (§5.2.1)
// BER-TLV (§5.2.2)
TlvInfo iso7816_tlv_parse(const uint8_t* data) {
    TlvInfo tlv;

    // Simple-TLV: tag can be any value from 1 to 254 (not '00' or 'FF')
    // BER-TLV: TODO describe
    // 00000 - 11110 => 0 - 30 (single byte)
    // 11111 00011111 - 11111 01111111 => 31 - 127 (2 byte)
    // 11111 10000001 00000001 - 11111 11111111 01111111 => 128 - 16383 (3 byte)

    tlv.tag = *(data++);
    tlv.ber.constructed = ((tlv.tag & 0x20) != 0);
    tlv.ber.classVar = (tlv.tag >> 6) & 0x03;
    if((tlv.tag & 0x1f) == 0x1f) {
        // BER-TLV, multi byte tag
        tlv.tag <<= 8;
        tlv.tag |= *(data++);
        tlv.ber.tag = tlv.tag & 0x7f;
        if(tlv.tag & 0x80) {
            // BER-TLV, 3 byte tag
            tlv.tag &= ~0x80;
            tlv.tag <<= 7;
            tlv.tag |= *(data++) & 0x7f;
            tlv.ber.tag = tlv.tag & 0x3fff;
        }
    } else {
        tlv.ber.tag = tlv.tag & 0x1f;
    }

    //TODO: check for invalid 'indefinite length'
    tlv.length = *(data++);
    if(tlv.length == 0xff) {
        // Simple-TLV 2 byte length
        tlv.length = *(data++) << 8;
        tlv.length += *(data++);
    } else if(tlv.length > 0x7f) {
        uint8_t length_bytes = tlv.length & 0x7f;
        //printf("BER length of %d bytes\n", length_bytes);
        if(length_bytes < 1 || length_bytes > 4) {
            //TODO: error: ISO7816 doesn't support more than 4 length bytes
            return (TlvInfo){.tag = 0};
        }
        tlv.length = 0;
        for(uint8_t i = 0; i < length_bytes; ++i) {
            //printf("byte %d: %02x\n", i, *data);
            tlv.length <<= 8;
            tlv.length |= *(data++);
        }
    }
    tlv.value = data;
    tlv.next = data + tlv.length;

    return tlv;
}

TlvInfo
    iso7816_tlv_select(const uint8_t* data, size_t length, const uint16_t tags[], size_t num_tags) {
    TlvInfo tlv;
    size_t offset = 0;

    if(num_tags == 0) {
        return (TlvInfo){.tag = 0x0000};
    }

    while(offset < length) {
        tlv = iso7816_tlv_parse(data + offset);

        if(tlv.tag == tags[0]) {
            if(num_tags == 1) {
                return tlv;
            } else {
                return iso7816_tlv_select(tlv.value, tlv.length, tags + 1, num_tags - 1);
            }
        }

        offset =
            tlv.next - data; // TODO: use some length value of TlvInfo instead of this monstrosity
    }

    return (TlvInfo){.tag = 0x0000};
}
