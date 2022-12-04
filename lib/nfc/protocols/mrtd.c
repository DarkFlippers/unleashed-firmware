#include <furi_hal_random.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <nfc/nfc_device.h>

#include "../helpers/iso7816.h"

#include "mrtd.h"

#define TAG "Mrtd"

//TODO: Check EF.DIR first? Before LDS1
//TODO: ICAO 9303 p11 §4.2 steps
//- Read EF.CardAccess (REQUIRED)
//  If not available or does not contain PACE params, try BAC
//- Read EF.DIR (OPTIONAL)
//  Check list of applications present
//- PACE (CONDITIONAL)
//- BAC (CONDITIONAL)

//TODO: idea - generalize ISO7816 reading. List available apps

#define num_elements(A) (sizeof(A) / sizeof(A[0]))

static const char* mrtd_auth_file_header = "Flipper MRTD params";
static const uint32_t mrtd_auth_file_version = 1;

static void hexdump(FuriLogLevel level, char* prefix, void* data, size_t length) {
    if(furi_log_get_level() >= level) {
        printf("%s ", prefix);
        for(size_t i = 0; i < length; i++) {
            printf("%02X ", ((uint8_t*)data)[i]);
        }
        printf("\r\n");
    }
}

static void mrtd_trace(MrtdApplication* app) {
    FuriHalNfcTxRxContext* tx_rx = app->tx_rx;
    if(furi_log_get_level() == FuriLogLevelTrace) {
        printf("TX: ");
        for(size_t i = 0; i < tx_rx->tx_bits / 8; i++) {
            printf("%02X ", tx_rx->tx_data[i]);
        }
        printf("\r\nRX: ");
        for(size_t i = 0; i < tx_rx->rx_bits / 8; i++) {
            printf("%02X ", tx_rx->rx_data[i]);
        }
        printf("\r\n");
    }
}

uint16_t mrtd_decode_response(uint8_t* buffer, size_t len) {
    // Last two bytes are return code
    return (buffer[len - 2] << 8) | buffer[len - 1];
}

//TODO: rename to transceive?
//TODO: PRIO output and output written writing seems to crash flipper, sometimes
bool mrtd_send_apdu(
    MrtdApplication* app,
    uint8_t cla,
    uint8_t ins,
    uint8_t p1,
    uint8_t p2,
    uint8_t lc,
    const void* data,
    int16_t le,
    uint8_t* output,
    size_t* output_written) {
    FuriHalNfcTxRxContext* tx_rx = app->tx_rx;
    size_t idx = 0;

    FURI_LOG_T(TAG, "Send APDU, lc: %d, le: %d", lc, le);

    if(app->secure_messaging) {
        app->ssc_long++;
        idx = mrtd_protect_apdu(
            cla, ins, p1, p2, lc, data, le, app->ksenc, app->ksmac, app->ssc_long, tx_rx->tx_data);
    } else {
        tx_rx->tx_data[idx++] = cla;
        tx_rx->tx_data[idx++] = ins;
        tx_rx->tx_data[idx++] = p1;
        tx_rx->tx_data[idx++] = p2;
        if(lc > 0) {
            tx_rx->tx_data[idx++] = lc;
            memcpy(tx_rx->tx_data + idx, data, lc);
            idx += lc;
        }
        if(le >= 0) {
            tx_rx->tx_data[idx++] = le & 0xff;
        }
    }

    tx_rx->tx_bits = idx * 8;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    //TODO: timeout as param?
    if(furi_hal_nfc_tx_rx(tx_rx, 300)) {
        mrtd_trace(app);
        uint16_t ret_code = mrtd_decode_response(tx_rx->rx_data, tx_rx->rx_bits / 8);

        if(app->secure_messaging && ret_code == 0x9000) {
            app->ssc_long++;
            ret_code = mrtd_bac_decrypt_verify_sm(
                tx_rx->rx_data,
                tx_rx->rx_bits / 8 - 2,
                app->ksenc,
                app->ksmac,
                app->ssc_long,
                output,
                output_written);
            //ret_code = 0x1337; //TODO: remove PRIO
        }

        //TODO: handle other return codes?
        if(ret_code == 0x9000) {
            if(!app->secure_messaging && le > 0) {
                // Secure Messaging sets output while decrypting
                output_written = memcpy(output, tx_rx->rx_data, le);
            }
            return true;
        } else {
            FURI_LOG_I(TAG, "APDU answer is not 0x9000, but 0x%04X", ret_code);

            switch(ret_code) {
            case 0x6987:
                FURI_LOG_I(TAG, "'expected secure messaging data objects are missing'");
                app->secure_messaging = false;
                break;
            case 0x6988:
                FURI_LOG_I(TAG, "'secure messaging data objects are incorrect'");
                app->secure_messaging = false;
                break;
            case 0xff01:
                //CUSTOM ERROR CODE from mrtd_helpers.c
                FURI_LOG_I(TAG, "'invalid padding'");
                break;
            case 0xff02:
                //CUSTOM ERROR CODE from mrtd_helpers.c
                FURI_LOG_I(TAG, "'verify failed'");
                break;
            }

            return false;
        }
    } else {
        FURI_LOG_D(TAG, "Sending - failed");
    }
    return false;
}

