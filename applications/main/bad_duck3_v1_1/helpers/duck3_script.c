/**
 * @file duck3_script.c
 * @brief DuckyScript 3.0 interpreter with BLE/USB HID support
 *
 * Full implementation with LOOP, WHILE, IF/ELSE, VAR support
 * and threaded execution like Bad USB.
 */

#include "duck3_script.h"
#include <furi_hal.h>
#include <furi_hal_usb_hid.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <toolbox/path.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAG "Duck3Script"

#define MAX_LINES 512
#define MAX_LINE_LEN 256
#define MAX_LOOP_DEPTH 8
#define MAX_VARIABLES 16
#define MAX_VAR_NAME 16
#define MAX_IF_DEPTH 8

#define SCRIPT_STATE_ERROR -1
#define SCRIPT_STATE_END -2
#define SCRIPT_STATE_WAIT_BTN -3

typedef struct {
    uint32_t start_line;
    int32_t remaining;
    bool is_while;
    char condition[64];
} LoopContext;

typedef struct {
    char name[MAX_VAR_NAME];
    int32_t value;
    bool is_set;
} Variable;

typedef struct {
    bool condition_met;
    bool in_else;
} IfContext;

struct Duck3Script {
    FuriThread* thread;
    Duck3State state;

    // HID interface
    Duck3HidInterface* interface;
    Duck3HidConfig* hid_cfg;
    const Duck3HidApi* hid;
    void* hid_inst;

    // Lines stored in memory
    char** lines;
    uint32_t line_count;
    uint32_t line_cur;

    // Keyboard layout
    uint16_t layout[128];

    // Delays
    uint32_t defdelay;
    uint32_t stringdelay;

    // Repeat
    uint32_t repeat_cnt;
    uint32_t repeat_line;

    // Loop stack
    LoopContext loop_stack[MAX_LOOP_DEPTH];
    uint8_t loop_depth;

    // Variables
    Variable variables[MAX_VARIABLES];
    uint8_t var_count;

    // IF stack
    IfContext if_stack[MAX_IF_DEPTH];
    uint8_t if_depth;

    // Skip flags
    bool skip_until_endif;
    bool skip_until_endloop;
    uint8_t skip_depth;

    // Worker state
    bool worker_running;
    bool paused;
    uint32_t start_time;
};

// Forward declarations
static int32_t duck3_worker(void* context);
static int32_t duck3_parse_line(Duck3Script* script, const char* line);

// ============================================================================
// Variable Management
// ============================================================================

static Variable* find_variable(Duck3Script* script, const char* name) {
    for(uint8_t i = 0; i < script->var_count; i++) {
        if(strcmp(script->variables[i].name, name) == 0) {
            return &script->variables[i];
        }
    }
    return NULL;
}

static int32_t get_var(Duck3Script* script, const char* name) {
    if(strcmp(name, "$_RANDOM") == 0 || strcmp(name, "RANDOM") == 0) {
        return (int32_t)(furi_hal_random_get() % 65536);
    }
    if(strcmp(name, "$_LINE") == 0) {
        return (int32_t)script->line_cur;
    }

    Variable* var = find_variable(script, name);
    if(var && var->is_set) {
        return var->value;
    }
    return 0;
}

static bool set_var(Duck3Script* script, const char* name, int32_t value) {
    Variable* var = find_variable(script, name);
    if(var) {
        var->value = value;
        var->is_set = true;
        return true;
    }

    if(script->var_count >= MAX_VARIABLES) return false;

    var = &script->variables[script->var_count++];
    strncpy(var->name, name, MAX_VAR_NAME - 1);
    var->name[MAX_VAR_NAME - 1] = '\0';
    var->value = value;
    var->is_set = true;
    return true;
}

// ============================================================================
// Expression Evaluator
// ============================================================================

static int32_t parse_value(Duck3Script* script, const char** ptr) {
    while(**ptr == ' ') (*ptr)++;

    if(**ptr == '$') {
        (*ptr)++;
        char var_name[MAX_VAR_NAME] = "$";
        int i = 1;
        while(isalnum((unsigned char)**ptr) || **ptr == '_') {
            if(i < MAX_VAR_NAME - 1) var_name[i++] = **ptr;
            (*ptr)++;
        }
        var_name[i] = '\0';
        return get_var(script, var_name);
    }

    bool negative = false;
    if(**ptr == '-') {
        negative = true;
        (*ptr)++;
    }

    int32_t value = 0;
    while(isdigit((unsigned char)**ptr)) {
        value = value * 10 + (**ptr - '0');
        (*ptr)++;
    }

    return negative ? -value : value;
}

