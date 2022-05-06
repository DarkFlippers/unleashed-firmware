#pragma once
#include <furi.h>
#include <notification/notification_messages.h>

#ifdef __cplusplus
extern "C" {
#endif

void sd_notify_wait(NotificationApp* notifications);
void sd_notify_wait_off(NotificationApp* notifications);
void sd_notify_success(NotificationApp* notifications);
void sd_notify_eject(NotificationApp* notifications);
void sd_notify_error(NotificationApp* notifications);

#ifdef __cplusplus
}
#endif
