#pragma once

#include <gui/view.h>
#include "../pomodoro_timer.h"

PomodoroTimer* pomodoro_50_alloc();

void pomodoro_50_free(PomodoroTimer* pomodoro_50);

View* pomodoro_50_get_view(PomodoroTimer* pomodoro_50);