static bool eval_expr(Duck3Script* script, const char* expr, int32_t* result) {
    const char* ptr = expr;
    while(*ptr == ' ' || *ptr == '(') ptr++;

    int32_t left = parse_value(script, &ptr);
    while(*ptr == ' ') ptr++;

    if(*ptr == '\0' || *ptr == ')') {
        *result = left;
        return true;
    }

    char op1 = *ptr++;
    char op2 = (*ptr == '=' || *ptr == '&' || *ptr == '|') ? *ptr++ : '\0';

    while(*ptr == ' ') ptr++;
    int32_t right = parse_value(script, &ptr);

    if(op1 == '+') *result = left + right;
    else if(op1 == '-') *result = left - right;
    else if(op1 == '*') *result = left * right;
    else if(op1 == '/' && right != 0) *result = left / right;
    else if(op1 == '%' && right != 0) *result = left % right;
    else if(op1 == '<' && op2 == '=') *result = left <= right ? 1 : 0;
    else if(op1 == '>' && op2 == '=') *result = left >= right ? 1 : 0;
    else if(op1 == '=' && op2 == '=') *result = left == right ? 1 : 0;
    else if(op1 == '!' && op2 == '=') *result = left != right ? 1 : 0;
    else if(op1 == '<') *result = left < right ? 1 : 0;
    else if(op1 == '>') *result = left > right ? 1 : 0;
    else if(op1 == '&' && op2 == '&') *result = (left && right) ? 1 : 0;
    else if(op1 == '|' && op2 == '|') *result = (left || right) ? 1 : 0;
    else return false;

    return true;
}

// ============================================================================
// HID Wrappers
// ============================================================================

static void hid_press(Duck3Script* script, uint16_t keycode) {
    script->hid->kb_press(script->hid_inst, keycode);
}

static void hid_release(Duck3Script* script, uint16_t keycode) {
    script->hid->kb_release(script->hid_inst, keycode);
}

static void hid_release_all(Duck3Script* script) {
    script->hid->release_all(script->hid_inst);
}

static bool type_string(Duck3Script* script, const char* str) {
    for(size_t i = 0; str[i] != '\0'; i++) {
        uint8_t c = (uint8_t)str[i];
        if(c == '\n') {
            hid_press(script, HID_KEYBOARD_RETURN);
            hid_release(script, HID_KEYBOARD_RETURN);
        } else if(c < 128) {
            uint16_t keycode = script->layout[c];
            if(keycode != 0) {
                hid_press(script, keycode);
                hid_release(script, keycode);
            }
        }
        if(script->stringdelay > 0) {
            furi_delay_ms(script->stringdelay);
        }
    }
    return true;
}

// ============================================================================
// Command Handlers
// ============================================================================

static int32_t cmd_delay(Duck3Script* script, const char* arg) {
    UNUSED(script);
    return atoi(arg);
}

static int32_t cmd_string(Duck3Script* script, const char* arg, bool newline) {
    type_string(script, arg);
    if(newline) {
        hid_press(script, HID_KEYBOARD_RETURN);
        hid_release(script, HID_KEYBOARD_RETURN);
    }
    return 0;
}

static int32_t cmd_loop(Duck3Script* script, const char* arg) {
    if(script->loop_depth >= MAX_LOOP_DEPTH) {
        snprintf(script->state.error, sizeof(script->state.error), "Loop depth exceeded");
        return SCRIPT_STATE_ERROR;
    }

    int32_t count;
    if(arg[0] == '$') {
        count = get_var(script, arg);
    } else {
        count = atoi(arg);
    }

    if(count <= 0) {
        script->skip_until_endloop = true;
        script->skip_depth = 1;
        return 0;
    }

    LoopContext* ctx = &script->loop_stack[script->loop_depth];
    ctx->start_line = script->line_cur + 1;
    ctx->remaining = count;
    ctx->is_while = false;
    script->loop_depth++;

    return 0;
}

