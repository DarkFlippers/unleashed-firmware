#include "picopass_device.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

#define TAG "PicopassDevice"

PicopassDevice* picopass_device_alloc() {
    PicopassDevice* picopass_dev = malloc(sizeof(PicopassDevice));
    picopass_dev->storage = furi_record_open("storage");
    picopass_dev->dialogs = furi_record_open("dialogs");
    return picopass_dev;
}

void picopass_device_clear(PicopassDevice* dev) {
    furi_assert(dev);

    picopass_device_data_clear(&dev->dev_data);
    memset(&dev->dev_data, 0, sizeof(dev->dev_data));
}

void picopass_device_free(PicopassDevice* picopass_dev) {
    furi_assert(picopass_dev);
    picopass_device_clear(picopass_dev);
    furi_record_close("storage");
    furi_record_close("dialogs");
    free(picopass_dev);
}

void picopass_device_data_clear(PicopassDeviceData* dev_data) {
    FURI_LOG_D(TAG, "picopass_device_data_clear");
    memset(&dev_data->AA1, 0, sizeof(ApplicationArea));
}
