#pragma once
#include "main.h"
#include <cmsis_os2.h>
#include <stdbool.h>

// Task stack size in bytes
#define DEFAULT_STACK_SIZE 4096

// Max system tasks count
#define MAX_TASK_COUNT 14

bool task_is_isr_context(void);