//TODO: rename commands to "mrtd_cmd_..."
bool mrtd_select_app(MrtdApplication* app, AIDValue aid) {
    FURI_LOG_D(
        TAG,
        "Send select App: %02X %02X %02X %02X %02X %02X %02X",
        aid[0],
        aid[1],
        aid[2],
        aid[3],
        aid[4],
        aid[5],
        aid[6]);
    if(!mrtd_send_apdu(app, 0x00, 0xA4, 0x04, 0x0C, 0x07, aid, -1, NULL, NULL)) {
        FURI_LOG_W(TAG, "Failed select App");
        return false;
    }
    return true;
}

bool mrtd_get_challenge(MrtdApplication* app, uint8_t challenge[8]) {
    FURI_LOG_D(TAG, "Send Get Challenge");
    size_t chal_size;
    if(!mrtd_send_apdu(app, 0x00, 0x84, 0x00, 0x00, 0x00, NULL, 0x08, challenge, &chal_size)) {
        FURI_LOG_W(TAG, "Failed get challenge");
        return false;
    }

    return true;
}

bool mrtd_external_authenticate(
    MrtdApplication* app,
    uint8_t* cmd_data,
    size_t cmd_size,
    uint8_t* out_data,
    size_t out_size) {
    furi_assert(cmd_size == 0x28);
    furi_assert(out_size >= 0x28);

    FURI_LOG_D(TAG, "Send External Authenticate");
    if(!mrtd_send_apdu(
           app, 0x00, 0x82, 0x00, 0x00, cmd_size, cmd_data, 0x28, out_data, &out_size)) {
        FURI_LOG_W(TAG, "Failed External Authenticate");
        return false;
    }

    return true;
}

bool mrtd_select_file(MrtdApplication* app, EFFile file) {
    uint8_t data[] = {file.file_id >> 8, file.file_id & 0xff};
    FURI_LOG_D(TAG, "Send select EF: %s (0x%04X)", file.name, file.file_id);
    if(!mrtd_send_apdu(app, 0x00, 0xA4, 0x02, 0x0C, 0x02, data, -1, NULL, NULL)) {
        FURI_LOG_E(TAG, "Failed select EF 0x%04X", file.file_id);
        return false;
    }

    return true;
}

size_t mrtd_read_binary(MrtdApplication* app, uint8_t* buffer, size_t bufsize, size_t offset) {
    UNUSED(buffer);
    UNUSED(bufsize);
    // 00 B0 offst -
    FURI_LOG_D(TAG, "Read binary, offset: %d", offset);
    //TODO: read first 4 bytes, determine length, iterate through file
    //TODO: limit reading/buffer fill to max bufsize

    //TODO: test with max_read = bufsize (value !0, > file size)
    int16_t max_read = 0; // 0 = 'everything', -1 = 'nothing', >0 = amount of bytes
    size_t buf_written = 0;
    if(!mrtd_send_apdu(
           app, 0x00, 0xB0, offset >> 8, offset & 0xff, 0x00, NULL, max_read, buffer, &buf_written)) {
        FURI_LOG_E(TAG, "Failed to read");
        return 0;
    }
    FURI_LOG_D(TAG, "buf_written: %d\n", buf_written);

    return buf_written;
}

