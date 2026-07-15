#include "../test.h" // IWYU pragma: keep
#include <float_tools.h>
#include <notification/notification_messages_notes.h>

void frequency_assert(const char* note_name, const NotificationMessage* message) {
    double a = notification_messages_notes_frequency_from_name(note_name);
    double b = message->data.sound.frequency;
    const double epsilon = message->data.sound.frequency > 5000 ? 0.02f : 0.01f;
    mu_assert_double_between(b - epsilon, b + epsilon, a);
}

MU_TEST(notification_messages_notes_frequency_from_name_test) {
    // Upper case
    mu_check(float_is_equal(
        notification_messages_notes_frequency_from_name("C0"),
        notification_messages_notes_frequency_from_name("c0")));

    // Mixed case
    mu_check(float_is_equal(
        notification_messages_notes_frequency_from_name("Cs0"),
        notification_messages_notes_frequency_from_name("cs0")));

    // Check errors
    mu_check(
        float_is_equal(notification_messages_notes_frequency_from_name("0"), 0.0)); // Without note
    mu_check(float_is_equal(
        notification_messages_notes_frequency_from_name("C"), 0.0)); // Without octave
    mu_check(float_is_equal(
        notification_messages_notes_frequency_from_name("C9"), 0.0)); // Unsupported octave
    mu_check(float_is_equal(
        notification_messages_notes_frequency_from_name("C10"), 0.0)); // Unsupported octave
    mu_check(float_is_equal(
        notification_messages_notes_frequency_from_name("X0"), 0.0)); // Unknown note
    mu_check(float_is_equal(
        notification_messages_notes_frequency_from_name("CCC0"), 0.0)); // Note name overflow

    // Notes and structures
    frequency_assert("c0", &message_note_c0);
    frequency_assert("cs0", &message_note_cs0);
    frequency_assert("d0", &message_note_d0);
    frequency_assert("ds0", &message_note_ds0);
    frequency_assert("e0", &message_note_e0);
    frequency_assert("f0", &message_note_f0);
    frequency_assert("fs0", &message_note_fs0);
    frequency_assert("g0", &message_note_g0);
    frequency_assert("gs0", &message_note_gs0);
    frequency_assert("a0", &message_note_a0);
    frequency_assert("as0", &message_note_as0);
    frequency_assert("b0", &message_note_b0);

    frequency_assert("c1", &message_note_c1);
    frequency_assert("cs1", &message_note_cs1);
    frequency_assert("d1", &message_note_d1);
    frequency_assert("ds1", &message_note_ds1);
    frequency_assert("e1", &message_note_e1);
    frequency_assert("f1", &message_note_f1);
    frequency_assert("fs1", &message_note_fs1);
    frequency_assert("g1", &message_note_g1);
    frequency_assert("gs1", &message_note_gs1);
    frequency_assert("a1", &message_note_a1);
    frequency_assert("as1", &message_note_as1);
    frequency_assert("b1", &message_note_b1);

    frequency_assert("c2", &message_note_c2);
    frequency_assert("cs2", &message_note_cs2);
    frequency_assert("d2", &message_note_d2);
    frequency_assert("ds2", &message_note_ds2);
    frequency_assert("e2", &message_note_e2);
    frequency_assert("f2", &message_note_f2);
    frequency_assert("fs2", &message_note_fs2);
    frequency_assert("g2", &message_note_g2);
    frequency_assert("gs2", &message_note_gs2);
    frequency_assert("a2", &message_note_a2);
    frequency_assert("as2", &message_note_as2);
    frequency_assert("b2", &message_note_b2);

    frequency_assert("c3", &message_note_c3);
    frequency_assert("cs3", &message_note_cs3);
    frequency_assert("d3", &message_note_d3);
    frequency_assert("ds3", &message_note_ds3);
    frequency_assert("e3", &message_note_e3);
    frequency_assert("f3", &message_note_f3);
    frequency_assert("fs3", &message_note_fs3);
    frequency_assert("g3", &message_note_g3);
    frequency_assert("gs3", &message_note_gs3);
    frequency_assert("a3", &message_note_a3);
    frequency_assert("as3", &message_note_as3);
    frequency_assert("b3", &message_note_b3);

    frequency_assert("c4", &message_note_c4);
    frequency_assert("cs4", &message_note_cs4);
    frequency_assert("d4", &message_note_d4);
    frequency_assert("ds4", &message_note_ds4);
    frequency_assert("e4", &message_note_e4);
    frequency_assert("f4", &message_note_f4);
    frequency_assert("fs4", &message_note_fs4);
    frequency_assert("g4", &message_note_g4);
    frequency_assert("gs4", &message_note_gs4);
    frequency_assert("a4", &message_note_a4);
    frequency_assert("as4", &message_note_as4);
    frequency_assert("b4", &message_note_b4);

    frequency_assert("c5", &message_note_c5);
    frequency_assert("cs5", &message_note_cs5);
    frequency_assert("d5", &message_note_d5);
    frequency_assert("ds5", &message_note_ds5);
    frequency_assert("e5", &message_note_e5);
    frequency_assert("f5", &message_note_f5);
    frequency_assert("fs5", &message_note_fs5);
    frequency_assert("g5", &message_note_g5);
    frequency_assert("gs5", &message_note_gs5);
    frequency_assert("a5", &message_note_a5);
    frequency_assert("as5", &message_note_as5);
    frequency_assert("b5", &message_note_b5);

    frequency_assert("c6", &message_note_c6);
    frequency_assert("cs6", &message_note_cs6);
    frequency_assert("d6", &message_note_d6);
    frequency_assert("ds6", &message_note_ds6);
    frequency_assert("e6", &message_note_e6);
    frequency_assert("f6", &message_note_f6);
    frequency_assert("fs6", &message_note_fs6);
    frequency_assert("g6", &message_note_g6);
    frequency_assert("gs6", &message_note_gs6);
    frequency_assert("a6", &message_note_a6);
    frequency_assert("as6", &message_note_as6);
    frequency_assert("b6", &message_note_b6);

    frequency_assert("c7", &message_note_c7);
    frequency_assert("cs7", &message_note_cs7);
    frequency_assert("d7", &message_note_d7);
    frequency_assert("ds7", &message_note_ds7);
    frequency_assert("e7", &message_note_e7);
    frequency_assert("f7", &message_note_f7);
    frequency_assert("fs7", &message_note_fs7);
    frequency_assert("g7", &message_note_g7);
    frequency_assert("gs7", &message_note_gs7);
    frequency_assert("a7", &message_note_a7);
    frequency_assert("as7", &message_note_as7);
    frequency_assert("b7", &message_note_b7);

    frequency_assert("c8", &message_note_c8);
    frequency_assert("cs8", &message_note_cs8);
    frequency_assert("d8", &message_note_d8);
    frequency_assert("ds8", &message_note_ds8);
    frequency_assert("e8", &message_note_e8);
    frequency_assert("f8", &message_note_f8);
    frequency_assert("fs8", &message_note_fs8);
    frequency_assert("g8", &message_note_g8);
    frequency_assert("gs8", &message_note_gs8);
    frequency_assert("a8", &message_note_a8);
    frequency_assert("as8", &message_note_as8);
    frequency_assert("b8", &message_note_b8);
}

MU_TEST_SUITE(notes_suite) {
    MU_RUN_TEST(notification_messages_notes_frequency_from_name_test);
}

int run_minunit_test_notes(void) {
    MU_RUN_SUITE(notes_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_notes)
