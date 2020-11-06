#pragma once
#include "main.h"
#include <cmsis_os.h>
#include <stdbool.h>

bool task_equal(TaskHandle_t a, TaskHandle_t b);
bool task_is_isr_context(void);
