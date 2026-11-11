/**
 * Tests for the NFC app's supported-card parsers (issue #1095). Bambu only so far, as the first
 * parser under this harness, plus a load check that covers all of them.
 *
 * The parsers live in .fal plugins of the NFC app, so the firmware cannot link them - but it does
 * not have to. They import only symbols the firmware already exports, so `firmware_api_interface`
 * alone resolves one; the NFC app's private table carries scene UI, which no parser touches, so no
 * CompositeApiResolver is needed here. Note that fbt only *warns* about an unresolved import in a
 * plugin - `do_strict_import_checks` is false for FlipperAppType.PLUGIN - so one ships silently,
 * which is what nfc_parsers_test_all_parsers_load exists to catch.
 *
 * A parser's parse() takes a loaded NfcDevice and writes a string: no radio, no card. verify() and
 * read() are the hardware half and stay out of scope.
 *
 * On fixtures: hand-building one is only safe because every Bambu validation rule is externally
 * documented - a magic "GF" prefix, a filament-type string, an ASCII test and a diameter range. A
 * parser gated on a computed check digit, CRC or MAC must NOT get a hand-built fixture: it would
 * be built with the same algorithm the validator uses, so the test would pass while the validator
 * still rejected real cards. That is how #1091 stayed hidden for four releases. Those parsers need
 * dumps of real cards, which brings the privacy problem #1095 describes.
 */
#include <furi.h>
#include <storage/storage.h>

#include <flipper_application/plugins/plugin_manager.h>
#include <nfc/helpers/nfc_data_generator.h>
#include <nfc/nfc_device.h>

#include <applications/main/nfc/plugins/supported_cards/nfc_supported_card_plugin.h>

#include "../test.h" // IWYU pragma: keep

// Must agree with NFC_SUPPORTED_CARDS_PLUGINS_PATH in the NFC app: APP_DATA_PATH() resolves
// against the calling thread's appid, so from here it would name the unit_tests directory
#define NFC_PARSERS_PLUGIN_DIR  EXT_PATH("apps_data/nfc/plugins")
#define NFC_PARSERS_FIXTURE_DIR EXT_PATH("unit_tests/nfc")

#define NFC_PARSERS_BAMBU_PLUGIN NFC_PARSERS_PLUGIN_DIR "/bambu_parser.fal"

#define NFC_PARSERS_BAMBU_FIXTURE      NFC_PARSERS_FIXTURE_DIR "/Bambu_PLA_Basic.nfc"
#define NFC_PARSERS_BAMBU_UNNORMALIZED NFC_PARSERS_FIXTURE_DIR "/Bambu_PLA_Basic_unnormalized.nfc"
#define NFC_PARSERS_BAMBU_WRONG_TYPE   NFC_PARSERS_FIXTURE_DIR "/Bambu_unknown_filament_type.nfc"

// What bambu_parse() renders from Bambu_PLA_Basic.nfc, in full, so a change to any one field fails
// here rather than drifting silently. The colour channels all differ, which pins the byte order.
#define NFC_PARSERS_BAMBU_EXPECTED \
    "\e#Bambu Lab Filament\n"      \
    "Type: PLA Basic\n"            \
    "Color: Black (#112233)\n"     \
    "Filament Code: 10101\n"       \
    "Prod: 2026-01-15 09:30\n"     \
    "\n\e#Configurations\n"        \
    "Hotend: 190-240 C\n"          \
    "Drying: 55 C for 8h\n"        \
    "Nozzle: >= 0.20mm\n"          \
    "\n\e#Specifications\n"        \
    "Weight: 1000g\n"              \
    "Diameter: 1.75mm\n"           \
    "Spool Width: 66.00mm\n"       \
    "Length: 330m\n"

typedef struct {
    Storage* storage;
    PluginManager* manager;
    NfcDevice* device;
    FuriString* parsed_data;
} NfcParsersTest;

static NfcParsersTest* test;

