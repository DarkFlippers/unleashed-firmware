#include "subghz_i.h"

const uint32_t subghz_frequencies[] = {
    /* 300 - 348 */
    300000000,
    315000000,
    348000000,
    /* 387 - 464 */
    387000000,
    433075000, /* LPD433 first */
    433920000, /* LPD433 mid */
    434775000, /* LPD433 last channels */
    438900000,
    464000000,
    /* 779 - 928 */
    779000000,
    868350000,
    915000000,
    925000000,
    928000000,
};

const uint32_t subghz_frequencies_count = sizeof(subghz_frequencies) / sizeof(uint32_t);
const uint32_t subghz_frequencies_433_92 = 5;

void subghz_menu_callback(void* context, uint32_t index) {
    furi_assert(context);
    SubGhz* subghz = context;

    view_dispatcher_switch_to_view(subghz->view_dispatcher, index);
}

uint32_t subghz_exit(void* context) {
    return VIEW_NONE;
}

SubGhz* subghz_alloc() {
    SubGhz* subghz = furi_alloc(sizeof(SubGhz));

    // GUI
    subghz->gui = furi_record_open("gui");

    // View Dispatcher
    subghz->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(subghz->view_dispatcher);
    view_dispatcher_attach_to_gui(
        subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeFullscreen);

    // Menu
    subghz->submenu = submenu_alloc();
    submenu_add_item(subghz->submenu, "Capture", SubGhzViewCapture, subghz_menu_callback, subghz);
    submenu_add_item(
        subghz->submenu, "Basic Test", SubGhzViewTestBasic, subghz_menu_callback, subghz);
    submenu_add_item(
        subghz->submenu, "Packet Test", SubGhzViewTestPacket, subghz_menu_callback, subghz);
    submenu_add_item(
        subghz->submenu, "Static Code", SubGhzViewStatic, subghz_menu_callback, subghz);

    View* submenu_view = submenu_get_view(subghz->submenu);
    view_set_previous_callback(submenu_view, subghz_exit);
    view_dispatcher_add_view(subghz->view_dispatcher, SubGhzViewMenu, submenu_view);

    // Capture
    subghz->subghz_capture = subghz_capture_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewCapture,
        subghz_capture_get_view(subghz->subghz_capture));

    // Basic Test Module
    subghz->subghz_test_basic = subghz_test_basic_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewTestBasic,
        subghz_test_basic_get_view(subghz->subghz_test_basic));

    // Packet Test
    subghz->subghz_test_packet = subghz_test_packet_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewTestPacket,
        subghz_test_packet_get_view(subghz->subghz_test_packet));

    // Static send
    subghz->subghz_static = subghz_static_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewStatic, subghz_static_get_view(subghz->subghz_static));

    // Switch to menu
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewMenu);

    return subghz;
}

void subghz_free(SubGhz* subghz) {
    furi_assert(subghz);

    // Packet Test
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewStatic);
    subghz_static_free(subghz->subghz_static);

    // Packet Test
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewTestPacket);
    subghz_test_packet_free(subghz->subghz_test_packet);

    // Basic Test
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewTestBasic);
    subghz_test_basic_free(subghz->subghz_test_basic);

    // Submenu
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewMenu);
    submenu_free(subghz->submenu);

    // Capture
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewCapture);
    subghz_capture_free(subghz->subghz_capture);

    // View Dispatcher
    view_dispatcher_free(subghz->view_dispatcher);

    // GUI
    furi_record_close("gui");
    subghz->gui = NULL;

    // The rest
    free(subghz);
}

int32_t subghz_app(void* context) {
    SubGhz* subghz = subghz_alloc();

    view_dispatcher_run(subghz->view_dispatcher);

    subghz_free(subghz);

    return 0;
}