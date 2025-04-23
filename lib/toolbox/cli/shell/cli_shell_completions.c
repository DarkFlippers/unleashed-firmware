#include "cli_shell_completions.h"

ARRAY_DEF(CommandCompletions, FuriString*, FURI_STRING_OPLIST); // -V524
#define M_OPL_CommandCompletions_t() ARRAY_OPLIST(CommandCompletions)

struct CliShellCompletions {
    CliRegistry* registry;
    CliShell* shell;
    CliShellLine* line;
    CommandCompletions_t variants;
    size_t selected;
    bool is_displaying;
};

#define COMPLETION_COLUMNS        3
#define COMPLETION_COLUMN_WIDTH   "30"
#define COMPLETION_COLUMN_WIDTH_I 30

/**
 * @brief Update for the completions menu
 */
typedef enum {
    CliShellCompletionsActionOpen,
    CliShellCompletionsActionClose,
    CliShellCompletionsActionUp,
    CliShellCompletionsActionDown,
    CliShellCompletionsActionLeft,
    CliShellCompletionsActionRight,
    CliShellCompletionsActionSelect,
    CliShellCompletionsActionSelectNoClose,
} CliShellCompletionsAction;

typedef enum {
    CliShellCompletionSegmentTypeCommand,
    CliShellCompletionSegmentTypeArguments,
} CliShellCompletionSegmentType;

typedef struct {
    CliShellCompletionSegmentType type;
    size_t start;
    size_t length;
} CliShellCompletionSegment;

// ==========
// Public API
// ==========

CliShellCompletions*
    cli_shell_completions_alloc(CliRegistry* registry, CliShell* shell, CliShellLine* line) {
    CliShellCompletions* completions = malloc(sizeof(CliShellCompletions));

    completions->registry = registry;
    completions->shell = shell;
    completions->line = line;
    CommandCompletions_init(completions->variants);

    return completions;
}

void cli_shell_completions_free(CliShellCompletions* completions) {
    CommandCompletions_clear(completions->variants);
    free(completions);
}

// =======
// Helpers
// =======

CliShellCompletionSegment cli_shell_completions_segment(CliShellCompletions* completions) {
    furi_assert(completions);
    CliShellCompletionSegment segment;

    FuriString* input = furi_string_alloc_set(cli_shell_line_get_editing(completions->line));
    furi_string_left(input, cli_shell_line_get_line_position(completions->line));

    // find index of first non-space character
    size_t first_non_space = 0;
    while(1) {
        size_t ret = furi_string_search_char(input, ' ', first_non_space);
        if(ret == FURI_STRING_FAILURE) break;
        if(ret - first_non_space > 1) break;
        first_non_space++;
    }

    size_t first_space_in_command = furi_string_search_char(input, ' ', first_non_space);

    if(first_space_in_command == FURI_STRING_FAILURE) {
        segment.type = CliShellCompletionSegmentTypeCommand;
        segment.start = first_non_space;
        segment.length = furi_string_size(input) - first_non_space;
    } else {
        segment.type = CliShellCompletionSegmentTypeArguments;
        segment.start = 0;
        segment.length = 0;
        // support removed, might reimplement in the future
    }

    furi_string_free(input);
    return segment;
}

void cli_shell_completions_fill_variants(CliShellCompletions* completions) {
    furi_assert(completions);
    CommandCompletions_reset(completions->variants);

    CliShellCompletionSegment segment = cli_shell_completions_segment(completions);
    FuriString* input = furi_string_alloc_set(cli_shell_line_get_editing(completions->line));
    furi_string_right(input, segment.start);
    furi_string_left(input, segment.length);

    if(segment.type == CliShellCompletionSegmentTypeCommand) {
        CliRegistry* registry = completions->registry;
        cli_registry_lock(registry);
        CliCommandDict_t* commands = cli_registry_get_commands(registry);
        for
            M_EACH(registered_command, *commands, CliCommandDict_t) {
                FuriString* command_name = registered_command->key;
                if(furi_string_start_with(command_name, input)) {
                    CommandCompletions_push_back(completions->variants, command_name);
                }
            }
        cli_registry_unlock(registry);

    } else {
        // support removed, might reimplement in the future
    }

    furi_string_free(input);
}

static size_t cli_shell_completions_rows_at_column(CliShellCompletions* completions, size_t x) {
    size_t completions_size = CommandCompletions_size(completions->variants);
    size_t n_full_rows = completions_size / COMPLETION_COLUMNS;
    size_t n_cols_in_last_row = completions_size % COMPLETION_COLUMNS;
    size_t n_rows_at_x = n_full_rows + ((x >= n_cols_in_last_row) ? 0 : 1);
    return n_rows_at_x;
}

