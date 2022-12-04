#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <stdbool.h> // Header-file for boolean data-type.
#include <string.h> // Header-file for string functions.
#include "tinyexpr.h" // Header-file for the TinyExpr library.

#include <stdio.h>
#include <stdlib.h>

const short MAX_TEXT_LENGTH = 20;

typedef struct {
    short x;
    short y;
} selectedPosition;

typedef struct {
    selectedPosition position;
    //string with the inputted calculator text
    char text[20];
    short textLength;
    char log[20];
} Calculator;

char getKeyAtPosition(short x, short y) {
    if(x == 0 && y == 0) {
        return 'C';
    }
    if(x == 1 && y == 0) {
        return '<';
    }
    if(x == 2 && y == 0) {
        return '%';
    }
    if(x == 3 && y == 0) {
        return '/';
    }
    if(x == 0 && y == 1) {
        return '1';
    }
    if(x == 1 && y == 1) {
        return '2';
    }
    if(x == 2 && y == 1) {
        return '3';
    }
    if(x == 3 && y == 1) {
        return '*';
    }
    if(x == 0 && y == 2) {
        return '4';
    }
    if(x == 1 && y == 2) {
        return '5';
    }
    if(x == 2 && y == 2) {
        return '6';
    }
    if(x == 3 && y == 2) {
        return '-';
    }
    if(x == 0 && y == 3) {
        return '7';
    }
    if(x == 1 && y == 3) {
        return '8';
    }
    if(x == 2 && y == 3) {
        return '9';
    }
    if(x == 3 && y == 3) {
        return '+';
    }
    if(x == 0 && y == 4) {
        return '(';
    }
    if(x == 1 && y == 4) {
        return '0';
    }
    if(x == 2 && y == 4) {
        return '.';
    }
    if(x == 3 && y == 4) {
        return '=';
    }
    return ' ';
}

short calculateStringWidth(const char* str, short lenght) {
    /* widths:
        1 = 2
        2, 3, 4, 5, 6, 7, 8, 9, 0, X, -, +, . =  = 5
        %, / = 7
        S = 5
        (, ) = 3

    */
    short width = 0;
    for(short i = 0; i < lenght; i++) {
        switch(str[i]) {
        case '1':
            width += 2;
            break;
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        case '*':
        case '-':
        case '+':
        case '.':
            width += 5;
            break;
        case '%':
        case '/':
            width += 7;
            break;
        case 'S':
            width += 5;
            break;
        case '(':
        case ')':
            width += 3;
            break;
        default:
            break;
        }
        width += 1;
    }

    return width;
}

