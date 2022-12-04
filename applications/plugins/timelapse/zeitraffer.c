#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
//#include "gpio/gpio_item.h"
#include "gpio_item.h"

// Часть кода покрадена из https://github.com/zmactep/flipperzero-hello-world

int Time = 10; // Таймер
int Count = 10; // Количество кадров
int WorkTime = 0; // Счётчик таймера
int WorkCount = 0; // Счётчик кадров
bool InfiniteShot = false; // Бесконечная съёмка
bool Bulb = false; // Режим BULB
int Backlight = 0; // Подсветка: вкл/выкл/авто

const NotificationSequence sequence_click = {
    &message_note_c7,
    &message_delay_50,
    &message_sound_off,
    NULL,
};

typedef enum {
    EventTypeTick,
    EventTypeInput,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} ZeitrafferEvent;

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    char temp_str[36];
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    switch(Count) {
    case -1:
        snprintf(temp_str, sizeof(temp_str), "Set: BULB %i sec", Time);
        break;
    case 0:
        snprintf(temp_str, sizeof(temp_str), "Set: infinite, %i sec", Time);
        break;
    default:
        snprintf(temp_str, sizeof(temp_str), "Set: %i frames, %i sec", Count, Time);
    }
    canvas_draw_str(canvas, 3, 15, temp_str);
    snprintf(temp_str, sizeof(temp_str), "Left: %i frames, %i sec", WorkCount, WorkTime);
    canvas_draw_str(canvas, 3, 35, temp_str);

    switch(Backlight) {
    case 1:
        canvas_draw_str(canvas, 3, 55, "Backlight: ON");
        break;
    case 2:
        canvas_draw_str(canvas, 3, 55, "Backlight: OFF");
        break;
    default:
        canvas_draw_str(canvas, 3, 55, "Backlight: AUTO");
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    // Проверяем, что контекст не нулевой
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;

    ZeitrafferEvent event = {.type = EventTypeInput, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void timer_callback(FuriMessageQueue* event_queue) {
    // Проверяем, что контекст не нулевой
    furi_assert(event_queue);

    ZeitrafferEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t zeitraffer_app(void* p) {
    UNUSED(p);

    // Текущее событие типа кастомного типа ZeitrafferEvent
    ZeitrafferEvent event;
    // Очередь событий на 8 элементов размера ZeitrafferEvent
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(ZeitrafferEvent));

    // Создаем новый view port
    ViewPort* view_port = view_port_alloc();
    // Создаем callback отрисовки, без контекста
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    // Создаем callback нажатий на клавиши, в качестве контекста передаем
    // нашу очередь сообщений, чтоб запихивать в неё эти события
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Создаем GUI приложения
    Gui* gui = furi_record_open(RECORD_GUI);
    // Подключаем view port к GUI в полноэкранном режиме
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Конфигурим пины
    gpio_item_configure_all_pins(GpioModeOutputPushPull);

    // Создаем периодический таймер с коллбэком, куда в качестве
    // контекста будет передаваться наша очередь событий
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    // Запускаем таймер
    //furi_timer_start(timer, 1500);

    // Включаем нотификации
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);

    // Бесконечный цикл обработки очереди событий
    while(1) {
        // Выбираем событие из очереди в переменную event (ждем бесконечно долго, если очередь пуста)
        // и проверяем, что у нас получилось это сделать
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);

        // Наше событие — это нажатие кнопки
        if(event.type == EventTypeInput) {
            if(event.input.type == InputTypeShort) { // Короткие нажатия

                if(event.input.key == InputKeyBack) {
                    if(furi_timer_is_running(
                           timer)) { // Если таймер запущен - нефиг мацать кнопки!
                        notification_message(notifications, &sequence_error);
                    } else {
                        WorkCount = Count;
                        WorkTime = 3;
                        if(Count == 0) {
                            InfiniteShot = true;
                            WorkCount = 1;
                        } else
                            InfiniteShot = false;

                        notification_message(notifications, &sequence_success);
                    }
                }
                if(event.input.key == InputKeyRight) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Count++;
                        notification_message(notifications, &sequence_click);
                    }
                }
                if(event.input.key == InputKeyLeft) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Count--;
                        notification_message(notifications, &sequence_click);
                    }
                }
                if(event.input.key == InputKeyUp) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Time++;
                        notification_message(notifications, &sequence_click);
                    }
                }
                if(event.input.key == InputKeyDown) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Time--;
                        notification_message(notifications, &sequence_click);
                    }
                }
                if(event.input.key == InputKeyOk) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_click);
                        furi_timer_stop(timer);
                    } else {
                        furi_timer_start(timer, 1000);

                        if(WorkCount == 0) WorkCount = Count;

                        if(WorkTime == 0) WorkTime = 3;

                        if(Count == 0) {
                            InfiniteShot = true;
                            WorkCount = 1;
                        } else
                            InfiniteShot = false;

                        if(Count == -1) {
                            gpio_item_set_pin(4, true);
                            gpio_item_set_pin(5, true);
                            Bulb = true;
                            WorkCount = 1;
                            WorkTime = Time;
                        } else
                            Bulb = false;

                        notification_message(notifications, &sequence_success);
                    }
                }
            }
            if(event.input.type == InputTypeLong) { // Длинные нажатия
                // Если нажата кнопка "назад", то выходим из цикла, а следовательно и из приложения
                if(event.input.key == InputKeyBack) {
                    if(furi_timer_is_running(timer)) { // А если работает таймер - не выходим :D
                        notification_message(notifications, &sequence_error);
                    } else {
                        notification_message(notifications, &sequence_click);
                        gpio_item_set_all_pins(false);
                        furi_timer_stop(timer);
                        notification_message(
                            notifications, &sequence_display_backlight_enforce_auto);
                        break;
                    }
                }
                if(event.input.key == InputKeyOk) {
                    // Нам ваша подсветка и нахой не нужна! Или нужна?
                    Backlight++;
                    if(Backlight > 2) Backlight = 0;
                }
            }

            if(event.input.type == InputTypeRepeat) { // Зажатые кнопки
                if(event.input.key == InputKeyRight) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Count = Count + 10;
                    }
                }
                if(event.input.key == InputKeyLeft) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Count = Count - 10;
                    }
                }
                if(event.input.key == InputKeyUp) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Time = Time + 10;
                    }
                }
                if(event.input.key == InputKeyDown) {
                    if(furi_timer_is_running(timer)) {
                        notification_message(notifications, &sequence_error);
                    } else {
                        Time = Time - 10;
                    }
                }
            }
        }

        // Наше событие — это сработавший таймер
        else if(event.type == EventTypeTick) {
            WorkTime--;

            if(WorkTime < 1) { // фоткаем
                notification_message(notifications, &sequence_blink_white_100);
                if(Bulb) {
                    gpio_item_set_all_pins(false);
                    WorkCount = 0;
                } else {
                    WorkCount--;
                    view_port_update(view_port);
                    notification_message(notifications, &sequence_click);
                    // Дрыгаем ногами
                    //gpio_item_set_all_pins(true);
                    gpio_item_set_pin(4, true);
                    gpio_item_set_pin(5, true);
                    furi_delay_ms(400); // На короткие нажатия фотик плохо реагирует
                    gpio_item_set_pin(4, false);
                    gpio_item_set_pin(5, false);
                    //gpio_item_set_all_pins(false);

                    if(InfiniteShot) WorkCount++;

                    WorkTime = Time;
                    view_port_update(view_port);
                }
            } else {
                // Отправляем нотификацию мигания синим светодиодом
                notification_message(notifications, &sequence_blink_blue_100);
            }

            if(WorkCount < 1) { // закончили
                gpio_item_set_all_pins(false);
                furi_timer_stop(timer);
                notification_message(notifications, &sequence_audiovisual_alert);
                WorkTime = 3;
                WorkCount = 0;
            }

            switch(Backlight) { // чо по подсветке?
            case 1:
                notification_message(notifications, &sequence_display_backlight_on);
                break;
            case 2:
                notification_message(notifications, &sequence_display_backlight_off);
                break;
            default:
                notification_message(notifications, &sequence_display_backlight_enforce_auto);
            }
        }
        if(Time < 1) Time = 1; // Не даём открутить таймер меньше единицы
        if(Count < -1)
            Count = 0; // А тут даём, бо 0 кадров это бесконечная съёмка, а -1 кадров - BULB
    }

    // Очищаем таймер
    furi_timer_free(timer);

    // Специальная очистка памяти, занимаемой очередью
    furi_message_queue_free(event_queue);

    // Чистим созданные объекты, связанные с интерфейсом
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    // Очищаем нотификации
    furi_record_close(RECORD_NOTIFICATION);

    return 0;
}
