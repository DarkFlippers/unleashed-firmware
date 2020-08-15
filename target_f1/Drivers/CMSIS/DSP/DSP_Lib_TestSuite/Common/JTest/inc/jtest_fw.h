#ifndef _JTEST_FW_H_
#define _JTEST_FW_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include <stdint.h>             /* int32_t */
#include <string.h>             /* strcpy() */
#include <stdio.h>              /* sprintf() */
#include "jtest_pf.h"           /* Extend JTEST_FW_t with Pass/Fail data */
#include "jtest_group.h"

/*--------------------------------------------------------------------------------*/
/* Type Definitions */
/*--------------------------------------------------------------------------------*/

/**
 *  A struct used to interface with the Keil Debugger.
 */
typedef struct JTEST_FW_struct
{
    /* Action Triggers: The Keil debugger monitors these values for changes.  In
     * response to a change, the debugger executes code on the host. */
    volatile int32_t test_start;
    volatile int32_t test_end;
    volatile int32_t group_start;
    volatile int32_t group_end;
    volatile int32_t dump_str;
    volatile int32_t dump_data;
    volatile int32_t exit_fw;

    JTEST_GROUP_t * current_group_ptr;

    /* Buffers: The C-code cannot send strings and data directly to the
     * debugging framework. Instead, the debugger can be told to read 128 byte
     * (by default) chunks of memory.  Data received in this manner requires
     * post-processing to be legible.*/
    char * str_buffer;
    char * data_buffer;

    /* Pass/Fail Data */
    JTEST_PF_MEMBERS;

} JTEST_FW_t;

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Default name for the JTEST_FW struct.
 *
 *  Define your own if you want the variable containing the #JTEST_FW_t to have
 *  a different name.
 */
#ifndef JTEST_FW
#define JTEST_FW JTEST_FW
#endif

/**
 *  Default name for the JTEST_FW_STR_BUFFER.
 *
 *  Define your own if you want the variable containing the char buffer to have
 *  a different name.
 */
#ifndef JTEST_FW_STR_BUFFER
#define JTEST_FW_STR_BUFFER JTEST_FW_STR_BUFFER
#endif

/**
 *  Size of the #JTEST_FW_t, output string-buffer.
 *
 *  If you change this value, make sure the "dump_str_fn" and "dump_data_fn"
 *  functions in jtest_fns.ini uses the same size. If you aren't sure, read the
 *  documentation Keil Debugger Command 'DISPLAY'.
 */
#define JTEST_BUF_SIZE 256


/**
 *  The maximum number of bytes output at once using #JTEST_DUMP_STRF().
 */
#define JTEST_STR_MAX_OUTPUT_SIZE 128

/**
 *  The maximum number of block transimissions needed to send a string from a
 *  buffer with JTEST_BUF_SIZE.
 */
#define JTEST_STR_MAX_OUTPUT_SEGMENTS           \
    (JTEST_BUF_SIZE / JTEST_STR_MAX_OUTPUT_SIZE)

/**
 *  Initialize the JTEST framework.
 */
#define JTEST_INIT()                                                    \
    do                                                                  \
    {                                                                   \
        JTEST_FW.str_buffer = JTEST_FW_STR_BUFFER;                      \
    } while (0)

/* Debugger Action-triggering Macros */
/*--------------------------------------------------------------------------------*/

/**
 *  Dispatch macro to trigger various actions in the Keil Debugger.
 */
#define JTEST_TRIGGER_ACTION(action_name)       \
    do                                          \
    {                                           \
        action_name();                          \
    } while (0)

/**
 *  Trigger the "Test Start" action in the Keil Debugger.
 */
#define JTEST_ACT_TEST_START()                  \
    JTEST_TRIGGER_ACTION(test_start)

/**
 *  Trigger the "Test End" action in the Keil Debugger.
 */
#define JTEST_ACT_TEST_END()                    \
    JTEST_TRIGGER_ACTION(test_end)


/**
 *  Trigger the "Group Start" action in the Keil Debugger.
 */
