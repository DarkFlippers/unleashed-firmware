#pragma once

#include <gui/view.h>
#include "../pomodoro_timer.h"

PomodoroTimer* pomodoro_25_alloc();

void pomodoro_25_free(PomodoroTimer* pomodoro_25);

View* pomodoro_25_get_view(PomodoroTimer* pomodoro_25);
