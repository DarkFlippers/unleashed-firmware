#pragma once
#include "minunit.h"

/*  Misc. counters */
int minunit_run = 0;
int minunit_assert = 0;
int minunit_fail = 0;
int minunit_status = 0;

/*  Timers */
double minunit_real_timer = 0;
double minunit_proc_timer = 0;

/*  Last message */
char minunit_last_message[MINUNIT_MESSAGE_LEN];
