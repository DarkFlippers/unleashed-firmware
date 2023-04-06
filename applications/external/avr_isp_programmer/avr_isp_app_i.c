#include "avr_isp_app_i.h"
#include <lib/toolbox/path.h>
#include <flipper_format/flipper_format_i.h>

#define TAG "AvrIsp"

bool avr_isp_load_from_file(AvrIspApp* app) {
    furi_assert(app);

    FuriString* file_path = furi_string_alloc();
    FuriString* file_name = furi_string_alloc();

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, AVR_ISP_APP_EXTENSION, &I_avr_app_icon_10x10);
    browser_options.base_path = STORAGE_APP_DATA_PATH_PREFIX;

    // Input events and views are managed by file_select
    bool res = dialog_file_browser_show(app->dialogs, file_path, app->file_path, &browser_options);

    if(res) {
        path_extract_dirname(furi_string_get_cstr(file_path), app->file_path);
        path_extract_filename(file_path, file_name, true);
        strncpy(app->file_name_tmp, furi_string_get_cstr(file_name), AVR_ISP_MAX_LEN_NAME);
    }

    furi_string_free(file_name);
    furi_string_free(file_path);

    return res;
}
