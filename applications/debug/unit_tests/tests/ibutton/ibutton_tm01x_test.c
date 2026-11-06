#include <furi.h>
#include <ibutton/ibutton_key.h>
#include <ibutton/ibutton_protocols.h>
#include <ibutton/protocols/blanks/tm01x.h>

#include "../test.h" // IWYU pragma: keep

MU_TEST(test_tm01x_bit_reverse32) {
    mu_check(tm01x_bit_reverse32(0x00000001) == 0x80000000);
    mu_check(tm01x_bit_reverse32(0x80000000) == 0x00000001);
    mu_check(tm01x_bit_reverse32(0x00000000) == 0x00000000);
    mu_check(tm01x_bit_reverse32(0xFFFFFFFF) == 0xFFFFFFFF);
    mu_check(tm01x_bit_reverse32(0x12345678) == 0x1E6A2C48);

    // Reversing twice is the identity
    mu_check(tm01x_bit_reverse32(tm01x_bit_reverse32(0xDEADBEEF)) == 0xDEADBEEF);
}

// Metakom frames carry even parity per byte; a key that fails it cannot be read back.
MU_TEST(test_metakom_parity_is_checked) {
    iButtonProtocols* protocols = ibutton_protocols_alloc();
    const iButtonProtocolId id = ibutton_protocols_get_id_by_name(protocols, "Metakom");
    mu_check(id != iButtonProtocolIdInvalid);

    iButtonKey* key = ibutton_key_alloc(ibutton_protocols_get_max_data_size(protocols));
    ibutton_key_set_protocol_id(key, id);

    iButtonEditableData editable;
    ibutton_protocols_get_editable_data(protocols, key, &editable);
    mu_assert_int_eq(sizeof(uint32_t), editable.size);

    // Every byte of 0xAAAAAAAA has four bits set; the low bit of the last one makes it five.
    const uint32_t even_parity = 0xAAAAAAAA;
    const uint32_t odd_parity = 0xAAAAAAAB;

    memcpy(editable.ptr, &even_parity, sizeof(even_parity));
    mu_check(ibutton_protocols_is_valid(protocols, key));

    memcpy(editable.ptr, &odd_parity, sizeof(odd_parity));
    mu_check(!ibutton_protocols_is_valid(protocols, key));

    FuriString* error = furi_string_alloc();
    ibutton_protocols_render_error(protocols, key, error);
    mu_check(furi_string_search_str(error, "parity", 0) != FURI_STRING_FAILURE);
    furi_string_free(error);

    // The write path refuses the key as well, without touching the bus.
    mu_check(!ibutton_protocols_write_id(protocols, key));

    ibutton_key_free(key);
    ibutton_protocols_free(protocols);
}

MU_TEST_SUITE(test_tm01x_suite) {
    MU_RUN_TEST(test_tm01x_bit_reverse32);
    MU_RUN_TEST(test_metakom_parity_is_checked);
}

int run_minunit_test_tm01x(void) {
    MU_RUN_SUITE(test_tm01x_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_tm01x)