static int32_t cmd_end_loop(Duck3Script* script) {
    if(script->skip_until_endloop) {
        script->skip_depth--;
        if(script->skip_depth == 0) {
            script->skip_until_endloop = false;
        }
        return 0;
    }

    if(script->loop_depth == 0) {
        snprintf(script->state.error, sizeof(script->state.error), "END_LOOP without LOOP");
        return SCRIPT_STATE_ERROR;
    }

    LoopContext* ctx = &script->loop_stack[script->loop_depth - 1];

    if(!ctx->is_while) {
        ctx->remaining--;
        if(ctx->remaining > 0) {
            script->line_cur = ctx->start_line - 1;
        } else {
            script->loop_depth--;
        }
    }

    return 0;
}

static int32_t cmd_while(Duck3Script* script, const char* arg) {
    if(script->loop_depth >= MAX_LOOP_DEPTH) {
        return SCRIPT_STATE_ERROR;
    }

    const char* cond = strchr(arg, '(');
    if(!cond) cond = arg;

    int32_t result;
    if(!eval_expr(script, cond, &result)) {
        return SCRIPT_STATE_ERROR;
    }

    if(result == 0) {
        script->skip_until_endloop = true;
        script->skip_depth = 1;
        return 0;
    }

    LoopContext* ctx = &script->loop_stack[script->loop_depth];
    ctx->start_line = script->line_cur + 1;
    ctx->remaining = -1;
    ctx->is_while = true;
    strncpy(ctx->condition, cond, sizeof(ctx->condition) - 1);
    script->loop_depth++;

    return 0;
}

static int32_t cmd_end_while(Duck3Script* script) {
    if(script->skip_until_endloop) {
        script->skip_depth--;
        if(script->skip_depth == 0) {
            script->skip_until_endloop = false;
        }
        return 0;
    }

    if(script->loop_depth == 0) return SCRIPT_STATE_ERROR;

    LoopContext* ctx = &script->loop_stack[script->loop_depth - 1];

    if(ctx->is_while) {
        int32_t result;
        if(eval_expr(script, ctx->condition, &result) && result != 0) {
            script->line_cur = ctx->start_line - 1;
        } else {
            script->loop_depth--;
        }
    }

    return 0;
}

static int32_t cmd_var(Duck3Script* script, const char* arg) {
    const char* ptr = arg;
    while(*ptr == ' ') ptr++;

    if(*ptr != '$') return SCRIPT_STATE_ERROR;

    char var_name[MAX_VAR_NAME];
    int i = 0;
    while(*ptr && *ptr != ' ' && *ptr != '=' && i < MAX_VAR_NAME - 1) {
        var_name[i++] = *ptr++;
    }
    var_name[i] = '\0';

    while(*ptr == ' ') ptr++;
    if(*ptr != '=') return SCRIPT_STATE_ERROR;
    ptr++;
    while(*ptr == ' ') ptr++;

    int32_t value;
    if(!eval_expr(script, ptr, &value)) {
        value = atoi(ptr);
    }

    set_var(script, var_name, value);
    return 0;
}

static int32_t cmd_if(Duck3Script* script, const char* arg) {
    if(script->if_depth >= MAX_IF_DEPTH) return SCRIPT_STATE_ERROR;

    const char* cond = strchr(arg, '(');
    if(!cond) cond = arg;

    int32_t result;
    bool condition = eval_expr(script, cond, &result) && result != 0;

    IfContext* ctx = &script->if_stack[script->if_depth];
    ctx->condition_met = condition;
    ctx->in_else = false;
    script->if_depth++;

    if(!condition) {
        script->skip_until_endif = true;
        script->skip_depth = 1;
    }

    return 0;
}

static int32_t cmd_else(Duck3Script* script) {
    if(script->if_depth == 0) return SCRIPT_STATE_ERROR;

    IfContext* ctx = &script->if_stack[script->if_depth - 1];
    ctx->in_else = true;

    if(script->skip_until_endif && script->skip_depth == 1) {
        script->skip_until_endif = false;
        script->skip_depth = 0;
    } else if(!script->skip_until_endif && ctx->condition_met) {
        script->skip_until_endif = true;
        script->skip_depth = 1;
    }

    return 0;
}

