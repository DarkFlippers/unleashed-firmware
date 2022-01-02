#include <furi.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>

#include <gui/modules/dialog_ex.h>
#include <power/power_settings_app/views/battery_info.h>

typedef struct {
    Power* power;
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    BatteryInfo* batery_info;
    DialogEx* dialog;
    PowerInfo info;
} BatteryTestApp;

typedef enum {
    BatteryTestAppViewBatteryInfo,
    BatteryTestAppViewExitDialog,
} BatteryTestAppView;
