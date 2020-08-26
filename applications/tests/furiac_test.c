#include <stdio.h>
#include <string.h>
#include "flipper.h"
#include "debug.h"

/*
Test: creating and killing task

1. create task
2. delay 10 ms
3. kill task
4. check that value changes
5. delay 2 ms
6. check that value stay unchanged
*/

void create_kill_app(void* p) {
    // this app simply increase counter
    uint8_t* counter = (uint8_t*)p;
    while(1) {
        *counter = *counter + 1;
        delay(1);
    }
}

bool furi_ac_create_kill(FILE* debug_uart) {
    uint8_t counter = 0;

    uint8_t value_a = counter;

    FuriApp* widget = furiac_start(create_kill_app, "create_kill_app", (void*)&counter);
    if(widget == NULL) {
        fprintf(debug_uart, "create widget fail\n");
        return false;
    }

    delay(10);

    if(!furiac_kill(widget)) {
        fprintf(debug_uart, "kill widget fail\n");
        return false;
    }

    if(value_a == counter) {
        fprintf(debug_uart, "counter unchanged\n");
        return false;
    }

    value_a = counter;

    delay(10);

    if(value_a != counter) {
        fprintf(debug_uart, "counter changes after kill (counter = %d vs %d)\n", value_a, counter);
        return false;
    }

    return true;
}

/*
Test: switch between tasks
1. init s
2. create task A, add 'A" to sequence'
3. switch to task B, add 'B' to sequence
4. exit from task B -> switch to A and add 'A' to sequence
5. cleanup: exit from task A
6. check sequence
*/

#define TEST_SWITCH_CONTEXT_SEQ_SIZE 8

typedef struct {
    char sequence[TEST_SWITCH_CONTEXT_SEQ_SIZE];
    size_t count;
} TestSwitchSequence;

void task_a(void*);
void task_b(void*);

void task_a(void *p) {
    // simply starts, add 'A' letter to sequence and switch
    // if sequence counter = 0, call task B, exit otherwise

    TestSwitchSequence* seq = (TestSwitchSequence*)p;

    seq->sequence[seq->count] = 'A';
    seq->count++;

    if(seq->count == 1) {
        furiac_switch(task_b, "task B", p);

        // if switch unsuccessfull, this code will executed
        seq->sequence[seq->count] = 'x';
        seq->count++;
    } else {
        // add '/' symbol on exit
        seq->sequence[seq->count] = '/';
        seq->count++;
        furiac_exit(NULL);
    }
}

// application simply add 'B' end exit
void task_b(void* p) {
    TestSwitchSequence* seq = (TestSwitchSequence*)p;

    seq->sequence[seq->count] = 'B';
    seq->count++;

    furiac_exit(p);
}

bool furi_ac_switch_exit(FILE* debug_uart) {
    // init sequence
    TestSwitchSequence seq;
    seq.count = 0;

    furiac_start(task_a, "task A", (void*)&seq);
    // TODO how to check that all child task ends?
    
    delay(10); // wait while task do its work

    seq.sequence[seq.count] = '\0';

    if(strcmp(seq.sequence, "ABA/") != 0) {
        fprintf(debug_uart, "wrong sequence: %s\n", seq.sequence);
        return false;
    }

    return true;
}