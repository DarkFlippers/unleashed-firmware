#include "../nfc_magic_i.h"

static bool nfc_magic_scene_file_select_is_file_suitable(NfcMagic* nfc_magic) {
    NfcDevice* nfc_dev = nfc_magic->source_dev;
    if(nfc_dev->format == NfcDeviceSaveFormatMifareClassic) {
        switch(nfc_magic->dev->type) {
        case MagicTypeClassicGen1:
        case MagicTypeClassicDirectWrite:
        case MagicTypeClassicAPDU:
            if((nfc_dev->dev_data.mf_classic_data.type != MfClassicType1k) ||
               (nfc_dev->dev_data.nfc_data.uid_len != nfc_magic->dev->uid_len)) {
                return false;
            }
            return true;

        case MagicTypeGen4:
            return true;
        default:
            return false;
        }
    } else if(
        (nfc_dev->format == NfcDeviceSaveFormatMifareUl) &&
        (nfc_dev->dev_data.nfc_data.uid_len == 7)) {
        switch(nfc_magic->dev->type) {
        case MagicTypeUltralightGen1:
        case MagicTypeUltralightDirectWrite:
        case MagicTypeUltralightC_Gen1:
        case MagicTypeUltralightC_DirectWrite:
        case MagicTypeGen4:
            switch(nfc_dev->dev_data.mf_ul_data.type) {
            case MfUltralightTypeNTAGI2C1K:
            case MfUltralightTypeNTAGI2C2K:
            case MfUltralightTypeNTAGI2CPlus1K:
            case MfUltralightTypeNTAGI2CPlus2K:
                return false;
            default:
                return true;
            }
        default:
            return false;
        }
    }

    return false;
}

void nfc_magic_scene_file_select_on_enter(void* context) {
    NfcMagic* nfc_magic = context;
    // Process file_select return
    nfc_device_set_loading_callback(
        nfc_magic->source_dev, nfc_magic_show_loading_popup, nfc_magic);

    if(!furi_string_size(nfc_magic->source_dev->load_path)) {
        furi_string_set_str(nfc_magic->source_dev->load_path, NFC_APP_FOLDER);
    }
    if(nfc_file_select(nfc_magic->source_dev)) {
        if(nfc_magic_scene_file_select_is_file_suitable(nfc_magic)) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneWriteConfirm);
        } else {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneWrongCard);
        }
    } else {
        scene_manager_previous_scene(nfc_magic->scene_manager);
    }
}

bool nfc_magic_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_magic_scene_file_select_on_exit(void* context) {
    NfcMagic* nfc_magic = context;
    nfc_device_set_loading_callback(nfc_magic->source_dev, NULL, nfc_magic);
}
