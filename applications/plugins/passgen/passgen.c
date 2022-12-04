#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <stdlib.h>
#include <Password_Generator_icons.h>

#define PASSGEN_MAX_LENGTH 16
#define PASSGEN_CHARACTERS_LENGTH (26 * 4)

#define PASSGEN_DIGITS "0123456789"
#define PASSGEN_LETTERS_LOW "abcdefghijklmnopqrstuvwxyz"
#define PASSGEN_LETTERS_UP "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define PASSGEN_SPECIAL "!#$%^&*.-_"

typedef enum PassGen_Alphabet {
    Digits = 1,
    Lowercase = 2,

    Uppercase = 4,
    Special = 8,

    DigitsLower = Digits | Lowercase,
    DigitsAllLetters = Digits | Lowercase | Uppercase,
    Mixed = DigitsAllLetters | Special
} PassGen_Alphabet;

const int AlphabetLevels[] = {Digits, Lowercase, DigitsLower, DigitsAllLetters, Mixed};
const char* AlphabetLevelNames[] = {"1234", "abcd", "ab12", "Ab12", "Ab1#"};
const int AlphabetLevelsCount = sizeof(AlphabetLevels) / sizeof(int);

const NotificationSequence PassGen_Alert_vibro = {
    &message_vibro_on,
    &message_blue_255,
    &message_delay_50,
    &message_vibro_off,
    NULL,
};

typedef struct {
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;
    FuriMutex** mutex;
    NotificationApp* notify;
    char password[PASSGEN_MAX_LENGTH + 1];
    char alphabet[PASSGEN_CHARACTERS_LENGTH + 1];
    int length;
    int level;
} PassGen;

void state_free(PassGen* app) {
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);
    furi_message_queue_free(app->input_queue);
    furi_mutex_free(app->mutex);
    furi_record_close(RECORD_NOTIFICATION);
    free(app);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    PassGen* app = ctx;
    if(input_event->type == InputTypeShort) {
        furi_message_queue_put(app->input_queue, input_event, 0);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    char str_length[8];
    PassGen* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);
    canvas_draw_box(canvas, 0, 0, 128, 14);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 11, "Password Generator");

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str_aligned(canvas, 64, 35, AlignCenter, AlignCenter, app->password);

    // Navigation menu:
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_icon(canvas, 96, 52, &I_Pin_back_arrow_10x8);
    canvas_draw_str(canvas, 108, 60, "Exit");

    canvas_draw_icon(canvas, 54, 52, &I_Vertical_arrow_7x9);
    canvas_draw_str(canvas, 64, 60, AlphabetLevelNames[app->level]);

    snprintf(str_length, sizeof(str_length), "Len: %d", app->length);
    canvas_draw_icon(canvas, 4, 53, &I_Horizontal_arrow_9x7);
    canvas_draw_str(canvas, 15, 60, str_length);

    furi_mutex_release(app->mutex);
}

void build_alphabet(PassGen* app) {
    PassGen_Alphabet mode = AlphabetLevels[app->level];
    app->alphabet[0] = '\0';
    if((mode & Digits) != 0) strcat(app->alphabet, PASSGEN_DIGITS);
    if((mode & Lowercase) != 0) strcat(app->alphabet, PASSGEN_LETTERS_LOW);
    if((mode & Uppercase) != 0) strcat(app->alphabet, PASSGEN_LETTERS_UP);
    if((mode & Special) != 0) strcat(app->alphabet, PASSGEN_SPECIAL);
}

PassGen* state_init() {
    PassGen* app = malloc(sizeof(PassGen));
    app->length = 8;
    app->level = 2;
    build_alphabet(app);
    app->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->view_port = view_port_alloc();
    app->gui = furi_record_open(RECORD_GUI);
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    view_port_input_callback_set(app->view_port, input_callback, app);
    view_port_draw_callback_set(app->view_port, render_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->notify = furi_record_open(RECORD_NOTIFICATION);

    return app;
}

void generate(PassGen* app) {
    int hi = strlen(app->alphabet);
    for(int i = 0; i < app->length; i++) {
        int x = rand() % hi;
        app->password[i] = app->alphabet[x];
    }
    app->password[app->length] = '\0';
}

void update_password(PassGen* app, bool vibro) {
    generate(app);

    if(vibro)
        notification_message(app->notify, &PassGen_Alert_vibro);
    else
        notification_message(app->notify, &sequence_blink_blue_100);
    view_port_update(app->view_port);
}

int32_t passgenapp(void) {
    PassGen* app = state_init();
    generate(app);

    while(1) {
        InputEvent input;
        while(furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk) {
            furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

            if(input.type == InputTypeShort) {
                switch(input.key) {
                case InputKeyBack:
                    furi_mutex_release(app->mutex);
                    state_free(app);
                    return 0;
                case InputKeyDown:
                    if(app->level > 0) {
                        app->level--;
                        build_alphabet(app);
                        update_password(app, false);
                    } else
                        notification_message(app->notify, &sequence_blink_red_100);
                    break;
                case InputKeyUp:
                    if(app->level < AlphabetLevelsCount - 1) {
                        app->level++;
                        build_alphabet(app);
                        update_password(app, false);
                    } else
                        notification_message(app->notify, &sequence_blink_red_100);
                    break;
                case InputKeyLeft:
                    if(app->length > 1) {
                        app->length--;
                        update_password(app, false);
                    } else
                        notification_message(app->notify, &sequence_blink_red_100);
                    break;
                case InputKeyRight:
                    if(app->length < PASSGEN_MAX_LENGTH) {
                        app->length++;
                        update_password(app, false);
                    } else
                        notification_message(app->notify, &sequence_blink_red_100);
                    break;
                case InputKeyOk:
                    update_password(app, true);
                    break;
                default:
                    break;
                }
            }
            furi_mutex_release(app->mutex);
        }
    }
    state_free(app);
    return 0;
}
