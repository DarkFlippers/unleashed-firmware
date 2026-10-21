/**
 * @file t5577_dump_view.h
 * @brief The blocks of a dump, a page at a time, and what block 0 says.
 */
#pragma once

#include <gui/view.h>
#include <lfrfid/tools/t5577_file.h>

typedef struct T5577DumpView T5577DumpView;

T5577DumpView* t5577_dump_view_alloc(void);

void t5577_dump_view_free(T5577DumpView* instance);

View* t5577_dump_view_get_view(T5577DumpView* instance);

/** Show this dump, from its first tab. Takes a copy: the dump may change under the view. */
void t5577_dump_view_set_dump(T5577DumpView* instance, const T5577Dump* dump);
