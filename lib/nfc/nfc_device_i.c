#include "nfc_device_i.h"
#include "protocols/nfc_device_defs.h"

#include <furi/furi.h>

static NfcDeviceData*
    nfc_device_search_base_protocol_data(const NfcDevice* instance, NfcProtocol protocol) {
    NfcProtocol protocol_tmp = instance->protocol;
    NfcDeviceData* dev_data_tmp = instance->protocol_data;

    while(true) {
        dev_data_tmp = nfc_devices[protocol_tmp]->get_base_data(dev_data_tmp);
        protocol_tmp = nfc_protocol_get_parent(protocol_tmp);
        if(protocol_tmp == protocol) {
            break;
        }
    }

    return dev_data_tmp;
}

NfcDeviceData* nfc_device_get_data_ptr(const NfcDevice* instance, NfcProtocol protocol) {
    furi_assert(instance);
    furi_assert(protocol < NfcProtocolNum);

    NfcDeviceData* dev_data = NULL;

    if(instance->protocol == protocol) {
        dev_data = instance->protocol_data;
    } else if(nfc_protocol_has_parent(instance->protocol, protocol)) {
        dev_data = nfc_device_search_base_protocol_data(instance, protocol);
    } else {
        furi_crash("Incorrect protocol");
    }

    return dev_data;
}
