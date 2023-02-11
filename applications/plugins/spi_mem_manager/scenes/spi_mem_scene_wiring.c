#include "../spi_mem_app_i.h"
#include "../lib/spi/spi_mem_chip.h"

void spi_mem_scene_wiring_on_enter(void* context) {
    SPIMemApp* app = context;
    widget_add_icon_element(app->widget, 0, 0, &I_Wiring_SPI_128x64);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewWidget);
}

bool spi_mem_scene_wiring_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void spi_mem_scene_wiring_on_exit(void* context) {
    SPIMemApp* app = context;
    widget_reset(app->widget);
}