//TODO: use short id to read, because it's mandatory for eMRTD
//TODO: check for support of extended length in EF.ATR/INFO, see ISO7816-4

void mrtd_read_dump(MrtdApplication* app, EFFile file) {
    FURI_LOG_D(TAG, "Read and dump %s:", file.name);

    if(!mrtd_select_file(app, file)) {
        return;
    }

    uint8_t data[2048];
    size_t read = 0;
    size_t offset = 0;
    do {
        read = mrtd_read_binary(app, data, sizeof(data), offset);
        offset += read;

        hexdump(FuriLogLevelDebug, "Data:", data, read);
    } while(read > 0);
}

bool parse_ef_dir(EF_DIR_contents* EF_DIR, const uint8_t* data, size_t length) {
    size_t offset = 0;
    uint8_t app_idx = 0;

    memset(EF_DIR->applications, 0x00, sizeof(EF_DIR->applications));
    EF_DIR->applications_count = 0;

    while(offset < length) {
        TlvInfo tlv = iso7816_tlv_parse(data + offset);

        if(tlv.tag != 0x61 || tlv.length != 0x09) {
            FURI_LOG_E(
                TAG,
                "Invalid EF.DIR, tag at offset %d must be '61' and length 9. Got '%02X' and %d",
                offset,
                tlv.tag,
                tlv.length);
            return false;
        }

        tlv = iso7816_tlv_parse(tlv.value);
        if(tlv.tag != 0x4F || tlv.length != 0x07) {
            FURI_LOG_E(
                TAG, "Invalid EF.DIR, subtag at offset %d must be '4F' and length 7", offset);
            return false;
        }

        memcpy(EF_DIR->applications[app_idx], tlv.value, tlv.length);
        EF_DIR->applications_count = ++app_idx;

        offset = tlv.next - data;
    }

    //TODO: remove testing block:
    FURI_LOG_D(TAG, "EF.DIR applications: %d", EF_DIR->applications_count);
    if(furi_log_get_level() >= FuriLogLevelDebug) {
        for(uint8_t i = 0; i < EF_DIR->applications_count; ++i) {
            printf("- ");
            for(uint8_t n = 0; n < sizeof(AIDValue); ++n) {
                printf("%02X ", EF_DIR->applications[i][n]);
            }
            printf("\r\n");
        }
    }

    return true;
}

bool parse_ef_com(EF_COM_contents* EF_COM, const uint8_t* data, size_t length) {
    uint16_t lds_tag_path[] = {0x60, 0x5f01};
    uint16_t unicode_tag_path[] = {0x60, 0x5f36};
    uint16_t tags_tag_path[] = {0x60, 0x5c};

    TlvInfo tlv_lds_version =
        iso7816_tlv_select(data, length, lds_tag_path, num_elements(lds_tag_path));
    if(!tlv_lds_version.tag) {
        FURI_LOG_W(TAG, "EF.COM LDS version not found");
        return false;
    }

    EF_COM->lds_version = tlv_number(tlv_lds_version);

    TlvInfo tlv_unicode_version =
        iso7816_tlv_select(data, length, unicode_tag_path, num_elements(unicode_tag_path));
    if(!tlv_unicode_version.tag) {
        FURI_LOG_W(TAG, "EF.COM Unicode info not found!");
        return false;
    }

    EF_COM->unicode_version = tlv_number(tlv_unicode_version);

    TlvInfo tlv_tag_list =
        iso7816_tlv_select(data, length, tags_tag_path, num_elements(tags_tag_path));
    if(!tlv_tag_list.tag) {
        FURI_LOG_W(TAG, "EF.CO Tag List not found!");
        return false;
    }

    for(size_t i = 0; i < MAX_EFCOM_TAGS; ++i) {
        EF_COM->tag_list[i] = (i < tlv_tag_list.length) ? tlv_tag_list.value[i] : 0x00;
    }

    return true;
}