static int32_t cmd_end_if(Duck3Script* script) {
    if(script->skip_until_endif) {
        script->skip_depth--;
        if(script->skip_depth == 0) {
            script->skip_until_endif = false;
        }
    }

    if(script->if_depth > 0) {
        script->if_depth--;
    }

    return 0;
}

static int32_t cmd_break(Duck3Script* script) {
    if(script->loop_depth == 0) return SCRIPT_STATE_ERROR;

    script->skip_until_endloop = true;
    script->skip_depth = 1;
    script->loop_stack[script->loop_depth - 1].remaining = 0;

    return 0;
}

static int32_t cmd_continue(Duck3Script* script) {
    if(script->loop_depth == 0) return SCRIPT_STATE_ERROR;

    LoopContext* ctx = &script->loop_stack[script->loop_depth - 1];

    if(ctx->is_while) {
        script->line_cur = ctx->start_line - 1;
    } else {
        ctx->remaining--;
        if(ctx->remaining > 0) {
            script->line_cur = ctx->start_line - 1;
        } else {
            script->skip_until_endloop = true;
            script->skip_depth = 1;
        }
    }

    return 0;
}

// ============================================================================
// Key codes
// ============================================================================

static uint16_t get_keycode(const char* name) {
    if(strcmp(name, "ENTER") == 0 || strcmp(name, "RETURN") == 0) return HID_KEYBOARD_RETURN;
    if(strcmp(name, "ESCAPE") == 0 || strcmp(name, "ESC") == 0) return HID_KEYBOARD_ESCAPE;
    if(strcmp(name, "BACKSPACE") == 0) return HID_KEYBOARD_DELETE;
    if(strcmp(name, "TAB") == 0) return HID_KEYBOARD_TAB;
    if(strcmp(name, "SPACE") == 0) return HID_KEYBOARD_SPACEBAR;
    if(strcmp(name, "CAPSLOCK") == 0) return HID_KEYBOARD_CAPS_LOCK;
    if(strcmp(name, "DELETE") == 0) return HID_KEYBOARD_DELETE_FORWARD;
    if(strcmp(name, "INSERT") == 0) return HID_KEYBOARD_INSERT;
    if(strcmp(name, "HOME") == 0) return HID_KEYBOARD_HOME;
    if(strcmp(name, "END") == 0) return HID_KEYBOARD_END;
    if(strcmp(name, "PAGEUP") == 0) return HID_KEYBOARD_PAGE_UP;
    if(strcmp(name, "PAGEDOWN") == 0) return HID_KEYBOARD_PAGE_DOWN;
    if(strcmp(name, "UP") == 0 || strcmp(name, "UPARROW") == 0) return HID_KEYBOARD_UP_ARROW;
    if(strcmp(name, "DOWN") == 0 || strcmp(name, "DOWNARROW") == 0) return HID_KEYBOARD_DOWN_ARROW;
    if(strcmp(name, "LEFT") == 0 || strcmp(name, "LEFTARROW") == 0) return HID_KEYBOARD_LEFT_ARROW;
    if(strcmp(name, "RIGHT") == 0 || strcmp(name, "RIGHTARROW") == 0) return HID_KEYBOARD_RIGHT_ARROW;
    if(strcmp(name, "PRINTSCREEN") == 0) return HID_KEYBOARD_PRINT_SCREEN;
    if(strcmp(name, "PAUSE") == 0) return HID_KEYBOARD_PAUSE;
    if(strcmp(name, "MENU") == 0 || strcmp(name, "APP") == 0) return HID_KEYBOARD_APPLICATION;
    if(strcmp(name, "F1") == 0) return HID_KEYBOARD_F1;
    if(strcmp(name, "F2") == 0) return HID_KEYBOARD_F2;
    if(strcmp(name, "F3") == 0) return HID_KEYBOARD_F3;
    if(strcmp(name, "F4") == 0) return HID_KEYBOARD_F4;
    if(strcmp(name, "F5") == 0) return HID_KEYBOARD_F5;
    if(strcmp(name, "F6") == 0) return HID_KEYBOARD_F6;
    if(strcmp(name, "F7") == 0) return HID_KEYBOARD_F7;
    if(strcmp(name, "F8") == 0) return HID_KEYBOARD_F8;
    if(strcmp(name, "F9") == 0) return HID_KEYBOARD_F9;
    if(strcmp(name, "F10") == 0) return HID_KEYBOARD_F10;
    if(strcmp(name, "F11") == 0) return HID_KEYBOARD_F11;
    if(strcmp(name, "F12") == 0) return HID_KEYBOARD_F12;
    return 0;
}

