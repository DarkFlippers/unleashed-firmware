// Avoid circular/nested/mulitple inclusion
#ifndef ERR_H_
#define ERR_H_

//----------------------------------------------------------------------------- ----------------------------------------
// Application name
//
static const char* const appName = "Wii_i2c"; //$ Name used in log files

//----------------------------------------------------------------------------- ----------------------------------------
// Error codes and messages
//

// You should only ever (need to) edit this list
// ...Watch out for extraneous whitespace after the terminating backslashes
#define FOREACH_ES(esPrial)                                                                       \
    /* The first line MUST define 'ERR_OK = 0' */                                                 \
    esPrial(0, ERR_OK, "OK (no error)")                                                           \
                                                                                                  \
        esPrial(1, ERR_MALLOC_QUEUE, "malloc() fail - queue") esPrial(                            \
            2,                                                                                    \
            ERR_MALLOC_STATE,                                                                     \
            "malloc() fail - state") esPrial(3, ERR_MALLOC_TEXT, "malloc() fail - text")          \
            esPrial(4, ERR_MALLOC_VIEW, "malloc() fail - viewport") esPrial(                      \
                5, ERR_NO_MUTEX, "Cannot create mutex") esPrial(6, ERR_NO_GUI, "Cannot open GUI") \
                esPrial(7, ERR_NO_TIMER, "Cannot create timer") esPrial(                          \
                    8, ERR_NO_NOTIFY, "Cannot acquire notifications handle")                      \
                                                                                                  \
                    esPrial(10, ERR_MUTEX_BLOCK, "Mutex block failed") esPrial(                   \
                        11, ERR_MUTEX_RELEASE, "Mutex release failed")                            \
                                                                                                  \
                        esPrial(20, ERR_QUEUE_RTOS, "queue - Undefined RTOS error")               \
                            esPrial(21, DEBUG_QUEUE_TIMEOUT, "queue - Timeout") esPrial(          \
                                22, ERR_QUEUE_RESOURCE, "queue - Resource not available")         \
                                esPrial(23, ERR_QUEUE_BADPRM, "queue - Bad parameter") esPrial(   \
                                    24, ERR_QUEUE_NOMEM, "queue - Out of memory")                 \
                                    esPrial(25, ERR_QUEUE_ISR, "queue - Banned in ISR") esPrial(  \
                                        26, ERR_QUEUE_UNK, "queue - Unknown")                     \
                                                                                                  \
                                        esPrial(30, WARN_SCAN_START, "Scan - Already started")    \
                                            esPrial(31, WARN_SCAN_STOP, "Scan - Already stopped") \
                                                esPrial(                                          \
                                                    32,                                           \
                                                    ERR_TIMER_START,                              \
                                                    "Scan - Cannot start timer")                  \
                                                    esPrial(                                      \
                                                        33,                                       \
                                                        ERR_TIMER_STOP,                           \
                                                        "Scan - Cannot stop timer") //[EOT]

// Declare list extraction macros
#define ES_ENUM(num, ename, string) ename = num,
#define ES_STRING(num, ename, string) string "\r\n",

// Build the enum
typedef enum err { FOREACH_ES(ES_ENUM) } err_t;

// You need to '#define ERR_C_' in precisely ONE source file
#ifdef ERR_C_
// Build the string list
const char* const wii_errs[] = {FOREACH_ES(ES_STRING)};
#else
// Give access to string list
extern const char* const wii_errs[];
#endif

// This is a header file, clean up
#undef ES_ENUM
#undef ES_STRING
#undef FOREACH_ES

#endif // ERR_H_
