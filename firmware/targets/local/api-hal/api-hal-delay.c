#include "api-hal-delay.h"
#include <stdio.h>
#include <unistd.h>

void delay_us(float microseconds) {
    usleep(microseconds);
}

void delay(float milliseconds) {
    usleep(milliseconds * 1000);
}