#pragma once

#include <gui/view.h>
#include "../pomodoro_timer.h"

PomodoroTimer* pomodoro_10_alloc();

void pomodoro_10_free(PomodoroTimer* pomodoro_10);

View* pomodoro_10_get_view(PomodoroTimer* pomodoro_10);