void generate_calculator_layout(Canvas* canvas) {
    //draw dotted lines
    for(int i = 0; i <= 64; i++) {
        if(i % 2 == 0) {
            canvas_draw_dot(canvas, i, 14);
            canvas_draw_dot(canvas, i, 33);
        }
        if(i % 2 == 1) {
            canvas_draw_dot(canvas, i, 15);
            canvas_draw_dot(canvas, i, 34);
        }
    }

    //draw horizontal lines
    canvas_draw_box(canvas, 0, 41, 64, 2);
    canvas_draw_box(canvas, 0, 57, 64, 2);
    canvas_draw_box(canvas, 0, 73, 64, 2);
    canvas_draw_box(canvas, 0, 89, 64, 2);
    canvas_draw_box(canvas, 0, 105, 64, 2);
    canvas_draw_box(canvas, 0, 121, 64, 2);

    //draw vertical lines
    canvas_draw_box(canvas, 0, 43, 1, 80);
    canvas_draw_box(canvas, 15, 43, 2, 80);
    canvas_draw_box(canvas, 31, 43, 2, 80);
    canvas_draw_box(canvas, 47, 43, 2, 80);
    canvas_draw_box(canvas, 63, 43, 1, 80);

    //draw buttons
    //row 1 (C, ;, %, ÷)
    canvas_draw_str(canvas, 5, 54, "C");
    canvas_draw_str(canvas, 19, 54, " <-");
    canvas_draw_str(canvas, 35, 54, " %");
    canvas_draw_str(canvas, 51, 54, " /");

    //row 2 (1, 2, 3, X)
    canvas_draw_str(canvas, 5, 70, " 1");
    canvas_draw_str(canvas, 19, 70, " 2");
    canvas_draw_str(canvas, 35, 70, " 3");
    canvas_draw_str(canvas, 51, 70, " X");

    //row 3 (4, 5, 6, -)
    canvas_draw_str(canvas, 5, 86, " 4");
    canvas_draw_str(canvas, 19, 86, " 5");
    canvas_draw_str(canvas, 35, 86, " 6");
    canvas_draw_str(canvas, 51, 86, " -");

    //row 4 (7, 8, 9, +)
    canvas_draw_str(canvas, 5, 102, " 7");
    canvas_draw_str(canvas, 19, 102, " 8");
    canvas_draw_str(canvas, 35, 102, " 9");
    canvas_draw_str(canvas, 51, 102, " +");

    //row 5 (+/-, 0, ., =)
    canvas_draw_str(canvas, 3, 118, "( )");
    canvas_draw_str(canvas, 19, 118, " 0");
    canvas_draw_str(canvas, 35, 118, "   .");
    canvas_draw_str(canvas, 51, 118, " =");
};

void calculator_draw_callback(Canvas* canvas, void* ctx) {
    const Calculator* calculator_state = acquire_mutex((ValueMutex*)ctx, 25);
    UNUSED(ctx);
    canvas_clear(canvas);

    //show selected button
    short startX = 1;
    short startY = 43;

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(
        canvas,
        startX + (calculator_state->position.x) * 16,
        (startY) + (calculator_state->position.y) * 16,
        16,
        16);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(
        canvas,
        startX + (calculator_state->position.x) * 16 + 2,
        (startY) + (calculator_state->position.y) * 16 + 2,
        10,
        10);

    canvas_set_color(canvas, ColorBlack);
    generate_calculator_layout(canvas);

    //draw text
    short stringWidth = calculateStringWidth(calculator_state->text, calculator_state->textLength);
    short startingPosition = 5;
    if(stringWidth > 60) {
        startingPosition += 60 - (stringWidth + 5);
    }
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(canvas, startingPosition, 28, calculator_state->text);
    //canvas_draw_str(canvas, 10, 10, calculator_state->log);

    //draw cursor
    canvas_draw_box(canvas, stringWidth + 5, 29, 5, 1);

    release_mutex((ValueMutex*)ctx, calculator_state);
}

void calculator_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

void calculate(Calculator* calculator_state) {
    double result;
    result = te_interp(calculator_state->text, 0);

    calculator_state->textLength = 0;
    calculator_state->text[0] = '\0';
    // sprintf(calculator_state->text, "%f", result);

    //invert sign if negative
    if(result < 0) {
        calculator_state->text[calculator_state->textLength++] = '-';
        result = -result;
    }

    //get numbers before and after decimal
    int beforeDecimal = result;
    int afterDecimal = (result - beforeDecimal) * 100;

    char beforeDecimalString[10];
    char afterDecimalString[10];
    int i = 0;
    //parse to a string
    while(beforeDecimal > 0) {
        beforeDecimalString[i++] = beforeDecimal % 10 + '0';
        beforeDecimal /= 10;
    }
    // invert string
    for(int j = 0; j < i / 2; j++) {
        char temp = beforeDecimalString[j];
        beforeDecimalString[j] = beforeDecimalString[i - j - 1];
        beforeDecimalString[i - j - 1] = temp;
    }
    //add it to the answer
    for(int j = 0; j < i; j++) {
        calculator_state->text[calculator_state->textLength++] = beforeDecimalString[j];
    }

    i = 0;
    if(afterDecimal > 0) {
        while(afterDecimal > 0) {
            afterDecimalString[i++] = afterDecimal % 10 + '0';
            afterDecimal /= 10;
        }
        // invert string
        for(int j = 0; j < i / 2; j++) {
            char temp = afterDecimalString[j];
            afterDecimalString[j] = afterDecimalString[i - j - 1];
            afterDecimalString[i - j - 1] = temp;
        }

        //add decimal point
        calculator_state->text[calculator_state->textLength++] = '.';

        //add numbers after decimal
        for(int j = 0; j < i; j++) {
            calculator_state->text[calculator_state->textLength++] = afterDecimalString[j];
        }
    }
    calculator_state->text[calculator_state->textLength] = '\0';
}

