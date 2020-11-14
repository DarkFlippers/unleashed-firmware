#pragma once
#include "main.h"
#include <cmsis_os.h>
#include <stdbool.h>

// Task stack size in bytes
#define DEFAULT_STACK_SIZE 4096

// Max system tasks count
#define MAX_TASK_COUNT 10

bool task_equal(TaskHandle_t a, TaskHandle_t b);
bool task_is_isr_context(void);
__attribute__((unused)) void taskDISABLE_INTERRUPTS(void);
