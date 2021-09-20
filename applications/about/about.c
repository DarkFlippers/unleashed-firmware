#include <furi.h>
#include <dialogs/dialogs.h>

int32_t about_settings_app(void* p) {
    const char* first_screen_text = "Product: Flipper Zero\n"
                                    "Model: FZ.1\n"
                                    "FCC ID: 2A2V6-FZIC\n"
                                    "ID: 27624-FZ";

    const char* second_screen_text = "Flipper Devices Inc\n"
                                     "Suite B #551, 2803\n"
                                     "Philadelphia Pike, Claymont\n"
                                     "DE, USA 19703\n";

    const char* third_screen_text = "For all compliance\n"
                                    "certificates please visit\n"
                                    "www.flipp.dev/compliance";

    DialogsApp* dialogs = furi_record_open("dialogs");
    DialogMessage* message = dialog_message_alloc();

    do {
        dialog_message_set_buttons(message, NULL, NULL, "Next");

        dialog_message_set_text(message, first_screen_text, 0, 0, AlignLeft, AlignTop);
        if(dialog_message_show(dialogs, message) != DialogMessageButtonRight) break;

        dialog_message_set_text(message, second_screen_text, 0, 0, AlignLeft, AlignTop);
        if(dialog_message_show(dialogs, message) != DialogMessageButtonRight) break;

        dialog_message_set_text(message, third_screen_text, 0, 0, AlignLeft, AlignTop);
        if(dialog_message_show(dialogs, message) != DialogMessageButtonRight) break;

        dialog_message_set_text(message, NULL, 0, 0, AlignLeft, AlignTop);

        dialog_message_set_icon(message, &I_Certification_128x64, 0, 0);
        dialog_message_set_buttons(message, NULL, NULL, NULL);
        dialog_message_show(dialogs, message);
    } while(false);

    dialog_message_free(message);
    furi_record_close("dialogs");

    return 0;
}