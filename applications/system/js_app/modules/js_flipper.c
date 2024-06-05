#include "../js_modules.h" // IWYU pragma: keep
#include <core/common_defines.h>
#include <furi_hal_version.h>
#include <power/power_service/power.h>

static void js_flipper_get_model(struct mjs* mjs) {
    mjs_val_t ret = mjs_mk_string(mjs, furi_hal_version_get_model_name(), ~0, true);
    mjs_return(mjs, ret);
}

static void js_flipper_get_name(struct mjs* mjs) {
    const char* name_str = furi_hal_version_get_name_ptr();
    if(name_str == NULL) {
        name_str = "Unknown";
    }
    mjs_val_t ret = mjs_mk_string(mjs, name_str, ~0, true);
    mjs_return(mjs, ret);
}

static void js_flipper_get_battery(struct mjs* mjs) {
    Power* power = furi_record_open(RECORD_POWER);
    PowerInfo info;
    power_get_info(power, &info);
    furi_record_close(RECORD_POWER);
    mjs_return(mjs, mjs_mk_number(mjs, info.charge));
}

void* js_flipper_create(struct mjs* mjs, mjs_val_t* object) {
    mjs_val_t flipper_obj = mjs_mk_object(mjs);
    mjs_set(mjs, flipper_obj, "getModel", ~0, MJS_MK_FN(js_flipper_get_model));
    mjs_set(mjs, flipper_obj, "getName", ~0, MJS_MK_FN(js_flipper_get_name));
    mjs_set(mjs, flipper_obj, "getBatteryCharge", ~0, MJS_MK_FN(js_flipper_get_battery));
    *object = flipper_obj;

    return (void*)1;
}