static void nfc_parsers_test_alloc(void) {
    test = malloc(sizeof(NfcParsersTest));
    test->storage = furi_record_open(RECORD_STORAGE);
    test->manager = NULL;
    test->device = nfc_device_alloc();
    // Owned by the suite, not by a test: mu_assert returns early, so a test cannot free its own
    test->parsed_data = furi_string_alloc();
}

static void nfc_parsers_test_free(void) {
    furi_check(test);
    if(test->manager) plugin_manager_free(test->manager);
    furi_string_free(test->parsed_data);
    nfc_device_free(test->device);
    furi_record_close(RECORD_STORAGE);
    free(test);
    test = NULL;
}

/**
 * Load one parser plugin by path, or NULL. Asserts nothing, so a caller's own assertion reports
 * the failure instead of overwriting it.
 *
 * The returned pointer lives in the loaded plugin's mapped pages, so it dangles after the next
 * call and after nfc_parsers_test_free(). Only one plugin is held at a time: the 45 parsers do not
 * all fit in RAM together.
 */
static const NfcSupportedCardsPlugin* nfc_parsers_test_load(const char* path) {
    if(test->manager) plugin_manager_free(test->manager);
    test->manager = plugin_manager_alloc(
        NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
        NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
        // NULL means firmware_api_interface, which is the whole point: no app-private symbols
        NULL);

    // Checks the appid and the API version too. An unresolved import surfaces here, as a loader
    // error.
    PluginManagerError error = plugin_manager_load_single(test->manager, path);
    if(error != PluginManagerErrorNone) {
        // printf: FURI_LOG goes to the log channel, not to this command's output
        printf("%s: load failed, error %d\r\n", path, error);
        return NULL;
    }

    return plugin_manager_get_ep(test->manager, 0);
}

MU_TEST(nfc_parsers_test_bambu_pla_basic) {
    mu_assert(
        nfc_device_load(test->device, NFC_PARSERS_BAMBU_FIXTURE),
        "could not load " NFC_PARSERS_BAMBU_FIXTURE "\r\n");

    const NfcSupportedCardsPlugin* plugin = nfc_parsers_test_load(NFC_PARSERS_BAMBU_PLUGIN);
    mu_assert(plugin && plugin->parse, "could not load " NFC_PARSERS_BAMBU_PLUGIN "\r\n");
    mu_assert_int_eq(plugin->protocol, nfc_device_get_protocol(test->device));

    furi_string_reset(test->parsed_data);
    mu_assert(
        plugin->parse(test->device, test->parsed_data), "bambu parse() declined its own card\r\n");
    mu_assert_string_eq(NFC_PARSERS_BAMBU_EXPECTED, furi_string_get_cstr(test->parsed_data));
}

/**
 * A real tag spells the variant id with a trailing zero the lookup table does not carry, so
 * bambu_normalize_variant_id() has to strip it. Getting this wrong renders "Unknown filament" for
 * genuine spools - quiet, cosmetic-looking, and the same shape as #1091.
 */
MU_TEST(nfc_parsers_test_bambu_unnormalized_variant) {
    mu_assert(
        nfc_device_load(test->device, NFC_PARSERS_BAMBU_UNNORMALIZED),
        "could not load " NFC_PARSERS_BAMBU_UNNORMALIZED "\r\n");

    const NfcSupportedCardsPlugin* plugin = nfc_parsers_test_load(NFC_PARSERS_BAMBU_PLUGIN);
    mu_assert(plugin && plugin->parse, "could not load " NFC_PARSERS_BAMBU_PLUGIN "\r\n");

    furi_string_reset(test->parsed_data);
    mu_assert(
        plugin->parse(test->device, test->parsed_data),
        "bambu parse() declined a card whose variant id only needed normalizing\r\n");
    // Identical to the canonical card: the trailing zero must make no difference
    mu_assert_string_eq(NFC_PARSERS_BAMBU_EXPECTED, furi_string_get_cstr(test->parsed_data));
}

/**
 * Every parser of a protocol runs against every card of that protocol, and the first one to claim
 * a card wins, so a false positive hides the right parser's output.
 *
 * This card clears the "GF" prefix, the printable-ASCII check and the diameter range, and fails
 * only on the filament type. A card that failed the first check instead would leave this test
 * passing with every later rule deleted.
 */
