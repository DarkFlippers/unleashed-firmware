#include <dialogs/dialogs.h>

#include "../minunit.h"

MU_TEST(test_dialog_file_browser_set_basic_options_should_init_all_fields) {
    mu_assert(
        sizeof(DialogsFileBrowserOptions) == 28,
        "Changes to `DialogsFileBrowserOptions` should also be reflected in `dialog_file_browser_set_basic_options`");

    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, ".fap", NULL);
    // note: this assertions can safely be changed, their primary purpose is to remind the maintainer
    // to update `dialog_file_browser_set_basic_options` by including all structure fields in it
    mu_assert_string_eq(".fap", options.extension);
    mu_assert_null(options.base_path);
    mu_assert(options.skip_assets, "`skip_assets` should default to `true");
    mu_assert(options.hide_dot_files, "`hide_dot_files` should default to `true");
    mu_assert_null(options.icon);
    mu_assert(options.hide_ext, "`hide_ext` should default to `true");
    mu_assert_null(options.item_loader_callback);
    mu_assert_null(options.item_loader_context);
}

MU_TEST_SUITE(dialogs_file_browser_options) {
    MU_RUN_TEST(test_dialog_file_browser_set_basic_options_should_init_all_fields);
}

int run_minunit_test_dialogs_file_browser_options() {
    MU_RUN_SUITE(dialogs_file_browser_options);

    return MU_EXIT_CODE;
}
