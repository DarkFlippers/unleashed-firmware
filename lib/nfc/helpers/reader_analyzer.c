#include "reader_analyzer.h"
#include <lib/nfc/protocols/nfc_util.h>
#include <lib/nfc/protocols/mifare_classic.h>
#include <m-array.h>

#include "mfkey32.h"
#include "nfc_debug_pcap.h"
#include "nfc_debug_log.h"

#define TAG "ReaderAnalyzer"

#define READER_ANALYZER_MAX_BUFF_SIZE (1024)

typedef struct {
    bool reader_to_tag;
    bool crc_dropped;
    uint16_t len;
} ReaderAnalyzerHeader;

typedef enum {
    ReaderAnalyzerNfcDataMfClassic,
} ReaderAnalyzerNfcData;

struct ReaderAnalyzer {
    FuriHalNfcDevData nfc_data;

    bool alive;
    FuriStreamBuffer* stream;
    FuriThread* thread;

    ReaderAnalyzerParseDataCallback callback;
    void* context;

    ReaderAnalyzerMode mode;
    Mfkey32* mfkey32;
    NfcDebugLog* debug_log;
    NfcDebugPcap* pcap;
};

const FuriHalNfcDevData reader_analyzer_nfc_data[] = {
    [ReaderAnalyzerNfcDataMfClassic] =
        {.sak = 0x08,
         .atqa = {0x44, 0x00},
         .interface = FuriHalNfcInterfaceRf,
         .type = FuriHalNfcTypeA,
         .uid_len = 7,
         .uid = {0x04, 0x77, 0x70, 0x2A, 0x23, 0x4F, 0x80},
         .cuid = 0x2A234F80},
};

void reader_analyzer_parse(ReaderAnalyzer* instance, uint8_t* buffer, size_t size) {
    if(size < sizeof(ReaderAnalyzerHeader)) return;

    size_t bytes_i = 0;
    while(bytes_i < size) {
        ReaderAnalyzerHeader* header = (ReaderAnalyzerHeader*)&buffer[bytes_i];
        uint16_t len = header->len;
        if(bytes_i + len > size) break;
        bytes_i += sizeof(ReaderAnalyzerHeader);
        if(instance->mfkey32) {
            mfkey32_process_data(
                instance->mfkey32,
                &buffer[bytes_i],
                len,
                header->reader_to_tag,
                header->crc_dropped);
        }
        if(instance->pcap) {
            nfc_debug_pcap_process_data(
                instance->pcap, &buffer[bytes_i], len, header->reader_to_tag, header->crc_dropped);
        }
        if(instance->debug_log) {
            nfc_debug_log_process_data(
                instance->debug_log,
                &buffer[bytes_i],
                len,
                header->reader_to_tag,
                header->crc_dropped);
        }
        bytes_i += len;
    }
}

int32_t reader_analyzer_thread(void* context) {
    ReaderAnalyzer* reader_analyzer = context;
    uint8_t buffer[READER_ANALYZER_MAX_BUFF_SIZE] = {};

    while(reader_analyzer->alive || !furi_stream_buffer_is_empty(reader_analyzer->stream)) {
        size_t ret = furi_stream_buffer_receive(
            reader_analyzer->stream, buffer, READER_ANALYZER_MAX_BUFF_SIZE, 50);
        if(ret) {
            reader_analyzer_parse(reader_analyzer, buffer, ret);
        }
    }

    return 0;
}

ReaderAnalyzer* reader_analyzer_alloc() {
    ReaderAnalyzer* instance = malloc(sizeof(ReaderAnalyzer));

    instance->nfc_data = reader_analyzer_nfc_data[ReaderAnalyzerNfcDataMfClassic];
    instance->alive = false;
    instance->stream =
        furi_stream_buffer_alloc(READER_ANALYZER_MAX_BUFF_SIZE, sizeof(ReaderAnalyzerHeader));

    instance->thread =
        furi_thread_alloc_ex("ReaderAnalyzerWorker", 2048, reader_analyzer_thread, instance);
    furi_thread_set_priority(instance->thread, FuriThreadPriorityLow);

    return instance;
}

static void reader_analyzer_mfkey_callback(Mfkey32Event event, void* context) {
    furi_assert(context);
    ReaderAnalyzer* instance = context;

    if(event == Mfkey32EventParamCollected) {
        if(instance->callback) {
            instance->callback(ReaderAnalyzerEventMfkeyCollected, instance->context);
        }
    }
}

