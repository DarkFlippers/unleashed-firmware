#include "../nfc_i.h"
#include <dolphin/dolphin.h>

const char months[13][4] = {
    "---",
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC",
};

void nfc_scene_passport_read_auth_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_passport_read_auth_on_enter(void* context) {
    Nfc* nfc = context;
    MrtdData* mrtd_data = &nfc->dev->dev_data.mrtd_data;

    Widget* widget = nfc->widget;

    // Setup Custom Widget view
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    furi_string_set(temp_str, "\e#Passport\n");
    furi_string_cat_printf(
        temp_str, "Auth.method: %s\n", mrtd_auth_method_string(mrtd_data->auth_method_used));
    // TODO: indicate BAC / PACE used

    uint16_t lds_version = mrtd_data->files.EF_COM.lds_version;
    furi_string_cat_printf(temp_str, "LDS version: %d.%d\n", lds_version / 100, lds_version % 100);

    uint32_t unicode_version = mrtd_data->files.EF_COM.unicode_version;
    furi_string_cat_printf(
        temp_str,
        "Unicode version: %d.%d.%d\n",
        (uint8_t)(unicode_version / 10000),
        (uint8_t)(unicode_version / 100 % 100),
        (uint8_t)(unicode_version % 100));

    furi_string_cat_printf(temp_str, "Avail.files: ");
    for(size_t i = 0; i < MAX_EFCOM_TAGS; ++i) {
        uint8_t tag = mrtd_data->files.EF_COM.tag_list[i];
        const EFFile* file = mrtd_tag_to_file(tag);
        if(file->tag) {
            if(i > 0) furi_string_cat_printf(temp_str, ", ");
            furi_string_cat_printf(temp_str, "%s", file->name);
        }
    }
    furi_string_cat_printf(temp_str, "\n");

    EF_DIR_contents* EF_DIR = &mrtd_data->files.EF_DIR;
    if(EF_DIR->applications_count > 0) {
        furi_string_cat_printf(temp_str, "Apps:\n");
        for(uint8_t i = 0; i < EF_DIR->applications_count; ++i) {
            for(uint8_t n = 0; n < sizeof(AIDValue); ++n) {
                furi_string_cat_printf(temp_str, "%02X ", EF_DIR->applications[i][n]);
            }
            furi_string_cat_printf(temp_str, "\n");
        }
    }

    EF_DG1_contents* DG1 = &mrtd_data->files.DG1;
    furi_string_cat_printf(temp_str, "\e#DG1\n");
    furi_string_cat_printf(temp_str, "Doc Type: %s\n", DG1->doctype);
    furi_string_cat_printf(temp_str, "Issuing State: %s\n", DG1->issuing_state);
    furi_string_cat_printf(temp_str, "Name: %s\n", DG1->name);
    furi_string_cat_printf(temp_str, "DocNr: %s\n", DG1->docnr);
    furi_string_cat_printf(temp_str, "Nationality: %s\n", DG1->nationality);
    furi_string_cat_printf(
        temp_str,
        "Birth Date: %02d %s %02d\n",
        DG1->birth_date.day,
        months[DG1->birth_date.month],
        DG1->birth_date.year);
    furi_string_cat_printf(temp_str, "Sex: %s\n", DG1->sex);
    furi_string_cat_printf(
        temp_str,
        "Expiry Date: %02d %s %02d\n",
        DG1->expiry_date.day,
        months[DG1->expiry_date.month],
        DG1->expiry_date.year);

    widget_add_text_scroll_element(widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Retry", nfc_scene_passport_read_auth_widget_callback, nfc);
    /*
    widget_add_button_element(
        nfc->widget, GuiButtonTypeCenter, "Auth", nfc_scene_passport_read_auth_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "More", nfc_scene_passport_read_auth_widget_callback, nfc);
    */

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_passport_read_auth_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            nfc->dev->dev_data.mrtd_data.auth_success = false;
            nfc->dev->dev_data.mrtd_data.auth.method = MrtdAuthMethodNone;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeCenter) {
            //scene_manager_next_scene(nfc->scene_manager, NfcScenePassportAuth);
            //consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            //scene_manager_next_scene(nfc->scene_manager, NfcScenePassportMenu);
            //consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_passport_read_auth_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    widget_reset(nfc->widget);
}
