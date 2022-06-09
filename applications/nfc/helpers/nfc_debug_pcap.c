#include "nfc_debug_pcap.h"

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

File* nfc_debug_pcap_open(Storage* storage) {
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, "/ext/nfc/debug.pcap", FSAM_WRITE, FSOM_OPEN_APPEND)) {
        storage_file_free(file);
        return NULL;
    }
    if(!storage_file_tell(file)) {
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
        if(storage_file_write(file, &pcap_hdr, sizeof(pcap_hdr)) != sizeof(pcap_hdr)) {
            FURI_LOG_E(TAG, "Failed to write pcap header");
        }
    }
    return file;
}

void nfc_debug_pcap_write(Storage* storage, uint8_t event, uint8_t* data, uint16_t len) {
    File* file = nfc_debug_pcap_open(storage);
    if(!file) return;

    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);

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
    if(storage_file_write(file, &pkt_hdr, sizeof(pkt_hdr)) != sizeof(pkt_hdr)) {
        FURI_LOG_E(TAG, "Failed to write pcap packet header");
    } else if(storage_file_write(file, data, len) != len) {
        FURI_LOG_E(TAG, "Failed to write pcap packet data");
    }
    storage_file_free(file);
}

void nfc_debug_pcap_write_tx(uint8_t* data, uint16_t bits, bool crc_dropped, void* context) {
    uint8_t event = crc_dropped ? DATA_PCD_TO_PICC_CRC_DROPPED : DATA_PCD_TO_PICC;
    nfc_debug_pcap_write(context, event, data, bits / 8);
}

void nfc_debug_pcap_write_rx(uint8_t* data, uint16_t bits, bool crc_dropped, void* context) {
    uint8_t event = crc_dropped ? DATA_PICC_TO_PCD_CRC_DROPPED : DATA_PICC_TO_PCD;
    nfc_debug_pcap_write(context, event, data, bits / 8);
}

void nfc_debug_pcap_prepare_tx_rx(FuriHalNfcTxRxContext* tx_rx, Storage* storage, bool is_picc) {
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        if(is_picc) {
            tx_rx->sniff_tx = nfc_debug_pcap_write_rx;
            tx_rx->sniff_rx = nfc_debug_pcap_write_tx;
        } else {
            tx_rx->sniff_tx = nfc_debug_pcap_write_tx;
            tx_rx->sniff_rx = nfc_debug_pcap_write_rx;
        }
        tx_rx->sniff_context = storage;
    }
}