void cli_shell_completions_render(
    CliShellCompletions* completions,
    CliShellCompletionsAction action) {
    furi_assert(completions);
    if(action == CliShellCompletionsActionOpen) furi_check(!completions->is_displaying);
    if(action == CliShellCompletionsActionClose) furi_check(completions->is_displaying);

    char prompt[64];
    cli_shell_line_format_prompt(completions->line, prompt, sizeof(prompt));

    if(action == CliShellCompletionsActionOpen) {
        cli_shell_completions_fill_variants(completions);
        completions->selected = 0;

        if(CommandCompletions_size(completions->variants) == 1) {
            cli_shell_completions_render(completions, CliShellCompletionsActionSelectNoClose);
            return;
        }

        // show completions menu (full re-render)
        printf("\n\r");
        size_t position = 0;
        for
            M_EACH(completion, completions->variants, CommandCompletions_t) {
                if(position == completions->selected) printf(ANSI_INVERT);
                printf("%-" COMPLETION_COLUMN_WIDTH "s", furi_string_get_cstr(*completion));
                if(position == completions->selected) printf(ANSI_RESET);
                if((position % COMPLETION_COLUMNS == COMPLETION_COLUMNS - 1) &&
                   position != CommandCompletions_size(completions->variants)) {
                    printf("\r\n");
                }
                position++;
            }

        if(!position) {
            printf(ANSI_FG_RED "no completions" ANSI_RESET);
        }

        size_t total_rows = (position / COMPLETION_COLUMNS) + 1;
        printf(
            ANSI_ERASE_DISPLAY(ANSI_ERASE_FROM_CURSOR_TO_END) ANSI_CURSOR_UP_BY("%zu")
                ANSI_CURSOR_HOR_POS("%zu"),
            total_rows,
            strlen(prompt) + cli_shell_line_get_line_position(completions->line) + 1);

        completions->is_displaying = true;

    } else if(action == CliShellCompletionsActionClose) {
        // clear completions menu
        printf(
            ANSI_CURSOR_HOR_POS("%zu") ANSI_ERASE_DISPLAY(ANSI_ERASE_FROM_CURSOR_TO_END)
                ANSI_CURSOR_HOR_POS("%zu"),
            strlen(prompt) + furi_string_size(cli_shell_line_get_selected(completions->line)) + 1,
            strlen(prompt) + cli_shell_line_get_line_position(completions->line) + 1);
        completions->is_displaying = false;

    } else if(
        action == CliShellCompletionsActionUp || action == CliShellCompletionsActionDown ||
        action == CliShellCompletionsActionLeft || action == CliShellCompletionsActionRight) {
        if(CommandCompletions_empty_p(completions->variants)) return;

        // move selection
        size_t completions_size = CommandCompletions_size(completions->variants);
        size_t old_selection = completions->selected;
        int n_columns = (completions_size >= COMPLETION_COLUMNS) ? COMPLETION_COLUMNS :
                                                                   completions_size;
        int selection_unclamped = old_selection;
        if(action == CliShellCompletionsActionLeft) {
            selection_unclamped--;
        } else if(action == CliShellCompletionsActionRight) {
            selection_unclamped++;
        } else {
            int selection_x = old_selection % COMPLETION_COLUMNS;
            int selection_y_unclamped = old_selection / COMPLETION_COLUMNS;
            if(action == CliShellCompletionsActionUp) selection_y_unclamped--;
            if(action == CliShellCompletionsActionDown) selection_y_unclamped++;
            size_t selection_y = 0;
            if(selection_y_unclamped < 0) {
                selection_x = CLAMP_WRAPAROUND(selection_x - 1, n_columns - 1, 0);
                selection_y =
                    cli_shell_completions_rows_at_column(completions, selection_x) - 1; // -V537
            } else if(
                (size_t)selection_y_unclamped >
                cli_shell_completions_rows_at_column(completions, selection_x) - 1) {
                selection_x = CLAMP_WRAPAROUND(selection_x + 1, n_columns - 1, 0);
                selection_y = 0;
            } else {
                selection_y = selection_y_unclamped;
            }
            selection_unclamped = (selection_y * COMPLETION_COLUMNS) + selection_x;
        }
        size_t new_selection = CLAMP_WRAPAROUND(selection_unclamped, (int)completions_size - 1, 0);
        completions->selected = new_selection;

        if(new_selection != old_selection) {
            // determine selection coordinates relative to top-left of suggestion menu
            size_t old_x = (old_selection % COMPLETION_COLUMNS) * COMPLETION_COLUMN_WIDTH_I;
            size_t old_y = old_selection / COMPLETION_COLUMNS;
            size_t new_x = (new_selection % COMPLETION_COLUMNS) * COMPLETION_COLUMN_WIDTH_I;
            size_t new_y = new_selection / COMPLETION_COLUMNS;
            printf("\n\r");

            // print old selection in normal colors
            if(old_y) printf(ANSI_CURSOR_DOWN_BY("%zu"), old_y);
            printf(ANSI_CURSOR_HOR_POS("%zu"), old_x + 1);
            printf(
                "%-" COMPLETION_COLUMN_WIDTH "s",
                furi_string_get_cstr(
                    *CommandCompletions_cget(completions->variants, old_selection)));
            if(old_y) printf(ANSI_CURSOR_UP_BY("%zu"), old_y);
            printf(ANSI_CURSOR_HOR_POS("1"));

            // print new selection in inverted colors
            if(new_y) printf(ANSI_CURSOR_DOWN_BY("%zu"), new_y);
            printf(ANSI_CURSOR_HOR_POS("%zu"), new_x + 1);
            printf(
                ANSI_INVERT "%-" COMPLETION_COLUMN_WIDTH "s" ANSI_RESET,
                furi_string_get_cstr(
                    *CommandCompletions_cget(completions->variants, new_selection)));

            // return cursor
            printf(ANSI_CURSOR_UP_BY("%zu"), new_y + 1);
            printf(
                ANSI_CURSOR_HOR_POS("%zu"),
                strlen(prompt) + furi_string_size(cli_shell_line_get_selected(completions->line)) +
                    1);
        }

    } else if(action == CliShellCompletionsActionSelectNoClose) {
        if(!CommandCompletions_size(completions->variants)) return;
        // insert selection into prompt
        CliShellCompletionSegment segment = cli_shell_completions_segment(completions);
        FuriString* input = cli_shell_line_get_selected(completions->line);
        FuriString* completion =
            *CommandCompletions_cget(completions->variants, completions->selected);
        furi_string_replace_at(
            input, segment.start, segment.length, furi_string_get_cstr(completion));
        printf(
            ANSI_CURSOR_HOR_POS("%zu") "%s" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END),
            strlen(prompt) + 1,
            furi_string_get_cstr(input));

        int position_change = (int)furi_string_size(completion) - (int)segment.length;
        cli_shell_line_set_line_position(
            completions->line,
            MAX(0, (int)cli_shell_line_get_line_position(completions->line) + position_change));

    } else if(action == CliShellCompletionsActionSelect) {
        cli_shell_completions_render(completions, CliShellCompletionsActionSelectNoClose);
        cli_shell_completions_render(completions, CliShellCompletionsActionClose);

    } else {
        furi_crash();
    }

    fflush(stdout);
}

