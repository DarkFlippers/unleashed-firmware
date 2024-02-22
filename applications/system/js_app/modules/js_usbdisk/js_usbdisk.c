#include "../../js_modules.h"
#include <furi_hal_usb.h>
#include <toolbox/path.h>
#include "mass_storage_usb.h"

#define TAG "JsUsbdisk"

typedef struct {
    File* file;
    char* path;
    MassStorageUsb* usb;
    bool was_ejected;
} JsUsbdiskInst;

static bool file_read(
    void* ctx,
    uint32_t lba,
    uint16_t count,
    uint8_t* out,
    uint32_t* out_len,
    uint32_t out_cap) {
    JsUsbdiskInst* usbdisk = ctx;
    FURI_LOG_T(TAG, "file_read lba=%08lX count=%04X out_cap=%08lX", lba, count, out_cap);
    if(!storage_file_seek(usbdisk->file, lba * SCSI_BLOCK_SIZE, true)) {
        FURI_LOG_W(TAG, "seek failed");
        return false;
    }
    uint16_t clamp = MIN(out_cap, count * SCSI_BLOCK_SIZE);
    *out_len = storage_file_read(usbdisk->file, out, clamp);
    FURI_LOG_T(TAG, "%lu/%lu", *out_len, count * SCSI_BLOCK_SIZE);
    return *out_len == clamp;
}

static bool file_write(void* ctx, uint32_t lba, uint16_t count, uint8_t* buf, uint32_t len) {
    JsUsbdiskInst* usbdisk = ctx;
    FURI_LOG_T(TAG, "file_write lba=%08lX count=%04X len=%08lX", lba, count, len);
    if(len != count * SCSI_BLOCK_SIZE) {
        FURI_LOG_W(TAG, "bad write params count=%u len=%lu", count, len);
        return false;
    }
    if(!storage_file_seek(usbdisk->file, lba * SCSI_BLOCK_SIZE, true)) {
        FURI_LOG_W(TAG, "seek failed");
        return false;
    }
    return storage_file_write(usbdisk->file, buf, len) == len;
}

static uint32_t file_num_blocks(void* ctx) {
    JsUsbdiskInst* usbdisk = ctx;
    return storage_file_size(usbdisk->file) / SCSI_BLOCK_SIZE;
}

static void file_eject(void* ctx) {
    JsUsbdiskInst* usbdisk = ctx;
    FURI_LOG_D(TAG, "EJECT");
    usbdisk->was_ejected = true;
}

static void js_usbdisk_internal_stop_free(JsUsbdiskInst* usbdisk) {
    if(usbdisk->usb) {
        mass_storage_usb_stop(usbdisk->usb);
        usbdisk->usb = NULL;
    }
    if(usbdisk->file) {
        storage_file_free(usbdisk->file);
        furi_record_close(RECORD_STORAGE);
        usbdisk->file = NULL;
    }
    if(usbdisk->path) {
        free(usbdisk->path);
        usbdisk->path = NULL;
    }
}

static void js_usbdisk_start(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsUsbdiskInst* usbdisk = mjs_get_ptr(mjs, obj_inst);
    furi_assert(usbdisk);

    if(usbdisk->usb) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "SCSI is already started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* error = NULL;
    do {
        if(mjs_nargs(mjs) != 1) {
            error = "Wrong argument count";
            break;
        }

        mjs_val_t path_arg = mjs_arg(mjs, 0);
        if(!mjs_is_string(path_arg)) {
            error = "Path must be a string";
            break;
        }

        size_t path_len = 0;
        const char* path = mjs_get_string(mjs, &path_arg, &path_len);
        if((path_len == 0) || (path == NULL)) {
            error = "Bad path argument";
            break;
        }
        usbdisk->path = strdup(path);

        usbdisk->file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
        if(!storage_file_open(
               usbdisk->file, usbdisk->path, FSAM_READ | FSAM_WRITE, FSOM_OPEN_EXISTING)) {
            error = storage_file_get_error_desc(usbdisk->file);
            break;
        }
    } while(0);

    if(error) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
        js_usbdisk_internal_stop_free(usbdisk);
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    SCSIDeviceFunc fn = {
        .ctx = usbdisk,
        .read = file_read,
        .write = file_write,
        .num_blocks = file_num_blocks,
        .eject = file_eject,
    };

    furi_hal_usb_unlock();
    usbdisk->was_ejected = false;
    FuriString* name = furi_string_alloc();
    path_extract_filename_no_ext(usbdisk->path, name);
    usbdisk->usb = mass_storage_usb_start(furi_string_get_cstr(name), fn);
    furi_string_free(name);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_usbdisk_was_ejected(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsUsbdiskInst* usbdisk = mjs_get_ptr(mjs, obj_inst);
    furi_assert(usbdisk);

    if(!usbdisk->usb) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "SCSI is not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_return(mjs, mjs_mk_boolean(mjs, usbdisk->was_ejected));
}

static void js_usbdisk_stop(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsUsbdiskInst* usbdisk = mjs_get_ptr(mjs, obj_inst);
    furi_assert(usbdisk);

    if(!usbdisk->usb) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "SCSI is not started");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    js_usbdisk_internal_stop_free(usbdisk);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void* js_usbdisk_create(struct mjs* mjs, mjs_val_t* object) {
    JsUsbdiskInst* usbdisk = malloc(sizeof(JsUsbdiskInst));
    mjs_val_t usbdisk_obj = mjs_mk_object(mjs);
    mjs_set(mjs, usbdisk_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, usbdisk));
    mjs_set(mjs, usbdisk_obj, "start", ~0, MJS_MK_FN(js_usbdisk_start));
    mjs_set(mjs, usbdisk_obj, "stop", ~0, MJS_MK_FN(js_usbdisk_stop));
    mjs_set(mjs, usbdisk_obj, "wasEjected", ~0, MJS_MK_FN(js_usbdisk_was_ejected));
    *object = usbdisk_obj;
    return usbdisk;
}

static void js_usbdisk_destroy(void* inst) {
    JsUsbdiskInst* usbdisk = inst;
    js_usbdisk_internal_stop_free(usbdisk);
    free(usbdisk);
}

static const JsModuleDescriptor js_usbdisk_desc = {
    "usbdisk",
    js_usbdisk_create,
    js_usbdisk_destroy,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_usbdisk_desc,
};

const FlipperAppPluginDescriptor* js_usbdisk_ep(void) {
    return &plugin_descriptor;
}
