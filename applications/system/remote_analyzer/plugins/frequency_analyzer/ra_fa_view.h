/**
 * @file ra_fa_view.h
 * @brief Frequency analyzer screen. Ported from the Sub-GHz app's analyzer view.
 */
#pragma once

#include "../../helpers/ra_fa_plugin.h"

#include <gui/view.h>

typedef struct RaFaView RaFaView;

RaFaView* ra_fa_view_alloc(RaFaEnv* env);

void ra_fa_view_free(RaFaView* instance);

View* ra_fa_view_get_view(RaFaView* instance);

/** Copy the trigger level and feedback level back into the env, for the app to persist. */
void ra_fa_view_store(RaFaView* instance);
