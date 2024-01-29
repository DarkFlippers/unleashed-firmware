#include "emv_render.h"

#include "../iso14443_4a/iso14443_4a_render.h"
#include "nfc/nfc_app_i.h"

void nfc_render_emv_info(const EmvData* data, NfcProtocolFormatType format_type, FuriString* str) {
    nfc_render_emv_name(data->emv_application.name, str);
    nfc_render_emv_pan(data->emv_application.pan, data->emv_application.pan_len, str);
    nfc_render_emv_expired(&data->emv_application, str);

    if(format_type == NfcProtocolFormatTypeFull) nfc_render_emv_extra(data, str);
}

void nfc_render_emv_data(const EmvData* data, FuriString* str) {
    nfc_render_emv_pan(data->emv_application.pan, data->emv_application.pan_len, str);
    nfc_render_emv_name(data->emv_application.name, str);
}

void nfc_render_emv_pan(const uint8_t* data, const uint8_t len, FuriString* str) {
    if(len == 0) return;

    FuriString* card_number = furi_string_alloc();
    for(uint8_t i = 0; i < len; i++) {
        if((i % 2 == 0) && (i != 0)) furi_string_cat_printf(card_number, " ");
        furi_string_cat_printf(card_number, "%02X", data[i]);
    }

    // Cut padding 'F' from card number
    furi_string_trim(card_number, "F");
    furi_string_cat(str, card_number);
    furi_string_free(card_number);

    furi_string_cat_printf(str, "\n");
}

void nfc_render_emv_expired(const EmvApplication* apl, FuriString* str) {
    if(apl->exp_month == 0) return;
    furi_string_cat_printf(str, "Exp: %02X/%02X\n", apl->exp_month, apl->exp_year);
}

void nfc_render_emv_currency(uint16_t cur_code, FuriString* str) {
    if(!cur_code) return;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* currency_name = furi_string_alloc();
    if(nfc_emv_parser_get_currency_name(storage, cur_code, currency_name)) {
        furi_string_cat_printf(str, "Currency: %s\n", furi_string_get_cstr(currency_name));
    }
    furi_string_free(currency_name);
    furi_record_close(RECORD_STORAGE);
}

void nfc_render_emv_country(uint16_t country_code, FuriString* str) {
    if(!country_code) return;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* country_name = furi_string_alloc();
    if(nfc_emv_parser_get_country_name(storage, country_code, country_name)) {
        furi_string_cat_printf(str, "Country: %s\n", furi_string_get_cstr(country_name));
    }
    furi_string_free(country_name);
    furi_record_close(RECORD_STORAGE);
}

void nfc_render_emv_name(const char* data, FuriString* str) {
    if(strlen(data) == 0) return;
    furi_string_cat_printf(str, "\e#");
    furi_string_cat(str, data);
    furi_string_cat_printf(str, "\n");
}

void nfc_render_emv_application(const EmvApplication* apl, FuriString* str) {
    const uint8_t len = apl->aid_len;

    if(!len) {
        furi_string_cat_printf(str, "No Pay Application found\n");
        return;
    }

    furi_string_cat_printf(str, "AID: ");
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* aid_name = furi_string_alloc();

    if(nfc_emv_parser_get_aid_name(storage, apl->aid, len, aid_name)) {
        furi_string_cat_printf(str, "%s", furi_string_get_cstr(aid_name));
    } else {
        for(uint8_t i = 0; i < len; i++) furi_string_cat_printf(str, "%02X", apl->aid[i]);
    }

    furi_string_cat_printf(str, "\n");
    furi_string_free(aid_name);
    furi_record_close(RECORD_STORAGE);
}

static void nfc_render_emv_pin_try_counter(uint8_t counter, FuriString* str) {
    if(counter == 0xff) return;
    furi_string_cat_printf(str, "PIN try left: %d\n", counter);
}

void nfc_render_emv_transactions(const EmvApplication* apl, FuriString* str) {
    if(apl->transaction_counter)
        furi_string_cat_printf(str, "Transactions: %d\n", apl->transaction_counter);
    if(apl->last_online_atc)
        furi_string_cat_printf(str, "Last Online ATC: %d\n", apl->last_online_atc);

    const uint8_t len = apl->active_tr;
    if(!len) {
        furi_string_cat_printf(str, "No transactions info\n");
        return;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* tmp = furi_string_alloc();

    //furi_string_cat_printf(str, "Transactions:\n");
    for(int i = 0; i < len; i++) {
        if(!apl->trans[i].amount) continue;
        // transaction counter
        furi_string_cat_printf(str, "\e#%d: ", apl->trans[i].atc);

        // Print transaction amount
        uint8_t* a = (uint8_t*)&apl->trans[i].amount;
        bool top = true;
        for(int x = 0; x < 6; x++) {
            // cents
            if(x == 5) {
                furi_string_cat_printf(str, ".%02X", a[x]);
                break;
            }
            if(a[x]) {
                if(top) {
                    furi_string_cat_printf(str, "%X", a[x]);
                    top = false;
                } else {
                    furi_string_cat_printf(str, "%02X", a[x]);
                }
            }
        }

        if(apl->trans[i].currency) {
            furi_string_set_str(tmp, "UNK");
            nfc_emv_parser_get_currency_name(storage, apl->trans[i].currency, tmp);
            furi_string_cat_printf(str, " %s\n", furi_string_get_cstr(tmp));
        }

        if(apl->trans[i].country) {
            furi_string_set_str(tmp, "UNK");
            nfc_emv_parser_get_country_name(storage, apl->trans[i].country, tmp);
            furi_string_cat_printf(str, "Country: %s\n", furi_string_get_cstr(tmp));
        }

        if(apl->trans[i].date)
            furi_string_cat_printf(
                str,
                "%02lx/%02lx/%02lx ",
                apl->trans[i].date >> 16,
                (apl->trans[i].date >> 8) & 0xff,
                apl->trans[i].date & 0xff);

        if(apl->trans[i].time)
            furi_string_cat_printf(
                str,
                "%02lx:%02lx:%02lx\n",
                apl->trans[i].time & 0xff,
                (apl->trans[i].time >> 8) & 0xff,
                apl->trans[i].time >> 16);
    }

    furi_string_free(tmp);
    furi_record_close(RECORD_STORAGE);
}

void nfc_render_emv_extra(const EmvData* data, FuriString* str) {
    nfc_render_emv_currency(data->emv_application.currency_code, str);
    nfc_render_emv_country(data->emv_application.country_code, str);
    nfc_render_emv_application(&data->emv_application, str);
    nfc_render_emv_pin_try_counter(data->emv_application.pin_try_counter, str);
}