void mrzcpy(uint8_t* dest, const uint8_t* src, size_t* idx, size_t n) {
    //FURI_LOG_D(TAG, "mrzcpy %d: %.*s", n, n, src + *idx);
    //memcpy(dest, src + *idx, n);
    for(size_t i = 0; i < n; ++i) {
        uint8_t c = src[i + *idx];
        if(c == '<') {
            c = ' ';
        }
        dest[i] = c;
    }
    dest[n] = 0x00;
    *idx += n;
}

bool parse_ef_dg1(EF_DG1_contents* DG1, const uint8_t* data, size_t length) {
    TlvInfo tlv_mrz = iso7816_tlv_select(data, length, (uint16_t[]){0x61, 0x5f1f}, 2);

    if(!tlv_mrz.tag) {
        FURI_LOG_W(TAG, "DG1, unexpected content. Could not find tag 0x61, 0x5f1f");
        return false;
    }

    const uint8_t* mrz = tlv_mrz.value;
    size_t idx = 0;

    switch(tlv_mrz.length) {
    case 90:
        DG1->type = MrtdTypeTD1;
        mrzcpy(DG1->doctype, mrz, &idx, 2);
        mrzcpy(DG1->issuing_state, mrz, &idx, 3);
        mrzcpy(DG1->docnr, mrz, &idx, 9);
        idx += 1; // docnr check digit
        idx += 15; // optional data
        mrtd_parse_date(&DG1->birth_date, mrz + idx);
        idx += 6; // birth_date
        idx += 1; // birth date check digit
        mrzcpy(DG1->sex, mrz, &idx, 1);
        mrtd_parse_date(&DG1->expiry_date, mrz + idx);
        idx += 6; // expiry_date
        idx += 1; // expiry date check digit
        mrzcpy(DG1->nationality, mrz, &idx, 3);
        idx += 11; // optional data
        idx += 1; // check digit
        mrzcpy(DG1->name, mrz, &idx, 30);
        // 30 + 30 + 30
        break;
    case 72:
        DG1->type = MrtdTypeTD2;
        mrzcpy(DG1->doctype, mrz, &idx, 2);
        mrzcpy(DG1->issuing_state, mrz, &idx, 3);
        mrzcpy(DG1->name, mrz, &idx, 31);
        mrzcpy(DG1->docnr, mrz, &idx, 9);
        idx += 1; // docnr check digit
        mrzcpy(DG1->nationality, mrz, &idx, 3);
        mrtd_parse_date(&DG1->birth_date, mrz + idx);
        idx += 6; // birth_date
        idx += 1; // birth date check digit
        mrzcpy(DG1->sex, mrz, &idx, 1);
        mrtd_parse_date(&DG1->expiry_date, mrz + idx);
        idx += 6; // expiry_date
        idx += 1; // expiry date check digit
        idx += 7; // optional data
        idx += 1; // check digit
        // 36 + 36
        break;
    case 88:
        DG1->type = MrtdTypeTD3;
        mrzcpy(DG1->doctype, mrz, &idx, 2);
        mrzcpy(DG1->issuing_state, mrz, &idx, 3);
        mrzcpy(DG1->name, mrz, &idx, 39);
        mrzcpy(DG1->docnr, mrz, &idx, 9);
        idx += 1; // docnr check digit
        mrzcpy(DG1->nationality, mrz, &idx, 3);
        mrtd_parse_date(&DG1->birth_date, mrz + idx);
        idx += 1; // birth date check digit
        idx += 6; // birth_date
        mrzcpy(DG1->sex, mrz, &idx, 1);
        mrtd_parse_date(&DG1->expiry_date, mrz + idx);
        idx += 6; // expiry_date
        idx += 1; // expiry date check digit
        idx += 14; // optional data
        idx += 1; //  check digit
        idx += 1; // check digit
        // 44 + 44
        break;
    default:
        FURI_LOG_W(
            TAG, "Unexpected MRZ length in DG1: %d. TD1=90, TD2=72, TD3=88.", tlv_mrz.length);
        return false;
    }

    return true;
}

