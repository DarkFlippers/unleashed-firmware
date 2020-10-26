#pragma once

typedef struct Gui Gui;

void gui_update(Gui* gui);
void gui_lock(Gui* gui);
void gui_unlock(Gui* gui);