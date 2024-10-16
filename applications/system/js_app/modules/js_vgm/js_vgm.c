#include "../../js_modules.h"
#include <furi.h>
#include <toolbox/path.h>
#include "imu.h"

#define TAG "JsVgm"

typedef struct {
    Imu* imu;
    bool present;
} JsVgmInst;

static void js_vgm_get_pitch(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsVgmInst* vgm = mjs_get_ptr(mjs, obj_inst);
    furi_assert(vgm);

    if(vgm->present) {
        mjs_return(mjs, mjs_mk_number(mjs, imu_pitch_get(vgm->imu)));
    } else {
        FURI_LOG_T(TAG, "VGM not present.");
        mjs_return(mjs, mjs_mk_undefined());
    }
}

static void js_vgm_get_roll(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsVgmInst* vgm = mjs_get_ptr(mjs, obj_inst);
    furi_assert(vgm);

    if(vgm->present) {
        mjs_return(mjs, mjs_mk_number(mjs, imu_roll_get(vgm->imu)));
    } else {
        FURI_LOG_T(TAG, "VGM not present.");
        mjs_return(mjs, mjs_mk_undefined());
    }
}

static void js_vgm_get_yaw(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsVgmInst* vgm = mjs_get_ptr(mjs, obj_inst);
    furi_assert(vgm);

    if(vgm->present) {
        mjs_return(mjs, mjs_mk_number(mjs, imu_yaw_get(vgm->imu)));
    } else {
        FURI_LOG_T(TAG, "VGM not present.");
        mjs_return(mjs, mjs_mk_undefined());
    }
}

static float distance(float current, float start, float angle) {
    // make 0 to 359.999...
    current = (current < 0) ? current + 360 : current;
    start = (start < 0) ? start + 360 : start;

    // get max and min
    bool max_is_current = current > start;
    float max = (max_is_current) ? current : start;
    float min = (max_is_current) ? start : current;

    // get diff, check if it's greater than 180, and adjust
    float diff = max - min;
    bool diff_gt_180 = diff > 180;
    if(diff_gt_180) {
        diff = 360 - diff;
    }

    // if diff is less than angle return 0
    if(diff < angle) {
        FURI_LOG_T(TAG, "diff: %f, angle: %f", (double)diff, (double)angle);
        return 0;
    }

    // return diff with sign
    return (max_is_current ^ diff_gt_180) ? -diff : diff;
}

static void js_vgm_delta_yaw(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsVgmInst* vgm = mjs_get_ptr(mjs, obj_inst);
    furi_assert(vgm);
    size_t num_args = mjs_nargs(mjs);
    if(num_args < 1 || num_args > 2) {
        mjs_prepend_errorf(
            mjs,
            MJS_BAD_ARGS_ERROR,
            "Invalid args. Pass (angle [, timeout]). Got %d args.",
            num_args);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    if(!vgm->present) {
        FURI_LOG_T(TAG, "VGM not present.");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    double angle = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if(isnan(angle)) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "Invalid arg (angle).");
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    uint32_t ms = (num_args == 2) ? mjs_get_int(mjs, mjs_arg(mjs, 1)) : 3000;
    uint32_t timeout = furi_get_tick() + ms;
    float initial_yaw = imu_yaw_get(vgm->imu);
    do {
        float current_yaw = imu_yaw_get(vgm->imu);
        double diff = distance(current_yaw, initial_yaw, angle);
        if(diff != 0) {
            mjs_return(mjs, mjs_mk_number(mjs, diff));
            return;
        }
        furi_delay_ms(100);
    } while(furi_get_tick() < timeout);

    mjs_return(mjs, mjs_mk_number(mjs, 0));
}

static void* js_vgm_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    JsVgmInst* vgm = malloc(sizeof(JsVgmInst));
    vgm->imu = imu_alloc();
    vgm->present = imu_present(vgm->imu);
    if(!vgm->present) FURI_LOG_W(TAG, "VGM not present.");
    mjs_val_t vgm_obj = mjs_mk_object(mjs);
    mjs_set(mjs, vgm_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, vgm));
    mjs_set(mjs, vgm_obj, "getPitch", ~0, MJS_MK_FN(js_vgm_get_pitch));
    mjs_set(mjs, vgm_obj, "getRoll", ~0, MJS_MK_FN(js_vgm_get_roll));
    mjs_set(mjs, vgm_obj, "getYaw", ~0, MJS_MK_FN(js_vgm_get_yaw));
    mjs_set(mjs, vgm_obj, "deltaYaw", ~0, MJS_MK_FN(js_vgm_delta_yaw));
    *object = vgm_obj;
    return vgm;
}

static void js_vgm_destroy(void* inst) {
    JsVgmInst* vgm = inst;
    furi_assert(vgm);
    imu_free(vgm->imu);
    vgm->imu = NULL;
    free(vgm);
}

static const JsModuleDescriptor js_vgm_desc = {
    "vgm",
    js_vgm_create,
    js_vgm_destroy,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_vgm_desc,
};

const FlipperAppPluginDescriptor* js_vgm_ep(void) {
    return &plugin_descriptor;
}