#define JTEST_ACT_GROUP_START()                 \
    JTEST_TRIGGER_ACTION(group_start)

/**
 *  Trigger the "Group End" action in the Keil Debugger.
 */
#define JTEST_ACT_GROUP_END()                   \
    JTEST_TRIGGER_ACTION(group_end)


/**
 *  Fill the buffer named buf_name with value and dump it to the Keil debugger
 *  using action.
 */
#define JTEST_ACT_DUMP(action, buf_name, value) \
    do                                          \
    {                                           \
        JTEST_CLEAR_BUFFER(buf_name);           \
        strcpy(JTEST_FW.buf_name, (value));     \
        JTEST_TRIGGER_ACTION(action);           \
    } while (0)

/**
 *  Trigger the "Exit Framework" action in the Keil Debugger.
 */
#define JTEST_ACT_EXIT_FW()                     \
    do                                          \
    {                                           \
        JTEST_TRIGGER_ACTION(exit_fw);          \
    } while (0)


/* Buffer Manipulation Macros */
/*--------------------------------------------------------------------------------*/

/**
 *  Clear the JTEST_FW buffer with name buf_name.
 */
#define JTEST_CLEAR_BUFFER(buf_name)                    \
    do                                                  \
    {                                                   \
        memset(JTEST_FW.buf_name, 0, JTEST_BUF_SIZE);   \
    } while (0)

/**
 *  Clear the memory needed for the JTEST_FW's string buffer.
 */
#define JTEST_CLEAR_STR_BUFFER()                \
        JTEST_CLEAR_BUFFER(str_buffer)

/**
 *  Clear the memory needed for the JTEST_FW's data buffer.
 */
#define JTEST_CLEAR_DATA_BUFFER()               \
        JTEST_CLEAR_BUFFER(data_buffer)

/**
 *  Dump the given string to the Keil Debugger.
 */
#define JTEST_DUMP_STR(string)                          \
        JTEST_ACT_DUMP(dump_str, str_buffer, string)

/**
 *  Dump a formatted string to the Keil Debugger.
 */
#define JTEST_DUMP_STRF(format_str, ... )                               \
    do                                                                  \
    {                                                                   \
        JTEST_CLEAR_STR_BUFFER();                                       \
        sprintf(JTEST_FW.str_buffer,format_str, __VA_ARGS__);           \
        jtest_dump_str_segments();                                      \
    } while (0)

/* Pass/Fail Macros */
/*--------------------------------------------------------------------------------*/

/**
 *  Increment the number of passed tests in #JTEST_FW.
 */
#define JTEST_FW_INC_PASSED(amount)             \
        JTEST_PF_INC_PASSED(&JTEST_FW, amount)

/**
 *  Increment the number of passed tests in #JTEST_FW.
 */
#define JTEST_FW_INC_FAILED(amount)             \
        JTEST_PF_INC_FAILED(&JTEST_FW, amount)

/* Manipulating the Current Group */
/*--------------------------------------------------------------------------------*/

/**
 *  Evaluate to the current_group_ptr in #JTEST_FW.
 */
#define JTEST_CURRENT_GROUP_PTR()               \
    (JTEST_FW.current_group_ptr)

#define JTEST_SET_CURRENT_GROUP(group_ptr)      \
    do                                          \
    {                                           \
        JTEST_CURRENT_GROUP_PTR() = group_ptr;  \
    } while (0)

/*--------------------------------------------------------------------------------*/
/* Declare Global Variables */
/*--------------------------------------------------------------------------------*/
extern char JTEST_FW_STR_BUFFER[JTEST_BUF_SIZE];
extern volatile JTEST_FW_t JTEST_FW;

/*--------------------------------------------------------------------------------*/
/* Function Prototypes */
/*--------------------------------------------------------------------------------*/
void jtest_dump_str_segments(void);

void test_start  (void);
void test_end    (void);
void group_start (void);
void group_end   (void);
void dump_str    (void);
void dump_data   (void);
void exit_fw     (void);


#endif /* _JTEST_FW_H_ */
