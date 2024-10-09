#include <furi_hal_region.h>

#include <furi.h>
#include <storage/storage.h>

#include <flipper.pb.h>
#include <pb_decode.h>

#define TAG "RegionSrv"

#define SUBGHZ_REGION_FILENAME INT_PATH(".region_data")

static bool region_istream_read(pb_istream_t* istream, pb_byte_t* buf, size_t count) {
    File* file = istream->state;
    size_t ret = storage_file_read(file, buf, count);
    return count == ret;
}

static bool region_istream_decode_band(pb_istream_t* stream, const pb_field_t* field, void** arg) {
    UNUSED(field);

    FuriHalRegion* region = *arg;

    PB_Region_Band band = {0};
    if(!pb_decode(stream, PB_Region_Band_fields, &band)) {
        FURI_LOG_E(TAG, "PB Region band decode error: %s", PB_GET_ERROR(stream));
        return false;
    }

    region->bands_count += 1;
    region = realloc( //-V701
        region,
        sizeof(FuriHalRegion) + sizeof(FuriHalRegionBand) * region->bands_count);
    size_t pos = region->bands_count - 1;
    region->bands[pos].start = band.start;
    region->bands[pos].end = band.end;
    region->bands[pos].power_limit = band.power_limit;
    region->bands[pos].duty_cycle = band.duty_cycle;
    *arg = region;

    FURI_LOG_I(
        TAG,
        "Add allowed band: start %luHz, stop %luHz, power_limit %ddBm, duty_cycle %u%%",
        band.start,
        band.end,
        band.power_limit,
        band.duty_cycle);
    return true;
}

static int32_t region_load_file(void* context) {
    UNUSED(context);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    PB_Region pb_region = {0};
    pb_region.bands.funcs.decode = region_istream_decode_band;

    do {
        FileInfo fileinfo = {0};

        if(storage_common_stat(storage, SUBGHZ_REGION_FILENAME, &fileinfo) != FSE_OK ||
           fileinfo.size == 0) {
            FURI_LOG_W(TAG, "Region file missing or empty");
            break;

        } else if(!storage_file_open(file, SUBGHZ_REGION_FILENAME, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open region file");
            break;
        }

        pb_istream_t istream = {
            .callback = region_istream_read,
            .state = file,
            .errmsg = NULL,
            .bytes_left = fileinfo.size,
        };

        pb_region.bands.arg = malloc(sizeof(FuriHalRegion));

        if(!pb_decode(&istream, PB_Region_fields, &pb_region)) {
            FURI_LOG_E(TAG, "Failed to decode region file");
            free(pb_region.bands.arg);
            break;
        }

        FuriHalRegion* region = pb_region.bands.arg;

        memcpy(
            region->country_code,
            pb_region.country_code->bytes,
            MIN(pb_region.country_code->size, sizeof(region->country_code) - 1));

        furi_hal_region_set(region);

        FURI_LOG_I(TAG, "Dynamic region set: %s", region->country_code);
    } while(0);

    pb_release(PB_Region_fields, &pb_region);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return 0;
}

static void
    region_loader_release_callback(FuriThread* thread, FuriThreadState state, void* context) {
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
}

static void region_storage_callback(const void* message, void* context) {
    UNUSED(context);
    const StorageEvent* event = message;

    if(event->type == StorageEventTypeCardMount) {
        FuriThread* loader = furi_thread_alloc_ex(NULL, 2048, region_load_file, NULL);
        furi_thread_set_state_callback(loader, region_loader_release_callback);
        furi_thread_start(loader);
    }
}

int32_t region_on_system_start(void* p) {
    UNUSED(p);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_subscribe(storage_get_pubsub(storage), region_storage_callback, NULL);

    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_D(TAG, "SD Card not ready, skipping dynamic region");
        return 0;
    }

    region_load_file(NULL);
    return 0;
}
