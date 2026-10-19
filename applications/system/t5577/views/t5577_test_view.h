/**
 * @file t5577_test_view.h
 * @brief Which of a tag's blocks a write can change, filling in as it is found out.
 */
#pragma once

#include <gui/view.h>

/** What is known about one block. The bar is the result forming, so there is no separate
 * progress: a cell is empty until its block is reached, filled while it is being written and
 * read back, and carries its verdict afterwards.
 */
typedef enum {
    T5577TestCellUnknown,
    T5577TestCellTesting,
    T5577TestCellWriteable,
    T5577TestCellRefused,
} T5577TestCell;

typedef struct T5577TestView T5577TestView;

T5577TestView* t5577_test_view_alloc(void);

void t5577_test_view_free(T5577TestView* instance);

View* t5577_test_view_get_view(T5577TestView* instance);

/** Back to nothing known, for a run starting. */
void t5577_test_view_reset(T5577TestView* instance);

/** What is known about one of page 0's blocks 1 to 7. */
void t5577_test_view_set_cell(T5577TestView* instance, uint8_t block, T5577TestCell cell);

/** What the test is doing right now, in its own line. */
void t5577_test_view_set_status(T5577TestView* instance, const char* status);

/** The bottom line: a warning while the test runs, and what to do about it afterwards. */
void t5577_test_view_set_hint(T5577TestView* instance, const char* hint);
