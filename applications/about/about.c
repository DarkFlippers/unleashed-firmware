#include <furi.h>
#include <dialogs/dialogs.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/empty_screen.h>
#include <m-string.h>
#include <furi_hal_version.h>
#include <furi_hal_bt.h>

typedef DialogMessageButton (*AboutDialogScreen)(DialogsApp* dialogs, DialogMessage* message);

static DialogMessageButton product_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    const char* screen_header = "Product: Flipper Zero\n"
                                "Model: FZ.1\n";
    const char* screen_text = "FCC ID: 2A2V6-FZ\n"
                              "IC: 27624-FZ";

    dialog_message_set_header(message, screen_header, 0, 0, AlignLeft, AlignTop);
    dialog_message_set_text(message, screen_text, 0, 26, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_header(message, NULL, 0, 0, AlignLeft, AlignTop);
    dialog_message_set_text(message, NULL, 0, 0, AlignLeft, AlignTop);

    return result;
}

static DialogMessageButton address_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    const char* screen_text = "Flipper Devices Inc\n"
                              "Suite B #551, 2803\n"
                              "Philadelphia Pike, Claymont\n"
                              "DE, USA 19703\n";

    dialog_message_set_text(message, screen_text, 0, 0, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_text(message, NULL, 0, 0, AlignLeft, AlignTop);

    return result;
}

static DialogMessageButton compliance_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    const char* screen_text = "For all compliance\n"
                              "certificates please visit:\n"
                              "www.flipp.dev/compliance";

    dialog_message_set_text(message, screen_text, 0, 0, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_text(message, NULL, 0, 0, AlignLeft, AlignTop);

    return result;
}

static DialogMessageButton icon1_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_Certification1_103x56, 13, 0);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_icon(message, NULL, 0, 0);

    return result;
}

static DialogMessageButton icon2_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_Certification2_98x33, 15, 10);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_icon(message, NULL, 0, 0);

    return result;
}

static DialogMessageButton hw_version_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;
    string_t buffer;
    string_init(buffer);
    const char* my_name = furi_hal_version_get_name_ptr();

    string_cat_printf(
        buffer,
        "%d.F%dB%dC%d %s %s\n",
        furi_hal_version_get_hw_version(),
        furi_hal_version_get_hw_target(),
        furi_hal_version_get_hw_body(),
        furi_hal_version_get_hw_connect(),
        furi_hal_version_get_hw_region_name(),
        my_name ? my_name : "Unknown");

    string_cat_printf(buffer, "Serial Number:\n");
    const uint8_t* uid = furi_hal_version_uid();
    for(size_t i = 0; i < furi_hal_version_uid_size(); i++) {
        string_cat_printf(buffer, "%02X", uid[i]);
    }

    dialog_message_set_header(message, "HW Version Info:", 0, 0, AlignLeft, AlignTop);
    dialog_message_set_text(message, string_get_cstr(buffer), 0, 13, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_text(message, NULL, 0, 0, AlignLeft, AlignTop);
    dialog_message_set_header(message, NULL, 0, 0, AlignLeft, AlignTop);
    string_clear(buffer);

    return result;
}

static DialogMessageButton fw_version_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;
    string_t buffer;
    string_init(buffer);
    const Version* ver = furi_hal_version_get_firmware_version();
    const BleGlueC2Info* c2_ver = NULL;
#ifdef SRV_BT
    c2_ver = ble_glue_get_c2_info();
#endif

    if(!ver) {
        string_cat_printf(buffer, "No info\n");
    } else {
        string_cat_printf(
            buffer,
            "%s [%s]\n%s%s [%s] %s\n[%d] %s",
            version_get_version(ver),
            version_get_builddate(ver),
            version_get_dirty_flag(ver) ? "[!] " : "",
            version_get_githash(ver),
            version_get_gitbranchnum(ver),
            c2_ver ? c2_ver->StackTypeString : "<none>",
            version_get_target(ver),
            version_get_gitbranch(ver));
    }

    dialog_message_set_header(message, "FW Version Info:", 0, 0, AlignLeft, AlignTop);
    dialog_message_set_text(message, string_get_cstr(buffer), 0, 13, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_text(message, NULL, 0, 0, AlignLeft, AlignTop);
    dialog_message_set_header(message, NULL, 0, 0, AlignLeft, AlignTop);
    string_clear(buffer);

    return result;
}

const AboutDialogScreen about_screens[] = {
    product_screen,
    compliance_screen,
    address_screen,
    icon1_screen,
    icon2_screen,
    hw_version_screen,
    fw_version_screen};

const size_t about_screens_count = sizeof(about_screens) / sizeof(AboutDialogScreen);

int32_t about_settings_app(void* p) {
    UNUSED(p);
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewDispatcher* view_dispatcher = view_dispatcher_alloc();
    EmptyScreen* empty_screen = empty_screen_alloc();
    const uint32_t empty_screen_index = 0;

    size_t screen_index = 0;
    DialogMessageButton screen_result;

    // draw empty screen to prevent menu flickering
    view_dispatcher_add_view(
        view_dispatcher, empty_screen_index, empty_screen_get_view(empty_screen));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(view_dispatcher, empty_screen_index);

    while(1) {
        if(screen_index >= about_screens_count - 1) {
            dialog_message_set_buttons(message, "Back", NULL, NULL);
        } else {
            dialog_message_set_buttons(message, "Back", NULL, "Next");
        }

        screen_result = about_screens[screen_index](dialogs, message);

        if(screen_result == DialogMessageButtonLeft) {
            if(screen_index <= 0) {
                break;
            } else {
                screen_index--;
            }
        } else if(screen_result == DialogMessageButtonRight) {
            if(screen_index < about_screens_count) {
                screen_index++;
            }
        } else if(screen_result == DialogMessageButtonBack) {
            break;
        }
    }

    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);

    view_dispatcher_remove_view(view_dispatcher, empty_screen_index);
    view_dispatcher_free(view_dispatcher);
    empty_screen_free(empty_screen);
    furi_record_close(RECORD_GUI);

    return 0;
}