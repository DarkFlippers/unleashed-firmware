/**
 * @file date_time_input.h
 * GUI: DateTimeInput view module API
 */

#pragma once

#include <gui/view.h>
#include <datetime.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Date/time input anonymous structure */
typedef struct DateTimeInput DateTimeInput;

/** callback that is executed on value change */
typedef void (*DateTimeChangedCallback)(void* context);

/** callback that is executed on back button press */
typedef void (*DateTimeDoneCallback)(void* context);

/** Allocate and initialize date/time input
 *
 * This screen used to input a date and time
 *
 * @return     DateTimeInput instance
 */
DateTimeInput* date_time_input_alloc(void);

/** Deinitialize and free date/time input
 *
 * @param      date_time_input  Date/time input instance
 */
void date_time_input_free(DateTimeInput* date_time_input);

/** Get date/time input view
 *
 * @param      date_time_input  Date/time input instance
 *
 * @return     View instance that can be used for embedding
 */
View* date_time_input_get_view(DateTimeInput* date_time_input);

/** Set date/time input result callback
 *
 * @param      date_time_input   date/time input instance
 * @param      changed_callback  changed callback fn
 * @param      done_callback     finished callback fn
 * @param      callback_context  callback context
 * @param      datetime          date/time value
 */
void date_time_input_set_result_callback(
    DateTimeInput* date_time_input,
    DateTimeChangedCallback changed_callback,
    DateTimeDoneCallback done_callback,
    void* callback_context,
    DateTime* datetime);

/** Set date/time fields which can be edited
 *
 * @param      date_time_input  date/time input instance
 * @param      year             whether to allow editing the year
 * @param      month            whether to allow editing the month
 * @param      day              whether to allow editing the day
 * @param      hour             whether to allow editing the hour
 * @param      minute           whether to allow editing the minute
 * @param      second           whether to allow editing the second
 */
void date_time_input_set_editable_fields(
    DateTimeInput* date_time_input,
    bool year,
    bool month,
    bool day,
    bool hour,
    bool minute,
    bool second);

#ifdef __cplusplus
}
#endif
