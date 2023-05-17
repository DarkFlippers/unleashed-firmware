#pragma once

typedef struct Desktop Desktop;

#define RECORD_DESKTOP "desktop"

bool desktop_api_is_locked(Desktop* instance);

void desktop_api_unlock(Desktop* instance);