static uint16_t get_modifier(const char* name) {
    if(strcmp(name, "CTRL") == 0 || strcmp(name, "CONTROL") == 0) return KEY_MOD_LEFT_CTRL;
    if(strcmp(name, "SHIFT") == 0) return KEY_MOD_LEFT_SHIFT;
    if(strcmp(name, "ALT") == 0) return KEY_MOD_LEFT_ALT;
    if(strcmp(name, "GUI") == 0 || strcmp(name, "WINDOWS") == 0 || strcmp(name, "COMMAND") == 0) return KEY_MOD_LEFT_GUI;
    return 0;
}

static bool duck3_execute_keys(Duck3Script* script, const char* line) {
    uint16_t modifiers = 0;
    const char* ptr = line;

    while(*ptr) {
        while(*ptr == ' ' || *ptr == '-') ptr++;
        if(!*ptr) break;

        char word[32];
        int i = 0;
        while(*ptr && *ptr != ' ' && *ptr != '-' && i < 31) {
            word[i++] = *ptr++;
        }
        word[i] = '\0';

        uint16_t mod = get_modifier(word);
        if(mod) {
            modifiers |= mod;
        } else {
            uint16_t key = get_keycode(word);
            if(key == 0 && strlen(word) == 1) {
                uint8_t c = (uint8_t)word[0];
                if(c < 128) {
                    key = script->layout[c];
                }
            }

            if(key != 0 || modifiers != 0) {
                hid_press(script, modifiers | key);
                hid_release(script, modifiers | key);
            }
            return true;
        }
    }

    if(modifiers != 0) {
        hid_press(script, modifiers);
        hid_release(script, modifiers);
    }

    return true;
}

// ============================================================================
// Line Parser
// ============================================================================

static int32_t duck3_parse_line(Duck3Script* script, const char* line) {
    if(!line || !*line) return 0;

    // Skip comments
    if(strncmp(line, "REM", 3) == 0) return 0;
    if(strncmp(line, "ID", 2) == 0) return 0;
    if(strncmp(line, "BT_ID", 5) == 0) return 0;
    if(strncmp(line, "BLE_ID", 6) == 0) return 0;

    // Skip if in skip mode
    if(script->skip_until_endif || script->skip_until_endloop) {
        if(strncmp(line, "END_IF", 6) == 0) return cmd_end_if(script);
        if(strncmp(line, "ELSE", 4) == 0) return cmd_else(script);
        if(strncmp(line, "END_LOOP", 8) == 0) return cmd_end_loop(script);
        if(strncmp(line, "END_WHILE", 9) == 0) return cmd_end_while(script);

        if(strncmp(line, "IF", 2) == 0 && script->skip_until_endif) script->skip_depth++;
        if(strncmp(line, "LOOP", 4) == 0 && script->skip_until_endloop) script->skip_depth++;
        if(strncmp(line, "WHILE", 5) == 0 && script->skip_until_endloop) script->skip_depth++;

        return 0;
    }

    size_t cmd_len = 0;
    while(line[cmd_len] && line[cmd_len] != ' ') cmd_len++;
    const char* arg = line[cmd_len] ? &line[cmd_len + 1] : "";

    // Commands
    if(strncmp(line, "DELAY", 5) == 0 && cmd_len == 5) return cmd_delay(script, arg);
    if(strncmp(line, "STRING ", 7) == 0) return cmd_string(script, arg, false);
    if(strncmp(line, "STRINGLN ", 9) == 0) return cmd_string(script, arg, true);
    if(strncmp(line, "DEFAULT_DELAY", 13) == 0 || strncmp(line, "DEFAULTDELAY", 12) == 0) {
        script->defdelay = atoi(arg);
        return 0;
    }
    if(strncmp(line, "STRING_DELAY", 12) == 0 || strncmp(line, "STRINGDELAY", 11) == 0) {
        script->stringdelay = atoi(arg);
        return 0;
    }
    if(strncmp(line, "LOOP", 4) == 0 && cmd_len == 4) return cmd_loop(script, arg);
    if(strncmp(line, "END_LOOP", 8) == 0) return cmd_end_loop(script);
    if(strncmp(line, "WHILE", 5) == 0 && cmd_len == 5) return cmd_while(script, arg);
    if(strncmp(line, "END_WHILE", 9) == 0) return cmd_end_while(script);
    if(strncmp(line, "VAR", 3) == 0 && cmd_len == 3) return cmd_var(script, arg);
    if(strncmp(line, "IF", 2) == 0 && cmd_len == 2) return cmd_if(script, arg);
    if(strncmp(line, "ELSE", 4) == 0 && cmd_len == 4) return cmd_else(script);
    if(strncmp(line, "END_IF", 6) == 0) return cmd_end_if(script);
    if(strncmp(line, "BREAK", 5) == 0) return cmd_break(script);
    if(strncmp(line, "CONTINUE", 8) == 0) return cmd_continue(script);
    if(strncmp(line, "REPEAT", 6) == 0) {
        script->repeat_cnt = atoi(arg);
        script->repeat_line = script->line_cur;
        return 0;
    }
    if(strncmp(line, "WAIT_FOR_BUTTON_PRESS", 21) == 0) {
        return SCRIPT_STATE_WAIT_BTN;
    }

    // Mouse commands
    if(strncmp(line, "MOUSE_MOVE", 10) == 0) {
        int dx = 0, dy = 0;
        sscanf(arg, "%d %d", &dx, &dy);
        script->hid->mouse_move(script->hid_inst, (int8_t)dx, (int8_t)dy);
        return 0;
    }
    if(strncmp(line, "MOUSE_CLICK", 11) == 0) {
        script->hid->mouse_press(script->hid_inst, 1);
        furi_delay_ms(50);
        script->hid->mouse_release(script->hid_inst, 1);
        return 0;
    }
    if(strncmp(line, "MOUSE_SCROLL", 12) == 0) {
        int delta = atoi(arg);
        script->hid->mouse_scroll(script->hid_inst, (int8_t)delta);
        return 0;
    }

    // Default: treat as key combo
    duck3_execute_keys(script, line);
    return 0;
}

