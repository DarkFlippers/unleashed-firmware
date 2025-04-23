#include <furi.h>
#include <furi_hal.h>
#include <cli/cli_main_commands.h>
#include <lib/toolbox/args.h>
#include <lib/toolbox/hex.h>
#include <lib/toolbox/bit_buffer.h>
#include <lib/nfc/nfc_poller.h>
#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller.h>
#include <lib/nfc/protocols/iso14443_4b/iso14443_4b_poller.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3_poller.h>
#include <toolbox/pipe.h>

#include <furi_hal_nfc.h>

#define FLAG_EVENT (1 << 10)

#define NFC_MAX_BUFFER_SIZE   (256)
#define NFC_BASE_PROTOCOL_MAX (3)
#define POLLER_DONE           (1 << 0)
#define POLLER_ERR            (1 << 1)
static NfcProtocol BASE_PROTOCOL[NFC_BASE_PROTOCOL_MAX] = {
    NfcProtocolIso14443_4a,
    NfcProtocolIso14443_4b,
    NfcProtocolIso15693_3};
typedef struct ApduContext {
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    bool ready;
    FuriThreadId thread_id;
} ApduContext;

static void nfc_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("nfc <cmd>\r\n");
    printf("Cmd list:\r\n");
    printf("\tapdu\t - Send APDU and print response \r\n");
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        printf("\tfield\t - turn field on\r\n");
    }
}

static void nfc_cli_field(PipeSide* pipe, FuriString* args) {
    UNUSED(args);
    // Check if nfc worker is not busy
    if(furi_hal_nfc_is_hal_ready() != FuriHalNfcErrorNone) {
        printf("NFC chip failed to start\r\n");
        return;
    }

    furi_hal_nfc_acquire();
    furi_hal_nfc_low_power_mode_stop();
    furi_hal_nfc_poller_field_on();

    printf("Field is on. Don't leave device in this mode for too long.\r\n");
    printf("Press Ctrl+C to abort\r\n");

    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_delay_ms(50);
    }

    furi_hal_nfc_low_power_mode_start();
    furi_hal_nfc_release();
}

static NfcCommand trx_callback(NfcGenericEvent event, void* context) {
    furi_check(context);
    ApduContext* apdu_context = (ApduContext*)context;

    if(apdu_context->ready) {
        apdu_context->ready = false;
        if(NfcProtocolIso14443_4a == event.protocol) {
            Iso14443_4aError err = iso14443_4a_poller_send_block(
                event.instance, apdu_context->tx_buffer, apdu_context->rx_buffer);
            if(Iso14443_4aErrorNone == err) {
                furi_thread_flags_set(apdu_context->thread_id, POLLER_DONE);
                return NfcCommandContinue;
            } else {
                furi_thread_flags_set(apdu_context->thread_id, POLLER_ERR);
                return NfcCommandStop;
            }
        } else if(NfcProtocolIso14443_4b == event.protocol) {
            Iso14443_4bError err = iso14443_4b_poller_send_block(
                event.instance, apdu_context->tx_buffer, apdu_context->rx_buffer);
            if(Iso14443_4bErrorNone == err) {
                furi_thread_flags_set(apdu_context->thread_id, POLLER_DONE);
                return NfcCommandContinue;
            } else {
                furi_thread_flags_set(apdu_context->thread_id, POLLER_ERR);
                return NfcCommandStop;
            }
        } else if(NfcProtocolIso15693_3 == event.protocol) {
            Iso15693_3Error err = iso15693_3_poller_send_frame(
                event.instance,
                apdu_context->tx_buffer,
                apdu_context->rx_buffer,
                ISO15693_3_FDT_POLL_FC);
            if(Iso15693_3ErrorNone == err) {
                furi_thread_flags_set(apdu_context->thread_id, POLLER_DONE);
                return NfcCommandContinue;
            } else {
                furi_thread_flags_set(apdu_context->thread_id, POLLER_ERR);
                return NfcCommandStop;
            }
        } else {
            // should never reach here
            furi_crash("Unknown protocol");
        }
    } else {
        furi_delay_ms(100);
    }

    return NfcCommandContinue;
}