bool mrtd_read_parse_file(MrtdApplication* app, EFFile file) {
    uint8_t buffer[100];
    size_t buf_len;

    FURI_LOG_D(TAG, "Read and parse %s (%04X)", file.name, file.file_id);

    if(!mrtd_select_file(app, file)) {
        FURI_LOG_E(TAG, "Could not select %s", file.name);
        return false;
    }

    FURI_LOG_D(TAG, "Selected %s", file.name);

    buf_len = mrtd_read_binary(app, buffer, num_elements(buffer), 0);

    if(!buf_len) {
        FURI_LOG_E(TAG, "Could not read %s", file.name);
        return false;
    }

    FURI_LOG_D(TAG, "Read %s", file.name);

    bool result = false;

    if(file.file_id == EF.COM.file_id) {
        result = parse_ef_com(&app->mrtd_data->files.EF_COM, buffer, buf_len);
        FURI_LOG_D(TAG, "Parsed EF.COM");
    } else if(file.file_id == EF.DIR.file_id) {
        result = parse_ef_dir(&app->mrtd_data->files.EF_DIR, buffer, buf_len);
        FURI_LOG_D(TAG, "Parsed EF.DIR");
    } else if(file.file_id == EF.DG1.file_id) {
        result = parse_ef_dg1(&app->mrtd_data->files.DG1, buffer, buf_len);
    } else {
        FURI_LOG_W(TAG, "Don't know how to parse file with id 0x%04X", file.file_id);
    }

    return result;
}

MrtdApplication* mrtd_alloc_init(FuriHalNfcTxRxContext* tx_rx, MrtdData* mrtd_data) {
    MrtdApplication* app = malloc(sizeof(MrtdApplication));

    app->tx_rx = tx_rx;
    app->mrtd_data = mrtd_data;

    return app;
}

void mrtd_free(MrtdApplication* app) {
    furi_assert(app);
    free(app);
}

bool mrtd_bac(MrtdApplication* app, MrtdAuthData* auth) {
    UNUSED(app);

    static bool rand_generator_inited = false;
    uint8_t rnd_ic[8];
    uint8_t rnd_ifd[8];
    uint8_t k_ifd[16];

    if(!rand_generator_inited) {
        // TODO: should random initialization maybe be system wide?
        srand(DWT->CYCCNT);
        rand_generator_inited = true;
    }

    mrtd_get_challenge(app, rnd_ic);
    //TODO: remove memcpy rnd_ic
    //memcpy(rnd_ic, "\x46\x08\xF9\x19\x88\x70\x22\x12", 8);

    furi_hal_random_fill_buf(rnd_ifd, 8);
    furi_hal_random_fill_buf(k_ifd, 16);
    //TODO: remove testing code:
    //memcpy(rnd_ifd, "\x78\x17\x23\x86\x0C\x06\xC2\x26", 8);
    //memcpy(k_ifd, "\x0B\x79\x52\x40\xCB\x70\x49\xB0\x1C\x19\xB3\x3E\x32\x80\x4F\x0B", 16);

    hexdump(FuriLogLevelDebug, "rnd_ifd:", rnd_ifd, 8);
    hexdump(FuriLogLevelDebug, "k_ifd:", k_ifd, 16);

    uint8_t kenc[16];
    uint8_t kmac[16];

    if(!mrtd_bac_keys(auth, kenc, kmac)) {
        FURI_LOG_E(TAG, "Failed to calculate BAC keys");
        return false;
    }

    uint8_t S[32];
    memcpy(S, rnd_ifd, 8);
    memcpy(S + 8, rnd_ic, 8);
    memcpy(S + 16, k_ifd, 16);

    hexdump(FuriLogLevelDebug, "S:", S, 32);

    uint8_t cmd_data[40];
    uint8_t* eifd = cmd_data;
    uint8_t* mifd = cmd_data + 32;
    mrtd_bac_encrypt(S, 32, kenc, eifd);
    mrtd_bac_padded_mac(eifd, 32, kmac, mifd);

    uint8_t response[40];
    if(!mrtd_external_authenticate(app, cmd_data, 40, response, 40)) {
        FURI_LOG_E(TAG, "BAC External Authenticate failed");
        return false;
    }

    uint8_t buffer[32]; // Received R = RND.IC (8) || RND.IFD (8) || KIC (16)
    if(!mrtd_bac_decrypt_verify(response, 40, kenc, kmac, buffer)) {
        FURI_LOG_W(TAG, "BAC DecryptVerify failed");
    }

    uint8_t* rnd_ifd_recv = buffer + 8;
    uint8_t* kic = buffer + 16;

    hexdump(FuriLogLevelDebug, "kic:", kic, 16);

    if(memcmp(rnd_ifd, rnd_ifd_recv, 8)) {
        FURI_LOG_W(TAG, "BAC RND.IFD sent and received mismatch.");
    }

    uint8_t kseed[16];
    for(uint8_t i = 0; i < 16; ++i) {
        kseed[i] = k_ifd[i] ^ kic[i];
        //printf("seed %2d = %02X ^ %02X = %02X\r\n", i, k_ifd[i], kic[i], kseed[i]);
    }

    hexdump(FuriLogLevelDebug, "kseed:", kseed, 16);

    if(!mrtd_bac_keys_from_seed(kseed, app->ksenc, app->ksmac)) {
        FURI_LOG_E(TAG, "BAC error, could not derive KSenc and KSmac");
        return false;
    }
    hexdump(FuriLogLevelDebug, "ksenc:", app->ksenc, 16);
    hexdump(FuriLogLevelDebug, "ksmac:", app->ksmac, 16);

    hexdump(FuriLogLevelTrace, "RND.IC:", rnd_ic, 8);
    hexdump(FuriLogLevelTrace, "RND.IFS:", rnd_ifd, 8);

    app->ssc_long = mrtd_ssc_from_data(rnd_ic, rnd_ifd);
    FURI_LOG_D(TAG, "SSC: %01llX", app->ssc_long);

    app->secure_messaging = true;

    return true;
}