int32_t calculator_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    Calculator* calculator_state = malloc(sizeof(Calculator));
    ValueMutex calculator_state_mutex;
    if(!init_mutex(&calculator_state_mutex, calculator_state, sizeof(Calculator))) {
        //FURI_LOG_E("calculator", "cannot create mutex\r\n");
        free(calculator_state);
        return -1;
    }

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, calculator_draw_callback, &calculator_state_mutex);
    view_port_input_callback_set(view_port, calculator_input_callback, event_queue);
    view_port_set_orientation(view_port, ViewPortOrientationVertical);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    //NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    InputEvent event;

    while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        //break out of the loop if the back key is pressed
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            break;
        }

        if(event.type == InputTypeShort) {
            switch(event.key) {
            case InputKeyUp:
                if(calculator_state->position.y > 0) {
                    calculator_state->position.y--;
                }
                break;
            case InputKeyDown:
                if(calculator_state->position.y < 4) {
                    calculator_state->position.y++;
                }
                break;
            case InputKeyLeft:
                if(calculator_state->position.x > 0) {
                    calculator_state->position.x--;
                }
                break;
            case InputKeyRight:
                if(calculator_state->position.x < 3) {
                    calculator_state->position.x++;
                }
                break;
            case InputKeyOk: {
                //add the selected button to the text
                //char* text = calculator_state->text;
                // short* textLength = &calculator_state->textLength;

                char key =
                    getKeyAtPosition(calculator_state->position.x, calculator_state->position.y);

                switch(key) {
                case 'C':
                    while(calculator_state->textLength > 0) {
                        calculator_state->text[calculator_state->textLength--] = '\0';
                    }
                    calculator_state->text[0] = '\0';
                    calculator_state->log[2] = key;
                    break;
                case '<':
                    calculator_state->log[2] = key;
                    if(calculator_state->textLength > 0) {
                        calculator_state->text[--calculator_state->textLength] = '\0';
                    } else {
                        calculator_state->text[0] = '\0';
                    }
                    break;
                case '=':
                    calculator_state->log[2] = key;
                    calculate(calculator_state);
                    break;
                case '%':
                case '/':
                case '*':
                case '-':
                case '+':
                case '.':
                case '(':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '0':
                    if(calculator_state->textLength < MAX_TEXT_LENGTH) {
                        calculator_state->text[calculator_state->textLength++] = key;
                        calculator_state->text[calculator_state->textLength] = '\0';
                    }
                    //calculator_state->log[1] = calculator_state->text[*textLength];
                    break;
                default:
                    break;
                }
            }
            default:
                break;
            }

            view_port_update(view_port);
        }

        if(event.type == InputTypeLong) {
            switch(event.key) {
            case InputKeyOk:
                if(calculator_state->position.x == 0 && calculator_state->position.y == 4) {
                    if(calculator_state->textLength < MAX_TEXT_LENGTH) {
                        calculator_state->text[calculator_state->textLength++] = ')';
                        calculator_state->text[calculator_state->textLength] = '\0';
                    }
                    view_port_update(view_port);
                }
                break;
            default:
                break;
            }
        }
    }
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    return 0;
}
