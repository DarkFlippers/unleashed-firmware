#include <furi.h>

#include <gui/gui.h>
#include <gui/view_holder.h>
#include <gui/modules/empty_screen.h>

#include <dialogs/dialogs.h>
#include <assets_icons.h>

#include <furi_hal_version.h>
#include <furi_hal_region.h>
#include <furi_hal_bt.h>
#include <furi_hal_info.h>

typedef DialogMessageButton (*AboutDialogScreen)(DialogsApp* dialogs, DialogMessage* message);

static DialogMessageButton about_screen_product(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    FuriString* screen_header = furi_string_alloc_printf(
        "Product: %s\n"
        "Model: %s",
        furi_hal_version_get_model_name(),
        furi_hal_version_get_model_code());

    FuriString* screen_text = furi_string_alloc_printf(
        "FCC ID: %s\n"
        "IC: %s",
        furi_hal_version_get_fcc_id(),
        furi_hal_version_get_ic_id());

    dialog_message_set_header(
        message, furi_string_get_cstr(screen_header), 0, 0, AlignLeft, AlignTop);
    dialog_message_set_text(
        message, furi_string_get_cstr(screen_text), 0, 26, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);

    furi_string_free(screen_header);
    furi_string_free(screen_text);

    return result;
}

static DialogMessageButton about_screen_address(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    const char* screen_text = "Flipper Devices Inc.\n"
                              "Suite B #551, 2803\n"
                              "Philadelphia Pike, Claymont\n"
                              "DE, USA 19703\n";

    dialog_message_set_text(message, screen_text, 0, 0, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_compliance(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    const char* screen_text = "For all compliance\n"
                              "certificates, please visit:\n"
                              "www.flipp.dev/compliance";

    dialog_message_set_text(message, screen_text, 0, 0, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_icon1(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_Certification1_103x56, 13, 0);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_icon2(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_Certification2_46x33, 15, 10);
    dialog_message_set_text(
        message, furi_hal_version_get_mic_id(), 63, 27, AlignLeft, AlignCenter);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_cert_china_0(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_CertificationChina0_121x41, 3, 3);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_cert_china_1(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_CertificationChina1_124x47, 3, 3);
    dialog_message_set_text(
        message, furi_hal_version_get_srrc_id(), 55, 11, AlignLeft, AlignBottom);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_cert_taiwan(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_CertificationTaiwan_33x32, 3, 10);
    dialog_message_set_text(
        message, furi_hal_version_get_ncc_id(), 39, 30, AlignLeft, AlignBottom);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_cert_mexico(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_CertificationMexico_98x41, 17, 4);
    result = dialog_message_show(dialogs, message);

    return result;
}

static DialogMessageButton about_screen_hw_version(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;
    FuriString* buffer;
    buffer = furi_string_alloc();
    const char* my_name = furi_hal_version_get_name_ptr();

    furi_string_cat_printf(
        buffer,
        "%d.F%dB%dC%d %s:%s %s\n",
        furi_hal_version_get_hw_version(),
        furi_hal_version_get_hw_target(),
        furi_hal_version_get_hw_body(),
        furi_hal_version_get_hw_connect(),
        furi_hal_version_get_hw_region_name(),
        furi_hal_region_get_name(),
        my_name ? my_name : "Unknown");

    furi_string_cat_printf(buffer, "Serial Number:\n");
    const uint8_t* uid = furi_hal_version_uid();
    for(size_t i = 0; i < furi_hal_version_uid_size(); i++) {
        furi_string_cat_printf(buffer, "%02X", uid[i]);
    }

    dialog_message_set_header(message, "HW Version Info:", 0, 0, AlignLeft, AlignTop);
    dialog_message_set_text(message, furi_string_get_cstr(buffer), 0, 13, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    furi_string_free(buffer);

    return result;
}

static DialogMessageButton about_screen_fw_version(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;
    FuriString* buffer;
    buffer = furi_string_alloc();
    const Version* ver = furi_hal_version_get_firmware_version();
    const BleGlueC2Info* c2_ver = NULL;
#ifdef SRV_BT
    c2_ver = ble_glue_get_c2_info();
#endif

    if(!ver) { //-V1051
        furi_string_cat_printf(buffer, "No info\n");
    } else {
        uint16_t api_major, api_minor;
        furi_hal_info_get_api_version(&api_major, &api_minor);
        furi_string_cat_printf(
            buffer,
            "%s [%s]\n%s%s [%d.%d] %s\n[%d] %s",
            version_get_version(ver),
            version_get_builddate(ver),
            version_get_dirty_flag(ver) ? "[!] " : "",
            version_get_githash(ver),
            api_major,
            api_minor,
            c2_ver ? c2_ver->StackTypeString : "<none>",
            version_get_target(ver),
            version_get_gitbranch(ver));
    }

    dialog_message_set_header(message, "FW Version Info:", 0, 0, AlignLeft, AlignTop);
    dialog_message_set_text(message, furi_string_get_cstr(buffer), 0, 13, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    furi_string_free(buffer);

    return result;
}

const AboutDialogScreen about_screens[] = {
    about_screen_product,
    about_screen_compliance,
    about_screen_address,
    about_screen_icon1,
    about_screen_icon2,
    about_screen_cert_china_0,
    about_screen_cert_china_1,
    about_screen_cert_taiwan,
    about_screen_cert_mexico,
    about_screen_hw_version,
    about_screen_fw_version,
};

int32_t about_settings_app(void* p) {
    UNUSED(p);
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewHolder* view_holder = view_holder_alloc();
    EmptyScreen* empty_screen = empty_screen_alloc();

    size_t screen_index = 0;
    DialogMessageButton screen_result;

    // draw empty screen to prevent menu flickering
    view_holder_attach_to_gui(view_holder, gui);
    view_holder_set_view(view_holder, empty_screen_get_view(empty_screen));

    while(1) {
        if(screen_index >= COUNT_OF(about_screens) - 1) {
            dialog_message_set_buttons(message, "Prev.", NULL, NULL);
        } else if(screen_index == 0) {
            dialog_message_set_buttons(message, NULL, NULL, "Next");
        } else {
            dialog_message_set_buttons(message, "Prev.", NULL, "Next");
        }

        screen_result = about_screens[screen_index](dialogs, message);

        dialog_message_set_icon(message, NULL, 0, 0);
        dialog_message_set_header(message, NULL, 0, 0, AlignLeft, AlignTop);
        dialog_message_set_text(message, NULL, 0, 0, AlignLeft, AlignTop);

        if(screen_result == DialogMessageButtonLeft) {
            if(screen_index <= 0) {
                break;
            } else {
                screen_index--;
            }
        } else if(screen_result == DialogMessageButtonRight) {
            if(screen_index < COUNT_OF(about_screens) - 1) {
                screen_index++;
            }
        } else if(screen_result == DialogMessageButtonBack) {
            break;
        }
    }

    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);

    view_holder_set_view(view_holder, NULL);
    view_holder_free(view_holder);
    empty_screen_free(empty_screen);
    furi_record_close(RECORD_GUI);

    return 0;
}