static void nfc_cli_apdu(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);
    Nfc* nfc = NULL;
    NfcPoller* poller = NULL;
    FuriString* data = furi_string_alloc();
    uint8_t* req_buffer = NULL;
    uint8_t* resp_buffer = NULL;
    size_t apdu_size = 0;
    size_t resp_size = 0;
    NfcProtocol current_protocol = NfcProtocolInvalid;

    do {
        if(0 == args_get_first_word_length(args)) {
            printf(
                "Use like `nfc apdu 00a404000e325041592e5359532e444446303100 00a4040008a0000003010102` \r\n");
            break;
        }
        nfc = nfc_alloc();

        printf("detecting tag\r\n");
        for(int i = 0; i < NFC_BASE_PROTOCOL_MAX; i++) {
            poller = nfc_poller_alloc(nfc, BASE_PROTOCOL[i]);
            bool is_detected = nfc_poller_detect(poller);
            nfc_poller_free(poller);
            if(is_detected) {
                current_protocol = BASE_PROTOCOL[i];
                printf("detected tag:%d\r\n", BASE_PROTOCOL[i]);
                break;
            }
        }
        if(NfcProtocolInvalid == current_protocol) {
            nfc_free(nfc);
            printf("Can not find any tag\r\n");
            break;
        }
        poller = nfc_poller_alloc(nfc, current_protocol);
        ApduContext* apdu_context = malloc(sizeof(ApduContext));
        apdu_context->tx_buffer = bit_buffer_alloc(NFC_MAX_BUFFER_SIZE);
        apdu_context->rx_buffer = bit_buffer_alloc(NFC_MAX_BUFFER_SIZE);
        apdu_context->ready = false;
        apdu_context->thread_id = furi_thread_get_current_id();

        nfc_poller_start(poller, trx_callback, apdu_context);
        while(args_read_string_and_trim(args, data)) {
            bit_buffer_reset(apdu_context->tx_buffer);
            bit_buffer_reset(apdu_context->rx_buffer);
            apdu_size = furi_string_size(data) / 2;
            req_buffer = malloc(apdu_size);

            hex_chars_to_uint8(furi_string_get_cstr(data), req_buffer);
            printf("Sending APDU:%s to Tag\r\n", furi_string_get_cstr(data));
            bit_buffer_copy_bytes(apdu_context->tx_buffer, req_buffer, apdu_size);
            apdu_context->ready = true;
            uint32_t flags = furi_thread_flags_wait(POLLER_DONE, FuriFlagWaitAny, 3000);
            if(0 == (flags & POLLER_DONE)) {
                printf("Error or Timeout");
                free(req_buffer);
                break;
            }
            furi_assert(apdu_context->ready == false);
            resp_size = bit_buffer_get_size_bytes(apdu_context->rx_buffer) * 2;
            if(!resp_size) {
                printf("No response\r\n");
                free(req_buffer);
                continue;
            }
            resp_buffer = malloc(resp_size);
            uint8_to_hex_chars(
                bit_buffer_get_data(apdu_context->rx_buffer), resp_buffer, resp_size);
            resp_buffer[resp_size] = 0;
            printf("Response: %s\r\n", resp_buffer);

            free(req_buffer);
            free(resp_buffer);
        }

        nfc_poller_stop(poller);
        nfc_poller_free(poller);
        nfc_free(nfc);

        bit_buffer_free(apdu_context->tx_buffer);
        bit_buffer_free(apdu_context->rx_buffer);
        free(apdu_context);
    } while(false);

    furi_string_free(data);
}

static void execute(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            nfc_cli_print_usage();
            break;
        }
        if(furi_string_cmp_str(cmd, "apdu") == 0) {
            nfc_cli_apdu(pipe, args);
            break;
        }
        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            if(furi_string_cmp_str(cmd, "field") == 0) {
                nfc_cli_field(pipe, args);
                break;
            }
        }

        nfc_cli_print_usage();
    } while(false);

    furi_string_free(cmd);
}

CLI_COMMAND_INTERFACE(nfc, execute, CliCommandFlagDefault, 1024, CLI_APPID);
