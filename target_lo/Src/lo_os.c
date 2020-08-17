#include "cmsis_os.h"
#include <unistd.h>
#include <stdio.h>

void osDelay(uint32_t ms) {
	usleep(ms * 1000);
	printf("[DELAY] %d ms\n", ms);
}