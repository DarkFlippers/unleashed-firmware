#include <update_util/resources/manifest.h>
#include <nfc/protocols/slix/slix_i.h>
#include <nfc/protocols/iso15693_3/iso15693_3_poller_i.h>
#include <FreeRTOS.h>
#include <FreeRTOS-Kernel/include/queue.h>

#include <rpc/rpc_i.h>
#include <flipper.pb.h>

static constexpr auto unit_tests_api_table = sort(create_array_t<sym_entry>(
    API_METHOD(resource_manifest_reader_alloc, ResourceManifestReader*, (Storage*)),
    API_METHOD(resource_manifest_reader_free, void, (ResourceManifestReader*)),
    API_METHOD(resource_manifest_reader_open, bool, (ResourceManifestReader*, const char* filename)),
    API_METHOD(resource_manifest_reader_next, ResourceManifestEntry*, (ResourceManifestReader*)),
    API_METHOD(resource_manifest_reader_previous, ResourceManifestEntry*, (ResourceManifestReader*)),
    API_METHOD(slix_process_iso15693_3_error, SlixError, (Iso15693_3Error)),
    API_METHOD(iso15693_3_poller_get_data, const Iso15693_3Data*, (Iso15693_3Poller*)),
    API_METHOD(rpc_system_storage_get_error, PB_CommandStatus, (FS_Error)),
    API_METHOD(xQueueSemaphoreTake, BaseType_t, (QueueHandle_t, TickType_t)),
    API_METHOD(vQueueDelete, void, (QueueHandle_t)),
    API_METHOD(
        xQueueGenericCreate,
        QueueHandle_t,
        (const UBaseType_t, const UBaseType_t, const uint8_t)),
    API_METHOD(
        xQueueGenericSend,
        BaseType_t,
        (QueueHandle_t, const void* const, TickType_t, const BaseType_t)),
    API_VARIABLE(PB_Main_msg, PB_Main_msg_t)));