// ============================================================================
// Script Loading
// ============================================================================

static bool load_script_file(Duck3Script* script, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool success = false;

    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint32_t line_count = 0;
        char buf[MAX_LINE_LEN];
        size_t line_len = 0;

        // Count lines
        while(!storage_file_eof(file)) {
            uint8_t c;
            if(storage_file_read(file, &c, 1) == 1) {
                if(c == '\n') {
                    if(line_len > 0) line_count++;
                    line_len = 0;
                } else if(c != '\r') {
                    line_len++;
                }
            }
        }
        if(line_len > 0) line_count++;

        if(line_count > MAX_LINES) {
            FURI_LOG_E(TAG, "Too many lines: %lu", line_count);
            goto cleanup;
        }

        // Allocate
        script->lines = malloc(sizeof(char*) * line_count);
        if(!script->lines) goto cleanup;
        memset(script->lines, 0, sizeof(char*) * line_count);

        // Read lines
        storage_file_seek(file, 0, true);
        script->line_count = 0;
        line_len = 0;

        while(!storage_file_eof(file) && script->line_count < line_count) {
            uint8_t c;
            if(storage_file_read(file, &c, 1) == 1) {
                if(c == '\n') {
                    if(line_len > 0) {
                        buf[line_len] = '\0';
                        while(line_len > 0 && (buf[line_len-1] == ' ' || buf[line_len-1] == '\t')) {
                            buf[--line_len] = '\0';
                        }
                        char* start = buf;
                        while(*start == ' ' || *start == '\t') start++;
                        if(strlen(start) > 0) {
                            script->lines[script->line_count] = strdup(start);
                            script->line_count++;
                        }
                    }
                    line_len = 0;
                } else if(c != '\r' && line_len < MAX_LINE_LEN - 1) {
                    buf[line_len++] = c;
                }
            }
        }
        if(line_len > 0) {
            buf[line_len] = '\0';
            char* start = buf;
            while(*start == ' ' || *start == '\t') start++;
            if(strlen(start) > 0 && script->line_count < line_count) {
                script->lines[script->line_count] = strdup(start);
                script->line_count++;
            }
        }

        script->state.line_nb = script->line_count;
        success = true;
        FURI_LOG_I(TAG, "Loaded %lu lines", script->line_count);
    }

