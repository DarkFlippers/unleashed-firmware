#ifndef __UTILS_H__
#define __UTILS_H__
#include <furi.h>
#include <notification/notification_messages.h>

void notification_beep_once();
void notification_off();
void notification_timeup();

void parse_sec_to_time_str(char* buffer, size_t len, int32_t sec);

#endif // __UTILS_H__