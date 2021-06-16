#include "subghz_i.h"

const SubGhzFrequency subghz_frequencies[] = {
    /* 301 */
    {
        .frequency = 301000000,
        .path = ApiHalSubGhzPath315,
    },
    /* 315 */
    {
        .frequency = 315000000,
        .path = ApiHalSubGhzPath315,
    },
    /* 346 - 385 */
    {
        .frequency = 346000000,
        .path = ApiHalSubGhzPath315,
    },
    {
        .frequency = 385000000,
        .path = ApiHalSubGhzPath315,
    },
    /* LPD433 first, mid, last channels */
    {
        .frequency = 433075000,
        .path = ApiHalSubGhzPath433,
    },
    {
        .frequency = 433920000,
        .path = ApiHalSubGhzPath433,
    },
    {
        .frequency = 434775000,
        .path = ApiHalSubGhzPath433,
    },
    /* 438.9 - 781 */
    {
        .frequency = 438900000,
        .path = ApiHalSubGhzPath433,
    },
    {
        .frequency = 463000000,
        .path = ApiHalSubGhzPath433,
    },
    {
        .frequency = 781000000,
        .path = ApiHalSubGhzPath868,
    },
    /* 868.35 */
    {
        .frequency = 868350000,
        .path = ApiHalSubGhzPath868,
    },
    /* 915 */
    {
        .frequency = 915000000,
        .path = ApiHalSubGhzPath868,
    },
    /* 925 */
    {
        .frequency = 925000000,
        .path = ApiHalSubGhzPath868,
    },
};

const uint32_t subghz_frequencies_count = sizeof(subghz_frequencies) / sizeof(SubGhzFrequency);
const uint32_t subghz_frequencies_433_92 = 5;

void subghz_menu_callback(void* context, uint32_t index) {
    furi_assert(context);

    SubGhz* subghz = context;

    if(index == 0) {
        view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewTestBasic);
    } else if(index == 1) {
        view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewTestPacket);
    } else if(index == 2) {
        view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewStatic);
    }
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
    submenu_add_item(subghz->submenu, "Basic Test", 0, subghz_menu_callback, subghz);
    submenu_add_item(subghz->submenu, "Packet Test", 1, subghz_menu_callback, subghz);
    submenu_add_item(subghz->submenu, "Static Code", 2, subghz_menu_callback, subghz);

    View* submenu_view = submenu_get_view(subghz->submenu);
    view_set_previous_callback(submenu_view, subghz_exit);
    view_dispatcher_add_view(subghz->view_dispatcher, SubGhzViewMenu, submenu_view);

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