// ==============
// Input handlers
// ==============

static bool hide_if_open_and_continue_handling(CliKeyCombo combo, void* context) {
    UNUSED(combo);
    CliShellCompletions* completions = context;
    if(completions->is_displaying)
        cli_shell_completions_render(completions, CliShellCompletionsActionClose);
    return false; // process other home events
}

static bool key_combo_cr(CliKeyCombo combo, void* context) {
    UNUSED(combo);
    CliShellCompletions* completions = context;
    if(!completions->is_displaying) return false;
    cli_shell_completions_render(completions, CliShellCompletionsActionSelect);
    return true;
}

static bool key_combo_up_down(CliKeyCombo combo, void* context) {
    CliShellCompletions* completions = context;
    if(!completions->is_displaying) return false;
    cli_shell_completions_render(
        completions,
        (combo.key == CliKeyUp) ? CliShellCompletionsActionUp : CliShellCompletionsActionDown);
    return true;
}

static bool key_combo_left_right(CliKeyCombo combo, void* context) {
    CliShellCompletions* completions = context;
    if(!completions->is_displaying) return false;
    cli_shell_completions_render(
        completions,
        (combo.key == CliKeyLeft) ? CliShellCompletionsActionLeft :
                                    CliShellCompletionsActionRight);
    return true;
}

static bool key_combo_tab(CliKeyCombo combo, void* context) {
    UNUSED(combo);
    CliShellCompletions* completions = context;
    cli_shell_completions_render(
        completions,
        completions->is_displaying ? CliShellCompletionsActionRight :
                                     CliShellCompletionsActionOpen);
    return true;
}

static bool key_combo_esc(CliKeyCombo combo, void* context) {
    UNUSED(combo);
    CliShellCompletions* completions = context;
    if(!completions->is_displaying) return false;
    cli_shell_completions_render(completions, CliShellCompletionsActionClose);
    return true;
}

CliShellKeyComboSet cli_shell_completions_key_combo_set = {
    .fallback = hide_if_open_and_continue_handling,
    .count = 7,
    .records =
        {
            {{CliModKeyNo, CliKeyCR}, key_combo_cr},
            {{CliModKeyNo, CliKeyUp}, key_combo_up_down},
            {{CliModKeyNo, CliKeyDown}, key_combo_up_down},
            {{CliModKeyNo, CliKeyLeft}, key_combo_left_right},
            {{CliModKeyNo, CliKeyRight}, key_combo_left_right},
            {{CliModKeyNo, CliKeyTab}, key_combo_tab},
            {{CliModKeyNo, CliKeyEsc}, key_combo_esc},
        },
};
