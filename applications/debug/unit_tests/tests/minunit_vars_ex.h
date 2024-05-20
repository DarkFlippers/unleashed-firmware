#pragma once
#include "minunit.h"

/*  Misc. counters */
extern int minunit_run;
extern int minunit_assert;
extern int minunit_fail;
extern int minunit_status;

/*  Timers */
extern double minunit_real_timer;
extern double minunit_proc_timer;

/*  Last message */
extern char minunit_last_message[MINUNIT_MESSAGE_LEN];
