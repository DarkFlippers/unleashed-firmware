#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IrdaProgressView IrdaProgressView;

typedef void (*IrdaProgressViewBackCallback)(void*);

IrdaProgressView* irda_progress_view_alloc();
void irda_progress_view_free(IrdaProgressView* progress);
View* irda_progress_view_get_view(IrdaProgressView* progress);

bool irda_progress_view_increase_progress(IrdaProgressView* progress);
void irda_progress_view_set_progress_total(IrdaProgressView* progress, uint16_t progress_max);
void irda_progress_view_set_back_callback(
    IrdaProgressView* instance,
    IrdaProgressViewBackCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