bool mrtd_authenticate(MrtdApplication* app) {
    MrtdAuthMethod method = app->mrtd_data->auth.method;
    app->mrtd_data->auth_success = false;
    app->mrtd_data->auth_method_used = MrtdAuthMethodNone;
    FURI_LOG_D(TAG, "Auth method: %d", method);
    switch(method) {
    case MrtdAuthMethodAny:
        //TODO: try PACE, then BAC. For now, fall through to just BAC
    case MrtdAuthMethodBac:
        app->mrtd_data->auth_success = mrtd_bac(app, &app->mrtd_data->auth);
        app->mrtd_data->auth_method_used = MrtdAuthMethodBac;
        break;
    case MrtdAuthMethodPace:
        FURI_LOG_E(TAG, "Auth method PACE not implemented");
        break;
    case MrtdAuthMethodNone:
    default:
        break;
    }

    if(!app->mrtd_data->auth_success) {
        return false;
    }

    return true;
}

bool mrtd_auth_params_save(
    Storage* storage,
    DialogsApp* dialogs,
    MrtdAuthData* auth_data,
    const char* file_name) {
    return mrtd_auth_params_save_file(
        storage, dialogs, auth_data, file_name, MRTD_APP_FOLDER, MRTD_APP_EXTENSION);
}

void mrtd_date_prepare_format_string(MrtdDate date, FuriString* format_string) {
    furi_string_printf(format_string, "%02u%02u%02u", date.year, date.month, date.day);
}

bool mrtd_date_parse_format_string(MrtdDate* date, FuriString* format_string) {
    int year;
    int month;
    int day;

    int ret = sscanf(furi_string_get_cstr(format_string), "%02d%02d%02d", &year, &month, &day);
    if(ret != 3) {
        return false;
    }

    date->year = year;
    date->month = month;
    date->day = day;
    return true;
}

