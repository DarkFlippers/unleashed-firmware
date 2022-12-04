#ifndef BC_LOGGING_H_
#define BC_LOGGING_H_

#include <furi.h>
#include "err.h" // appName

//!  WARNING: There is a bug in Furi such that if you crank LOG_LEVEL up to 6=TRACE
//!           AND you have menu->settings->system->logLevel = trace
//!           THEN this program will cause the FZ to crash when the plugin exits!
#define LOG_LEVEL 4

//----------------------------------------------------------------------------- ----------------------------------------
// The FlipperZero Settings->System menu allows you to set the logging level at RUN-time
// ... LOG_LEVEL lets you limit it at COMPILE-time
//
// FURI logging has 6 levels (numbered 1 thru 6}
//    1.  None
//    2.  Errors       FURI_LOG_E
//    3.  Warnings     FURI_LOG_W
//    4.  Information  FURI_LOG_I
//    5.  Debug        FURI_LOG_D
//    6.  Trace        FURI_LOG_T
//
// --> furi/core/log.h
//

// The FlipperZero Settings->System menu allows you to set the logging level at RUN-time
// This lets you limit it at COMPILE-time
#ifndef LOG_LEVEL
#define LOG_LEVEL 6 // default = full logging
#endif

#if(LOG_LEVEL < 2)
#undef FURI_LOG_E
#define FURI_LOG_E(tag, fmt, ...)
#endif

#if(LOG_LEVEL < 3)
#undef FURI_LOG_W
#define FURI_LOG_W(tag, fmt, ...)
#endif

#if(LOG_LEVEL < 4)
#undef FURI_LOG_I
#define FURI_LOG_I(tag, fmt, ...)
#endif

#if(LOG_LEVEL < 5)
#undef FURI_LOG_D
#define FURI_LOG_D(tag, fmt, ...)
#endif

#if(LOG_LEVEL < 6)
#undef FURI_LOG_T
#define FURI_LOG_T(tag, fmt, ...)
#endif

//----------------------------------------------------------
// Logging helper macros
//
#define ERROR(fmt, ...) FURI_LOG_E(appName, fmt __VA_OPT__(, ) __VA_ARGS__)
#define WARN(fmt, ...) FURI_LOG_W(appName, fmt __VA_OPT__(, ) __VA_ARGS__)
#define INFO(fmt, ...) FURI_LOG_I(appName, fmt __VA_OPT__(, ) __VA_ARGS__)
#define DEBUG(fmt, ...) FURI_LOG_D(appName, fmt __VA_OPT__(, ) __VA_ARGS__)
#define TRACE(fmt, ...) FURI_LOG_T(appName, fmt __VA_OPT__(, ) __VA_ARGS__)

#define ENTER TRACE("(+) %s", __func__)
#define LEAVE TRACE("(-) %s", __func__)

#endif //BC_LOGGING_H_
