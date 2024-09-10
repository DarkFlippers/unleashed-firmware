#include "../gpio_app_i.h"
#include "../usb_uart_bridge.h"

typedef struct {
    UsbUartConfig cfg;
    UsbUartState state;
} SceneUsbUartBridge;

static SceneUsbUartBridge* scene_usb_uart;

typedef enum {
    UsbUartSceneStateInitialize,
    UsbUartSceneStateKeep,
} UsbUartSceneState;

void gpio_scene_usb_uart_callback(GpioCustomEvent event, void* context) {
    furi_assert(context);
    GpioApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void gpio_scene_usb_uart_on_enter(void* context) {
    GpioApp* app = context;
    UsbUartSceneState state =
        scene_manager_get_scene_state(app->scene_manager, GpioAppViewUsbUart);
    if(state == UsbUartSceneStateInitialize) {
        scene_usb_uart = malloc(sizeof(SceneUsbUartBridge));
        scene_usb_uart->cfg.vcp_ch = 0;
        scene_usb_uart->cfg.uart_ch = 0;
        scene_usb_uart->cfg.flow_pins = 0;
        scene_usb_uart->cfg.baudrate_mode = 0;
        scene_usb_uart->cfg.baudrate = 0;
        app->usb_uart_bridge = usb_uart_enable(&scene_usb_uart->cfg);
    }

    usb_uart_get_config(app->usb_uart_bridge, &scene_usb_uart->cfg);
    usb_uart_get_state(app->usb_uart_bridge, &scene_usb_uart->state);

    gpio_usb_uart_set_callback(app->gpio_usb_uart, gpio_scene_usb_uart_callback, app);
    scene_manager_set_scene_state(app->scene_manager, GpioSceneUsbUart, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewUsbUart);
    notification_message(app->notifications, &sequence_display_backlight_enforce_on);
}

bool gpio_scene_usb_uart_on_event(void* context, SceneManagerEvent event) {
    GpioApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GpioUsbUartEventConfig) {
            scene_manager_set_scene_state(
                app->scene_manager, GpioSceneUsbUart, UsbUartSceneStateKeep);
            scene_manager_next_scene(app->scene_manager, GpioSceneUsbUartCfg);
        } else if(event.event == GpioUsbUartEventStop) {
            scene_manager_set_scene_state(
                app->scene_manager, GpioSceneUsbUart, UsbUartSceneStateInitialize);
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, GpioSceneStart);
        }
        return true;
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(app->scene_manager, GpioSceneUsbUart, UsbUartSceneStateKeep);
        scene_manager_next_scene(app->scene_manager, GpioSceneExitConfirm);
        return true;
    } else if(event.type == SceneManagerEventTypeTick) {
        uint32_t tx_cnt_last = scene_usb_uart->state.tx_cnt;
        uint32_t rx_cnt_last = scene_usb_uart->state.rx_cnt;
        usb_uart_get_state(app->usb_uart_bridge, &scene_usb_uart->state);
        gpio_usb_uart_update_state(
            app->gpio_usb_uart, &scene_usb_uart->cfg, &scene_usb_uart->state);
        if(tx_cnt_last != scene_usb_uart->state.tx_cnt)
            notification_message(app->notifications, &sequence_blink_blue_10);
        if(rx_cnt_last != scene_usb_uart->state.rx_cnt)
            notification_message(app->notifications, &sequence_blink_green_10);
    }
    return false;
}

void gpio_scene_usb_uart_on_exit(void* context) {
    GpioApp* app = context;
    uint32_t state = scene_manager_get_scene_state(app->scene_manager, GpioSceneUsbUart);
    if(state == UsbUartSceneStateInitialize) {
        usb_uart_disable(app->usb_uart_bridge);
        free(scene_usb_uart);
    }
    notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
}