bool mrtd_auth_params_save_file(
    Storage* storage,
    DialogsApp* dialogs,
    MrtdAuthData* auth_data,
    const char* file_name,
    const char* folder,
    const char* extension) {
    furi_assert(auth_data);

    bool saved = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str;
    temp_str = furi_string_alloc();

    do {
        // Create mrtd directory if necessary
        if(!storage_simply_mkdir(storage, MRTD_APP_FOLDER)) break;

        furi_string_printf(temp_str, "%s/%s%s", folder, file_name, extension);

        // Open file
        if(!flipper_format_file_open_always(file, furi_string_get_cstr(temp_str))) break;
        // Write header
        if(!flipper_format_write_header_cstr(file, mrtd_auth_file_header, mrtd_auth_file_version))
            break;

        // Write auth method
        furi_string_set(temp_str, mrtd_auth_method_string(auth_data->method));
        if(!flipper_format_write_string(file, "Method", temp_str)) break;

        // Write birth date
        mrtd_date_prepare_format_string(auth_data->birth_date, temp_str);
        if(!flipper_format_write_string(file, "BirthDate", temp_str)) break;

        // Write expiry date
        mrtd_date_prepare_format_string(auth_data->expiry_date, temp_str);
        if(!flipper_format_write_string(file, "ExpiryDate", temp_str)) break;

        // Write docnr
        furi_string_set(temp_str, auth_data->doc_number);
        if(!flipper_format_write_string(file, "DocNr", temp_str)) break;

        saved = true;
    } while(false);

    if(!saved) {
        dialog_message_show_storage_error(dialogs, "Can not save\nparams file");
    }
    furi_string_free(temp_str);
    flipper_format_free(file);
    return saved;
}

bool mrtd_auth_params_load(
    Storage* storage,
    DialogsApp* dialogs,
    MrtdAuthData* auth_data,
    const char* file_path,
    bool show_dialog) {
    furi_assert(storage);
    furi_assert(dialogs);
    furi_assert(auth_data);
    furi_assert(file_path);

    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool deprecated_version = false;

    FuriString* temp_str;
    temp_str = furi_string_alloc();

    MrtdAuthData copy;

    FURI_LOG_D(TAG, "Load auth params");

    do {
        if(!flipper_format_file_open_existing(file, file_path)) break;

        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        FURI_LOG_D(TAG, "Version: %s", furi_string_get_cstr(temp_str));
        if(furi_string_cmp_str(temp_str, mrtd_auth_file_header) ||
           (version != mrtd_auth_file_version)) {
            deprecated_version = true;
            break;
        }

        if(!flipper_format_read_string(file, "Method", temp_str)) break;
        FURI_LOG_D(TAG, "Method: %s", furi_string_get_cstr(temp_str));
        if(!mrtd_auth_method_parse_string(&copy.method, furi_string_get_cstr(temp_str))) break;

        if(!flipper_format_read_string(file, "BirthDate", temp_str)) break;
        FURI_LOG_D(TAG, "BirthDate: %s", furi_string_get_cstr(temp_str));
        if(!mrtd_date_parse_format_string(&copy.birth_date, temp_str)) break;

        if(!flipper_format_read_string(file, "ExpiryDate", temp_str)) break;
        FURI_LOG_D(TAG, "ExpiryDate: %s", furi_string_get_cstr(temp_str));
        if(!mrtd_date_parse_format_string(&copy.expiry_date, temp_str)) break;

        if(!flipper_format_read_string(file, "DocNr", temp_str)) break;
        FURI_LOG_D(TAG, "DocNr: %s", furi_string_get_cstr(temp_str));
        strlcpy(copy.doc_number, furi_string_get_cstr(temp_str), MRTD_DOCNR_MAX_LENGTH);

        // Everything went fine. Save copy to pointed auth data
        *auth_data = copy;
        parsed = true;
    } while(false);

    FURI_LOG_D(TAG, "Load done, success: %d", parsed);

    if(!parsed && show_dialog) {
        if(deprecated_version) {
            dialog_message_show_storage_error(dialogs, "File format deprecated");
        } else {
            dialog_message_show_storage_error(dialogs, "Can not parse\nfile");
        }
    }

    furi_string_free(temp_str);
    flipper_format_free(file);
    return parsed;
}