void reader_analyzer_start(ReaderAnalyzer* instance, ReaderAnalyzerMode mode) {
    furi_assert(instance);

    furi_stream_buffer_reset(instance->stream);
    if(mode & ReaderAnalyzerModeDebugLog) {
        instance->debug_log = nfc_debug_log_alloc();
    }
    if(mode & ReaderAnalyzerModeMfkey) {
        instance->mfkey32 = mfkey32_alloc(instance->nfc_data.cuid);
        if(instance->mfkey32) {
            mfkey32_set_callback(instance->mfkey32, reader_analyzer_mfkey_callback, instance);
        }
    }
    if(mode & ReaderAnalyzerModeDebugPcap) {
        instance->pcap = nfc_debug_pcap_alloc();
    }

    instance->alive = true;
    furi_thread_start(instance->thread);
}

void reader_analyzer_stop(ReaderAnalyzer* instance) {
    furi_assert(instance);

    instance->alive = false;
    furi_thread_join(instance->thread);

    if(instance->debug_log) {
        nfc_debug_log_free(instance->debug_log);
        instance->debug_log = NULL;
    }
    if(instance->mfkey32) {
        mfkey32_free(instance->mfkey32);
        instance->mfkey32 = NULL;
    }
    if(instance->pcap) {
        nfc_debug_pcap_free(instance->pcap);
        instance->pcap = NULL;
    }
}

void reader_analyzer_free(ReaderAnalyzer* instance) {
    furi_assert(instance);

    reader_analyzer_stop(instance);
    furi_thread_free(instance->thread);
    furi_stream_buffer_free(instance->stream);
    free(instance);
}

void reader_analyzer_set_callback(
    ReaderAnalyzer* instance,
    ReaderAnalyzerParseDataCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

NfcProtocol
    reader_analyzer_guess_protocol(ReaderAnalyzer* instance, uint8_t* buff_rx, uint16_t len) {
    furi_assert(instance);
    furi_assert(buff_rx);
    UNUSED(len);
    NfcProtocol protocol = NfcDeviceProtocolUnknown;

    if((buff_rx[0] == 0x60) || (buff_rx[0] == 0x61)) {
        protocol = NfcDeviceProtocolMifareClassic;
    }

    return protocol;
}

FuriHalNfcDevData* reader_analyzer_get_nfc_data(ReaderAnalyzer* instance) {
    furi_assert(instance);
    instance->nfc_data = reader_analyzer_nfc_data[ReaderAnalyzerNfcDataMfClassic];
    return &instance->nfc_data;
}

void reader_analyzer_set_nfc_data(ReaderAnalyzer* instance, FuriHalNfcDevData* nfc_data) {
    furi_assert(instance);
    furi_assert(nfc_data);

    memcpy(&instance->nfc_data, nfc_data, sizeof(FuriHalNfcDevData));
}

static void reader_analyzer_write(
    ReaderAnalyzer* instance,
    uint8_t* data,
    uint16_t len,
    bool reader_to_tag,
    bool crc_dropped) {
    ReaderAnalyzerHeader header = {
        .reader_to_tag = reader_to_tag, .crc_dropped = crc_dropped, .len = len};
    size_t data_sent = 0;
    data_sent = furi_stream_buffer_send(
        instance->stream, &header, sizeof(ReaderAnalyzerHeader), FuriWaitForever);
    if(data_sent != sizeof(ReaderAnalyzerHeader)) {
        FURI_LOG_W(TAG, "Sent %zu out of %zu bytes", data_sent, sizeof(ReaderAnalyzerHeader));
    }
    data_sent = furi_stream_buffer_send(instance->stream, data, len, FuriWaitForever);
    if(data_sent != len) {
        FURI_LOG_W(TAG, "Sent %zu out of %u bytes", data_sent, len);
    }
}

static void
    reader_analyzer_write_rx(uint8_t* data, uint16_t bits, bool crc_dropped, void* context) {
    UNUSED(crc_dropped);
    ReaderAnalyzer* reader_analyzer = context;
    uint16_t bytes = bits < 8 ? 1 : bits / 8;
    reader_analyzer_write(reader_analyzer, data, bytes, false, crc_dropped);
}

static void
    reader_analyzer_write_tx(uint8_t* data, uint16_t bits, bool crc_dropped, void* context) {
    UNUSED(crc_dropped);
    ReaderAnalyzer* reader_analyzer = context;
    uint16_t bytes = bits < 8 ? 1 : bits / 8;
    reader_analyzer_write(reader_analyzer, data, bytes, true, crc_dropped);
}

void reader_analyzer_prepare_tx_rx(
    ReaderAnalyzer* instance,
    FuriHalNfcTxRxContext* tx_rx,
    bool is_picc) {
    furi_assert(instance);
    furi_assert(tx_rx);

    if(is_picc) {
        tx_rx->sniff_tx = reader_analyzer_write_rx;
        tx_rx->sniff_rx = reader_analyzer_write_tx;
    } else {
        tx_rx->sniff_rx = reader_analyzer_write_rx;
        tx_rx->sniff_tx = reader_analyzer_write_tx;
    }

    tx_rx->sniff_context = instance;
}
