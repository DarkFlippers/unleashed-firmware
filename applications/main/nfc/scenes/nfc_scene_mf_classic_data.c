#include "../nfc_i.h"

void nfc_scene_mf_classic_data_on_enter(void* context) {
    Nfc* nfc = context;
    MfClassicType type = nfc->dev->dev_data.mf_classic_data.type;
    MfClassicData* data = &nfc->dev->dev_data.mf_classic_data;
    TextBox* text_box = nfc->text_box;

    text_box_set_font(text_box, TextBoxFontHex);

    int card_blocks = 0;
    if(type == MfClassicType1k) {
        card_blocks = MF_CLASSIC_1K_TOTAL_SECTORS_NUM * 4;
    } else if(type == MfClassicType4k) {
        // 16 sectors of 4 blocks each plus 8 sectors of 16 blocks each
        card_blocks = MF_CLASSIC_1K_TOTAL_SECTORS_NUM * 4 + 8 * 16;
    } else if(type == MfClassicTypeMini) {
        card_blocks = MF_MINI_TOTAL_SECTORS_NUM * 4;
    }

    int bytes_written = 0;
    for(int block_num = 0; block_num < card_blocks; block_num++) {
        bool is_sec_trailer = mf_classic_is_sector_trailer(block_num);
        if(is_sec_trailer) {
            uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
            MfClassicSectorTrailer* sec_tr =
                mf_classic_get_sector_trailer_by_sector(data, sector_num);
            // Key A
            for(size_t i = 0; i < sizeof(sec_tr->key_a); i += 2) {
                if((bytes_written % 8 == 0) && (bytes_written != 0)) {
                    furi_string_push_back(nfc->text_box_store, '\n');
                }
                if(mf_classic_is_key_found(data, sector_num, MfClassicKeyA)) {
                    furi_string_cat_printf(
                        nfc->text_box_store, "%02X%02X ", sec_tr->key_a[i], sec_tr->key_a[i + 1]);
                } else {
                    furi_string_cat_printf(nfc->text_box_store, "???? ");
                }
                bytes_written += 2;
            }
            // Access bytes
            for(size_t i = 0; i < MF_CLASSIC_ACCESS_BYTES_SIZE; i += 2) {
                if((bytes_written % 8 == 0) && (bytes_written != 0)) {
                    furi_string_push_back(nfc->text_box_store, '\n');
                }
                if(mf_classic_is_block_read(data, block_num)) {
                    furi_string_cat_printf(
                        nfc->text_box_store,
                        "%02X%02X ",
                        sec_tr->access_bits[i],
                        sec_tr->access_bits[i + 1]);
                } else {
                    furi_string_cat_printf(nfc->text_box_store, "???? ");
                }
                bytes_written += 2;
            }
            // Key B
            for(size_t i = 0; i < sizeof(sec_tr->key_b); i += 2) {
                if((bytes_written % 8 == 0) && (bytes_written != 0)) {
                    furi_string_push_back(nfc->text_box_store, '\n');
                }
                if(mf_classic_is_key_found(data, sector_num, MfClassicKeyB)) {
                    furi_string_cat_printf(
                        nfc->text_box_store, "%02X%02X ", sec_tr->key_b[i], sec_tr->key_b[i + 1]);
                } else {
                    furi_string_cat_printf(nfc->text_box_store, "???? ");
                }
                bytes_written += 2;
            }
        } else {
            // Write data block
            for(size_t i = 0; i < MF_CLASSIC_BLOCK_SIZE; i += 2) {
                if((bytes_written % 8 == 0) && (bytes_written != 0)) {
                    furi_string_push_back(nfc->text_box_store, '\n');
                }
                if(mf_classic_is_block_read(data, block_num)) {
                    furi_string_cat_printf(
                        nfc->text_box_store,
                        "%02X%02X ",
                        data->block[block_num].value[i],
                        data->block[block_num].value[i + 1]);
                } else {
                    furi_string_cat_printf(nfc->text_box_store, "???? ");
                }
                bytes_written += 2;
            }
        }
    }
    text_box_set_text(text_box, furi_string_get_cstr(nfc->text_box_store));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
}

bool nfc_scene_mf_classic_data_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_mf_classic_data_on_exit(void* context) {
    Nfc* nfc = context;

    // Clean view
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
}
