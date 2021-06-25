#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IrdaAppPopupBrut IrdaAppPopupBrut;

void popup_brut_increase_progress(IrdaAppPopupBrut* popup_brut);
IrdaAppPopupBrut* popup_brut_alloc();
void popup_brut_free(IrdaAppPopupBrut* popup_brut);
void popup_brut_draw_callback(Canvas* canvas, void* model);
void popup_brut_set_progress_max(IrdaAppPopupBrut* popup_brut, uint16_t progress_max);

#ifdef __cplusplus
}
#endif