MU_TEST(nfc_parsers_test_bambu_declines_near_miss) {
    mu_assert(
        nfc_device_load(test->device, NFC_PARSERS_BAMBU_WRONG_TYPE),
        "could not load " NFC_PARSERS_BAMBU_WRONG_TYPE "\r\n");

    const NfcSupportedCardsPlugin* plugin = nfc_parsers_test_load(NFC_PARSERS_BAMBU_PLUGIN);
    mu_assert(plugin && plugin->parse, "could not load " NFC_PARSERS_BAMBU_PLUGIN "\r\n");

    furi_string_reset(test->parsed_data);
    mu_assert(
        !plugin->parse(test->device, test->parsed_data),
        "bambu parse() claimed a card whose filament type it does not know\r\n");
}

/** A factory-blank card of the right protocol carries nothing any parser should claim. */
MU_TEST(nfc_parsers_test_bambu_declines_blank_card) {
    nfc_data_generator_fill_data(NfcDataGeneratorTypeMfClassic1k_4b, test->device);

    const NfcSupportedCardsPlugin* plugin = nfc_parsers_test_load(NFC_PARSERS_BAMBU_PLUGIN);
    mu_assert(plugin && plugin->parse, "could not load " NFC_PARSERS_BAMBU_PLUGIN "\r\n");

    furi_string_reset(test->parsed_data);
    mu_assert(
        !plugin->parse(test->device, test->parsed_data),
        "bambu parse() claimed a factory-blank card\r\n");
}

/**
 * Every parser must load and expose a usable interface. This is the check the build does not do:
 * an unresolved import in a plugin is only a warning at build time, and in the app a parser that
 * fails to load is skipped in silence (nfc_supported_cards.c:287), so a parser can stop working
 * without anything saying so. One plugin is held at a time, since all 45 together do not fit.
 */
MU_TEST(nfc_parsers_test_all_parsers_load) {
    File* dir = storage_file_alloc(test->storage);
    mu_assert(
        storage_dir_open(dir, NFC_PARSERS_PLUGIN_DIR),
        "could not open " NFC_PARSERS_PLUGIN_DIR "\r\n");

    FuriString* path = furi_string_alloc();
    char name[256];
    size_t loaded = 0;
    size_t failed = 0;

    while(storage_dir_read(dir, NULL, name, sizeof(name))) {
        if(!strstr(name, NFC_SUPPORTED_CARDS_PLUGIN_SUFFIX)) continue;

        furi_string_printf(path, "%s/%s", NFC_PARSERS_PLUGIN_DIR, name);
        const NfcSupportedCardsPlugin* plugin = nfc_parsers_test_load(furi_string_get_cstr(path));

        if(plugin && plugin->parse && plugin->protocol < NfcProtocolNum) {
            loaded++;
        } else {
            // Named here, not just counted: keep going so one bad plugin does not hide the rest
            printf("%s is not a usable parser\r\n", name);
            failed++;
        }
    }

    furi_string_free(path);
    storage_dir_close(dir);
    storage_file_free(dir);

    mu_assert_int_eq(0, failed);
    mu_assert(loaded > 0, "found no parser plugins at all - are the resources deployed?\r\n");
}

MU_TEST_SUITE(nfc_parsers) {
    nfc_parsers_test_alloc();

    MU_RUN_TEST(nfc_parsers_test_bambu_pla_basic);
    MU_RUN_TEST(nfc_parsers_test_bambu_unnormalized_variant);
    MU_RUN_TEST(nfc_parsers_test_bambu_declines_near_miss);
    MU_RUN_TEST(nfc_parsers_test_bambu_declines_blank_card);
    MU_RUN_TEST(nfc_parsers_test_all_parsers_load);

    nfc_parsers_test_free();
}

int run_minunit_test_nfc_parsers(void) {
    MU_RUN_SUITE(nfc_parsers);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_nfc_parsers)