cleanup:
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return success;
}

// ============================================================================
// Keyboard Layout
// ============================================================================

void duck3_script_set_keyboard_layout(Duck3Script* script, FuriString* layout_path) {
    // Reset to default
    memcpy(script->layout, hid_asciimap, MIN(sizeof(hid_asciimap), sizeof(script->layout)));

    if(furi_string_empty(layout_path)) return;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(storage_file_open(
           file, furi_string_get_cstr(layout_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint16_t layout_data[128];
        if(storage_file_read(file, layout_data, sizeof(layout_data)) == sizeof(layout_data)) {
            memcpy(script->layout, layout_data, sizeof(script->layout));
        }
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

// ============================================================================
// Worker Thread
// ============================================================================

static void duck3_hid_state_callback(bool state, void* context) {
    Duck3Script* script = context;
    if(state) {
        if(script->state.state == Duck3StateNotConnected) {
            script->state.state = Duck3StateIdle;
        } else if(script->state.state == Duck3StateWillRun) {
            script->state.state = Duck3StateRunning;
            furi_thread_flags_set(furi_thread_get_id(script->thread), 1);
        }
    } else {
        if(script->state.state == Duck3StateRunning ||
           script->state.state == Duck3StateDelay) {
            script->state.state = Duck3StateNotConnected;
        }
    }
}

static int32_t duck3_worker(void* context) {
    Duck3Script* script = context;
    FURI_LOG_I(TAG, "Worker started");

    // Wait for start signal
    while(script->worker_running) {
        uint32_t flags = furi_thread_flags_wait(1, FuriFlagWaitAny, 100);
        if(flags == 1) break;
        if(script->state.state == Duck3StateRunning) break;
    }

    if(!script->worker_running) return 0;

    // Delay for host recognition
    furi_delay_ms(500);

    script->state.state = Duck3StateRunning;
    script->start_time = furi_get_tick();

    while(script->worker_running && script->state.state == Duck3StateRunning) {
        // Handle pause
        while(script->paused && script->worker_running) {
            script->state.state = Duck3StatePaused;
            furi_delay_ms(100);
        }
        if(!script->worker_running) break;
        if(script->state.state == Duck3StatePaused) {
            script->state.state = Duck3StateRunning;
        }

        // Handle repeat
        if(script->repeat_cnt > 0 && script->repeat_line < script->line_count) {
            script->repeat_cnt--;
            const char* line = script->lines[script->repeat_line];
            int32_t result = duck3_parse_line(script, line);
            if(result > 0) {
                script->state.state = Duck3StateDelay;
                script->state.delay_remain = result;
                while(result > 0 && script->worker_running && !script->paused) {
                    uint32_t chunk = (result > 100) ? 100 : result;
                    furi_delay_ms(chunk);
                    result -= chunk;
                    script->state.delay_remain = result;
                }
                script->state.state = Duck3StateRunning;
            }
            continue;
        }

        // Check if done
        if(script->line_cur >= script->line_count) {
            script->state.state = Duck3StateDone;
            hid_release_all(script);
            break;
        }

        // Execute line
        const char* line = script->lines[script->line_cur];
        script->state.line_cur = script->line_cur + 1;
        script->state.elapsed = furi_get_tick() - script->start_time;

        int32_t result = duck3_parse_line(script, line);
        script->line_cur++;

        if(result == SCRIPT_STATE_ERROR) {
            script->state.state = Duck3StateScriptError;
            script->state.error_line = script->state.line_cur;
            break;
        } else if(result == SCRIPT_STATE_WAIT_BTN) {
            script->state.state = Duck3StateWaitForBtn;
            while(script->state.state == Duck3StateWaitForBtn && script->worker_running) {
                furi_delay_ms(100);
            }
            if(script->state.state == Duck3StateWaitForBtn) {
                script->state.state = Duck3StateRunning;
            }
        } else if(result > 0) {
            result += script->defdelay;
            script->state.state = Duck3StateDelay;
            script->state.delay_remain = result;
            while(result > 0 && script->worker_running && !script->paused) {
                uint32_t chunk = (result > 100) ? 100 : result;
                furi_delay_ms(chunk);
                result -= chunk;
                script->state.delay_remain = result;
            }
            if(script->state.state == Duck3StateDelay) {
                script->state.state = Duck3StateRunning;
            }
        } else if(script->defdelay > 0) {
            furi_delay_ms(script->defdelay);
        }
    }

    FURI_LOG_I(TAG, "Worker finished");
    return 0;
}

// ============================================================================
// Public API
// ============================================================================

Duck3Script* duck3_script_open(
    FuriString* file_path,
    Duck3HidInterface* interface,
    Duck3HidConfig* hid_cfg,
    bool load_id_config) {
    UNUSED(load_id_config);

    Duck3Script* script = malloc(sizeof(Duck3Script));
    memset(script, 0, sizeof(Duck3Script));

    script->interface = interface;
    script->hid_cfg = hid_cfg;

    // Load default layout
    memcpy(script->layout, hid_asciimap, MIN(sizeof(hid_asciimap), sizeof(script->layout)));

    // Load script
    if(!load_script_file(script, furi_string_get_cstr(file_path))) {
        script->state.state = Duck3StateFileError;
        return script;
    }

    // Initialize HID
    script->hid = duck3_hid_get_interface(*interface);
    script->hid->adjust_config(hid_cfg);
    script->hid_inst = script->hid->init(hid_cfg);
    script->hid->set_state_callback(script->hid_inst, duck3_hid_state_callback, script);

    // Check connection
    if(script->hid->is_connected(script->hid_inst)) {
        script->state.state = Duck3StateIdle;
    } else {
        script->state.state = Duck3StateNotConnected;
    }

    // Start worker thread
    script->worker_running = true;
    script->thread = furi_thread_alloc_ex("Duck3Worker", 4096, duck3_worker, script);
    furi_thread_start(script->thread);

    return script;
}

void duck3_script_close(Duck3Script* script) {
    furi_assert(script);

    // Stop worker
    script->worker_running = false;
    script->state.state = Duck3StateDone;
    furi_thread_flags_set(furi_thread_get_id(script->thread), 1);
    furi_thread_join(script->thread);
    furi_thread_free(script->thread);

    // Deinit HID
    if(script->hid_inst) {
        script->hid->set_state_callback(script->hid_inst, NULL, NULL);
        script->hid->deinit(script->hid_inst);
    }

    // Free lines
    if(script->lines) {
        for(uint32_t i = 0; i < script->line_count; i++) {
            if(script->lines[i]) free(script->lines[i]);
        }
        free(script->lines);
    }

    free(script);
}

void duck3_script_start_stop(Duck3Script* script) {
    furi_assert(script);

    if(script->state.state == Duck3StateNotConnected) {
        script->state.state = Duck3StateWillRun;
    } else if(script->state.state == Duck3StateIdle || script->state.state == Duck3StateDone) {
        // Reset state
        script->line_cur = 0;
        script->loop_depth = 0;
        script->if_depth = 0;
        script->var_count = 0;
        script->skip_until_endif = false;
        script->skip_until_endloop = false;
        script->skip_depth = 0;
        script->repeat_cnt = 0;
        script->defdelay = 0;
        script->stringdelay = 0;
        script->paused = false;
        script->state.error[0] = '\0';

        script->state.state = Duck3StateRunning;
        furi_thread_flags_set(furi_thread_get_id(script->thread), 1);
    } else if(script->state.state == Duck3StateRunning ||
              script->state.state == Duck3StateDelay ||
              script->state.state == Duck3StatePaused) {
        script->worker_running = false;
        script->state.state = Duck3StateIdle;
        hid_release_all(script);
    } else if(script->state.state == Duck3StateWillRun) {
        script->state.state = Duck3StateNotConnected;
    } else if(script->state.state == Duck3StateWaitForBtn) {
        script->state.state = Duck3StateRunning;
    }
}

void duck3_script_pause_resume(Duck3Script* script) {
    furi_assert(script);

    if(script->state.state == Duck3StateRunning || script->state.state == Duck3StateDelay) {
        script->paused = true;
    } else if(script->state.state == Duck3StatePaused) {
        script->paused = false;
    }
}

Duck3State* duck3_script_get_state(Duck3Script* script) {
    furi_assert(script);
    return &script->state;
}
