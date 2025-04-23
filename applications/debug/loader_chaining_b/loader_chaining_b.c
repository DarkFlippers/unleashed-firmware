#include <furi.h>
#include <dialogs/dialogs.h>
#include <loader/loader.h>

int32_t chaining_test_app_b(const char* arg) {
    if(!arg) return 0;

    Loader* loader = furi_record_open(RECORD_LOADER);
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Hi, I am B", 64, 0, AlignCenter, AlignTop);
    FuriString* text = furi_string_alloc_printf("And A told me:\n%s", arg);
    dialog_message_set_text(message, furi_string_get_cstr(text), 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, "Just quit", NULL, "Launch A");
    DialogMessageButton result = dialog_message_show(dialogs, message);
    dialog_message_free(message);
    furi_string_free(text);

    if(result == DialogMessageButtonRight)
        loader_enqueue_launch(
            loader, "/ext/apps/Debug/loader_chaining_a.fap", NULL, LoaderDeferredLaunchFlagGui);

    furi_record_close(RECORD_LOADER);
    furi_record_close(RECORD_DIALOGS);
    return 0;
}
