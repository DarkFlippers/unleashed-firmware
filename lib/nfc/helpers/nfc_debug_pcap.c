#include "nfc_debug_pcap.h"

#include <storage/storage.h>
#include <stream/buffered_file_stream.h>
#include <furi_hal_nfc.h>
#include <furi_hal_rtc.h>

#define TAG "NfcDebugPcap"

#define PCAP_MAGIC 0xa1b2c3d4
#define PCAP_MAJOR 2
#define PCAP_MINOR 4
#define DLT_ISO_14443 264

#define DATA_PICC_TO_PCD 0xFF
#define DATA_PCD_TO_PICC 0xFE
#define DATA_PICC_TO_PCD_CRC_DROPPED 0xFB
#define DATA_PCD_TO_PICC_CRC_DROPPED 0xFA

#define NFC_DEBUG_PCAP_FILENAME EXT_PATH("nfc/debug.pcap")

struct NfcDebugPcap {
    Stream* file_stream;
};

static Stream* nfc_debug_pcap_open(Storage* storage) {
    Stream* stream = NULL;
    stream = buffered_file_stream_alloc(storage);
    if(!buffered_file_stream_open(stream, NFC_DEBUG_PCAP_FILENAME, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        buffered_file_stream_close(stream);
        stream_free(stream);
        stream = NULL;
    } else {
        if(!stream_tell(stream)) {
            struct {
                uint32_t magic;
                uint16_t major, minor;
                uint32_t reserved[2];
                uint32_t snaplen;
                uint32_t link_type;
            } __attribute__((__packed__)) pcap_hdr = {
                .magic = PCAP_MAGIC,
                .major = PCAP_MAJOR,
                .minor = PCAP_MINOR,
                .snaplen = FURI_HAL_NFC_DATA_BUFF_SIZE,
                .link_type = DLT_ISO_14443,
            };
            if(stream_write(stream, (uint8_t*)&pcap_hdr, sizeof(pcap_hdr)) != sizeof(pcap_hdr)) {
                FURI_LOG_E(TAG, "Failed to write pcap header");
                buffered_file_stream_close(stream);
                stream_free(stream);
                stream = NULL;
            }
        }
    }
    return stream;
}

NfcDebugPcap* nfc_debug_pcap_alloc() {
    NfcDebugPcap* instance = malloc(sizeof(NfcDebugPcap));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    instance->file_stream = nfc_debug_pcap_open(storage);
    if(!instance->file_stream) {
        free(instance);
        instance = NULL;
    }
    furi_record_close(RECORD_STORAGE);

    return instance;
}

void nfc_debug_pcap_free(NfcDebugPcap* instance) {
    furi_assert(instance);
    furi_assert(instance->file_stream);

    buffered_file_stream_close(instance->file_stream);
    stream_free(instance->file_stream);

    free(instance);
}

void nfc_debug_pcap_process_data(
    NfcDebugPcap* instance,
    uint8_t* data,
    uint16_t len,
    bool reader_to_tag,
    bool crc_dropped) {
    furi_assert(instance);
    furi_assert(data);
    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);

    uint8_t event = 0;
    if(reader_to_tag) {
        if(crc_dropped) {
            event = DATA_PCD_TO_PICC_CRC_DROPPED;
        } else {
            event = DATA_PCD_TO_PICC;
        }
    } else {
        if(crc_dropped) {
            event = DATA_PICC_TO_PCD_CRC_DROPPED;
        } else {
            event = DATA_PICC_TO_PCD;
        }
    }

    struct {
        // https://wiki.wireshark.org/Development/LibpcapFileFormat#record-packet-header
        uint32_t ts_sec;
        uint32_t ts_usec;
        uint32_t incl_len;
        uint32_t orig_len;
        // https://www.kaiser.cx/posts/pcap-iso14443/#_packet_data
        uint8_t version;
        uint8_t event;
        uint16_t len;
    } __attribute__((__packed__)) pkt_hdr = {
        .ts_sec = furi_hal_rtc_datetime_to_timestamp(&datetime),
        .ts_usec = 0,
        .incl_len = len + 4,
        .orig_len = len + 4,
        .version = 0,
        .event = event,
        .len = len << 8 | len >> 8,
    };
    stream_write(instance->file_stream, (uint8_t*)&pkt_hdr, sizeof(pkt_hdr));
    stream_write(instance->file_stream, data, len);
}
