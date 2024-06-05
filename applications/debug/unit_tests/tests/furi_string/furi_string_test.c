#include <furi.h>
#include "../test.h" // IWYU pragma: keep

static void test_setup(void) {
}

static void test_teardown(void) {
}

static FuriString* furi_string_alloc_vprintf_test(const char format[], ...) {
    va_list args;
    va_start(args, format);
    FuriString* string = furi_string_alloc_vprintf(format, args);
    va_end(args);
    return string;
}

MU_TEST(mu_test_furi_string_alloc_free) {
    FuriString* tmp;
    FuriString* string;

    // test alloc and free
    string = furi_string_alloc();
    mu_check(string != NULL);
    mu_check(furi_string_empty(string));
    furi_string_free(string);

    // test furi_string_alloc_set_str and free
    string = furi_string_alloc_set_str("test");
    mu_check(string != NULL);
    mu_check(!furi_string_empty(string));
    mu_check(furi_string_cmp(string, "test") == 0);
    furi_string_free(string);

    // test furi_string_alloc_set and free
    tmp = furi_string_alloc_set("more");
    string = furi_string_alloc_set(tmp);
    furi_string_free(tmp);
    mu_check(string != NULL);
    mu_check(!furi_string_empty(string));
    mu_check(furi_string_cmp(string, "more") == 0);
    furi_string_free(string);

    // test alloc_printf and free
    string = furi_string_alloc_printf("test %d %s %c 0x%02x", 1, "two", '3', 0x04);
    mu_check(string != NULL);
    mu_check(!furi_string_empty(string));
    mu_check(furi_string_cmp(string, "test 1 two 3 0x04") == 0);
    furi_string_free(string);

    // test alloc_vprintf and free
    string = furi_string_alloc_vprintf_test("test %d %s %c 0x%02x", 4, "five", '6', 0x07);
    mu_check(string != NULL);
    mu_check(!furi_string_empty(string));
    mu_check(furi_string_cmp(string, "test 4 five 6 0x07") == 0);
    furi_string_free(string);

    // test alloc_move and free
    tmp = furi_string_alloc_set("move");
    string = furi_string_alloc_move(tmp);
    mu_check(string != NULL);
    mu_check(!furi_string_empty(string));
    mu_check(furi_string_cmp(string, "move") == 0);
    furi_string_free(string);
}

MU_TEST(mu_test_furi_string_mem) {
    FuriString* string = furi_string_alloc_set("test");
    mu_check(string != NULL);
    mu_check(!furi_string_empty(string));

    // TODO FL-3493: how to test furi_string_reserve?

    // test furi_string_reset
    furi_string_reset(string);
    mu_check(furi_string_empty(string));

    // test furi_string_swap
    furi_string_set(string, "test");
    FuriString* swap_string = furi_string_alloc_set("swap");
    furi_string_swap(string, swap_string);
    mu_check(furi_string_cmp(string, "swap") == 0);
    mu_check(furi_string_cmp(swap_string, "test") == 0);
    furi_string_free(swap_string);

    // test furi_string_move
    FuriString* move_string = furi_string_alloc_set("move");
    furi_string_move(string, move_string);
    mu_check(furi_string_cmp(string, "move") == 0);
    // move_string is now empty
    // and tested by leaked memory check at the end of the tests

    furi_string_set(string, "abracadabra");

    // test furi_string_hash
    mu_assert_int_eq(0xc3bc16d7, furi_string_hash(string));

    // test furi_string_size
    mu_assert_int_eq(11, furi_string_size(string));

    // test furi_string_empty
    mu_check(!furi_string_empty(string));
    furi_string_reset(string);
    mu_check(furi_string_empty(string));

    furi_string_free(string);
}

MU_TEST(mu_test_furi_string_getters) {
    FuriString* string = furi_string_alloc_set("test");

    // test furi_string_get_char
    mu_check(furi_string_get_char(string, 0) == 't');
    mu_check(furi_string_get_char(string, 1) == 'e');
    mu_check(furi_string_get_char(string, 2) == 's');
    mu_check(furi_string_get_char(string, 3) == 't');

    // test furi_string_get_cstr
    mu_assert_string_eq("test", furi_string_get_cstr(string));
    furi_string_free(string);
}

static FuriString* furi_string_vprintf_test(FuriString* string, const char format[], ...) {
    va_list args;
    va_start(args, format);
    furi_string_vprintf(string, format, args);
    va_end(args);
    return string;
}

MU_TEST(mu_test_furi_string_setters) {
    FuriString* tmp;
    FuriString* string = furi_string_alloc();

    // test furi_string_set_str
    furi_string_set_str(string, "test");
    mu_assert_string_eq("test", furi_string_get_cstr(string));

    // test furi_string_set
    tmp = furi_string_alloc_set("more");
    furi_string_set(string, tmp);
    furi_string_free(tmp);
    mu_assert_string_eq("more", furi_string_get_cstr(string));

    // test furi_string_set_strn
    furi_string_set_strn(string, "test", 2);
    mu_assert_string_eq("te", furi_string_get_cstr(string));

    // test furi_string_set_char
    furi_string_set_char(string, 0, 'a');
    furi_string_set_char(string, 1, 'b');
    mu_assert_string_eq("ab", furi_string_get_cstr(string));

    // test furi_string_set_n
    tmp = furi_string_alloc_set("dodecahedron");
    furi_string_set_n(string, tmp, 4, 5);
    furi_string_free(tmp);
    mu_assert_string_eq("cahed", furi_string_get_cstr(string));

    // test furi_string_printf
    furi_string_printf(string, "test %d %s %c 0x%02x", 1, "two", '3', 0x04);
    mu_assert_string_eq("test 1 two 3 0x04", furi_string_get_cstr(string));

    // test furi_string_vprintf
    furi_string_vprintf_test(string, "test %d %s %c 0x%02x", 4, "five", '6', 0x07);
    mu_assert_string_eq("test 4 five 6 0x07", furi_string_get_cstr(string));

    furi_string_free(string);
}

static FuriString* furi_string_cat_vprintf_test(FuriString* string, const char format[], ...) {
    va_list args;
    va_start(args, format);
    furi_string_cat_vprintf(string, format, args);
    va_end(args);
    return string;
}

MU_TEST(mu_test_furi_string_appends) {
    FuriString* tmp;
    FuriString* string = furi_string_alloc();

    // test furi_string_push_back
    furi_string_push_back(string, 't');
    furi_string_push_back(string, 'e');
    furi_string_push_back(string, 's');
    furi_string_push_back(string, 't');
    mu_assert_string_eq("test", furi_string_get_cstr(string));
    furi_string_push_back(string, '!');
    mu_assert_string_eq("test!", furi_string_get_cstr(string));

    // test furi_string_cat_str
    furi_string_cat_str(string, "test");
    mu_assert_string_eq("test!test", furi_string_get_cstr(string));

    // test furi_string_cat
    tmp = furi_string_alloc_set("more");
    furi_string_cat(string, tmp);
    furi_string_free(tmp);
    mu_assert_string_eq("test!testmore", furi_string_get_cstr(string));

    // test furi_string_cat_printf
    furi_string_cat_printf(string, "test %d %s %c 0x%02x", 1, "two", '3', 0x04);
    mu_assert_string_eq("test!testmoretest 1 two 3 0x04", furi_string_get_cstr(string));

    // test furi_string_cat_vprintf
    furi_string_cat_vprintf_test(string, "test %d %s %c 0x%02x", 4, "five", '6', 0x07);
    mu_assert_string_eq(
        "test!testmoretest 1 two 3 0x04test 4 five 6 0x07", furi_string_get_cstr(string));

    furi_string_free(string);
}

MU_TEST(mu_test_furi_string_compare) {
    FuriString* string_1 = furi_string_alloc_set("string_1");
    FuriString* string_2 = furi_string_alloc_set("string_2");

    // test furi_string_cmp
    mu_assert_int_eq(0, furi_string_cmp(string_1, string_1));
    mu_assert_int_eq(0, furi_string_cmp(string_2, string_2));
    mu_assert_int_eq(-1, furi_string_cmp(string_1, string_2));
    mu_assert_int_eq(1, furi_string_cmp(string_2, string_1));

    // test furi_string_cmp_str
    mu_assert_int_eq(0, furi_string_cmp_str(string_1, "string_1"));
    mu_assert_int_eq(0, furi_string_cmp_str(string_2, "string_2"));
    mu_assert_int_eq(-1, furi_string_cmp_str(string_1, "string_2"));
    mu_assert_int_eq(1, furi_string_cmp_str(string_2, "string_1"));

    // test furi_string_cmpi
    furi_string_set(string_1, "string");
    furi_string_set(string_2, "StrIng");
    mu_assert_int_eq(0, furi_string_cmpi(string_1, string_1));
    mu_assert_int_eq(0, furi_string_cmpi(string_2, string_2));
    mu_assert_int_eq(0, furi_string_cmpi(string_1, string_2));
    mu_assert_int_eq(0, furi_string_cmpi(string_2, string_1));
    furi_string_set(string_1, "string_1");
    furi_string_set(string_2, "StrIng_2");
    mu_assert_int_eq(32, furi_string_cmp(string_1, string_2));
    mu_assert_int_eq(-32, furi_string_cmp(string_2, string_1));
    mu_assert_int_eq(-1, furi_string_cmpi(string_1, string_2));
    mu_assert_int_eq(1, furi_string_cmpi(string_2, string_1));

    // test furi_string_cmpi_str
    furi_string_set(string_1, "string");
    mu_assert_int_eq(0, furi_string_cmp_str(string_1, "string"));
    mu_assert_int_eq(32, furi_string_cmp_str(string_1, "String"));
    mu_assert_int_eq(32, furi_string_cmp_str(string_1, "STring"));
    mu_assert_int_eq(32, furi_string_cmp_str(string_1, "STRing"));
    mu_assert_int_eq(32, furi_string_cmp_str(string_1, "STRIng"));
    mu_assert_int_eq(32, furi_string_cmp_str(string_1, "STRINg"));
    mu_assert_int_eq(32, furi_string_cmp_str(string_1, "STRING"));
    mu_assert_int_eq(0, furi_string_cmpi_str(string_1, "string"));
    mu_assert_int_eq(0, furi_string_cmpi_str(string_1, "String"));
    mu_assert_int_eq(0, furi_string_cmpi_str(string_1, "STring"));
    mu_assert_int_eq(0, furi_string_cmpi_str(string_1, "STRing"));
    mu_assert_int_eq(0, furi_string_cmpi_str(string_1, "STRIng"));
    mu_assert_int_eq(0, furi_string_cmpi_str(string_1, "STRINg"));
    mu_assert_int_eq(0, furi_string_cmpi_str(string_1, "STRING"));

    furi_string_free(string_1);
    furi_string_free(string_2);
}

MU_TEST(mu_test_furi_string_search) {
    //                                            012345678901234567
    FuriString* haystack = furi_string_alloc_set("test321test123test");
    FuriString* needle = furi_string_alloc_set("test");

    // test furi_string_search
    mu_assert_int_eq(0, furi_string_search(haystack, needle));
    mu_assert_int_eq(7, furi_string_search(haystack, needle, 1));
    mu_assert_int_eq(14, furi_string_search(haystack, needle, 8));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_search(haystack, needle, 15));

    FuriString* tmp = furi_string_alloc_set("testnone");
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_search(haystack, tmp));
    furi_string_free(tmp);

    // test furi_string_search_str
    mu_assert_int_eq(0, furi_string_search_str(haystack, "test"));
    mu_assert_int_eq(7, furi_string_search_str(haystack, "test", 1));
    mu_assert_int_eq(14, furi_string_search_str(haystack, "test", 8));
    mu_assert_int_eq(4, furi_string_search_str(haystack, "321"));
    mu_assert_int_eq(11, furi_string_search_str(haystack, "123"));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_search_str(haystack, "testnone"));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_search_str(haystack, "test", 15));

    // test furi_string_search_char
    mu_assert_int_eq(0, furi_string_search_char(haystack, 't'));
    mu_assert_int_eq(1, furi_string_search_char(haystack, 'e'));
    mu_assert_int_eq(2, furi_string_search_char(haystack, 's'));
    mu_assert_int_eq(3, furi_string_search_char(haystack, 't', 1));
    mu_assert_int_eq(7, furi_string_search_char(haystack, 't', 4));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_search_char(haystack, 'x'));

    // test furi_string_search_rchar
    mu_assert_int_eq(17, furi_string_search_rchar(haystack, 't'));
    mu_assert_int_eq(15, furi_string_search_rchar(haystack, 'e'));
    mu_assert_int_eq(16, furi_string_search_rchar(haystack, 's'));
    mu_assert_int_eq(13, furi_string_search_rchar(haystack, '3'));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_search_rchar(haystack, '3', 14));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_search_rchar(haystack, 'x'));

    furi_string_free(haystack);
    furi_string_free(needle);
}

MU_TEST(mu_test_furi_string_equality) {
    FuriString* string = furi_string_alloc_set("test");
    FuriString* string_eq = furi_string_alloc_set("test");
    FuriString* string_neq = furi_string_alloc_set("test2");

    // test furi_string_equal
    mu_check(furi_string_equal(string, string_eq));
    mu_check(!furi_string_equal(string, string_neq));

    // test furi_string_equal_str
    mu_check(furi_string_equal_str(string, "test"));
    mu_check(!furi_string_equal_str(string, "test2"));
    mu_check(furi_string_equal_str(string_neq, "test2"));
    mu_check(!furi_string_equal_str(string_neq, "test"));

    furi_string_free(string);
    furi_string_free(string_eq);
    furi_string_free(string_neq);
}

MU_TEST(mu_test_furi_string_replace) {
    FuriString* needle = furi_string_alloc_set("test");
    FuriString* replace = furi_string_alloc_set("replace");
    FuriString* string = furi_string_alloc_set("test123test");

    // test furi_string_replace_at
    furi_string_replace_at(string, 4, 3, "!biglongword!");
    mu_assert_string_eq("test!biglongword!test", furi_string_get_cstr(string));

    // test furi_string_replace
    mu_assert_int_eq(17, furi_string_replace(string, needle, replace, 1));
    mu_assert_string_eq("test!biglongword!replace", furi_string_get_cstr(string));
    mu_assert_int_eq(0, furi_string_replace(string, needle, replace));
    mu_assert_string_eq("replace!biglongword!replace", furi_string_get_cstr(string));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_replace(string, needle, replace));
    mu_assert_string_eq("replace!biglongword!replace", furi_string_get_cstr(string));

    // test furi_string_replace_str
    mu_assert_int_eq(20, furi_string_replace_str(string, "replace", "test", 1));
    mu_assert_string_eq("replace!biglongword!test", furi_string_get_cstr(string));
    mu_assert_int_eq(0, furi_string_replace_str(string, "replace", "test"));
    mu_assert_string_eq("test!biglongword!test", furi_string_get_cstr(string));
    mu_assert_int_eq(FURI_STRING_FAILURE, furi_string_replace_str(string, "replace", "test"));
    mu_assert_string_eq("test!biglongword!test", furi_string_get_cstr(string));

    // test furi_string_replace_all
    furi_string_replace_all(string, needle, replace);
    mu_assert_string_eq("replace!biglongword!replace", furi_string_get_cstr(string));

    // test furi_string_replace_all_str
    furi_string_replace_all_str(string, "replace", "test");
    mu_assert_string_eq("test!biglongword!test", furi_string_get_cstr(string));

    furi_string_free(string);
    furi_string_free(needle);
    furi_string_free(replace);
}

MU_TEST(mu_test_furi_string_start_end) {
    FuriString* string = furi_string_alloc_set("start_end");
    FuriString* start = furi_string_alloc_set("start");
    FuriString* end = furi_string_alloc_set("end");

    // test furi_string_start_with
    mu_check(furi_string_start_with(string, start));
    mu_check(!furi_string_start_with(string, end));

    // test furi_string_start_with_str
    mu_check(furi_string_start_with_str(string, "start"));
    mu_check(!furi_string_start_with_str(string, "end"));

    // test furi_string_end_with
    mu_check(furi_string_end_with(string, end));
    mu_check(!furi_string_end_with(string, start));

    // test furi_string_end_with_str
    mu_check(furi_string_end_with_str(string, "end"));
    mu_check(!furi_string_end_with_str(string, "start"));

    furi_string_free(string);
    furi_string_free(start);
    furi_string_free(end);
}

MU_TEST(mu_test_furi_string_trim) {
    FuriString* string = furi_string_alloc_set("biglongstring");

    // test furi_string_left
    furi_string_left(string, 7);
    mu_assert_string_eq("biglong", furi_string_get_cstr(string));

    // test furi_string_right
    furi_string_right(string, 3);
    mu_assert_string_eq("long", furi_string_get_cstr(string));

    // test furi_string_mid
    furi_string_mid(string, 1, 2);
    mu_assert_string_eq("on", furi_string_get_cstr(string));

    // test furi_string_trim
    furi_string_set(string, "   \n\r\tbiglongstring \n\r\t  ");
    furi_string_trim(string);
    mu_assert_string_eq("biglongstring", furi_string_get_cstr(string));
    furi_string_set(string, "aaaabaaaabbaaabaaaabbtestaaaaaabbaaabaababaa");
    furi_string_trim(string, "ab");
    mu_assert_string_eq("test", furi_string_get_cstr(string));

    furi_string_free(string);
}

MU_TEST(mu_test_furi_string_utf8) {
    FuriString* utf8_string = furi_string_alloc_set("イルカ");

    // test furi_string_utf8_length
    mu_assert_int_eq(9, furi_string_size(utf8_string));
    mu_assert_int_eq(3, furi_string_utf8_length(utf8_string));

    // test furi_string_utf8_decode
    const uint8_t dolphin_emoji_array[4] = {0xF0, 0x9F, 0x90, 0xAC};
    FuriStringUTF8State state = FuriStringUTF8StateStarting;
    FuriStringUnicodeValue value = 0;
    furi_string_utf8_decode(dolphin_emoji_array[0], &state, &value);
    mu_assert_int_eq(FuriStringUTF8StateDecoding3, state);
    furi_string_utf8_decode(dolphin_emoji_array[1], &state, &value);
    mu_assert_int_eq(FuriStringUTF8StateDecoding2, state);
    furi_string_utf8_decode(dolphin_emoji_array[2], &state, &value);
    mu_assert_int_eq(FuriStringUTF8StateDecoding1, state);
    furi_string_utf8_decode(dolphin_emoji_array[3], &state, &value);
    mu_assert_int_eq(FuriStringUTF8StateStarting, state);
    mu_assert_int_eq(0x1F42C, value);

    // test furi_string_utf8_push
    furi_string_set(utf8_string, "");
    furi_string_utf8_push(utf8_string, value);
    mu_assert_string_eq("🐬", furi_string_get_cstr(utf8_string));

    furi_string_free(utf8_string);
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(mu_test_furi_string_alloc_free);
    MU_RUN_TEST(mu_test_furi_string_mem);
    MU_RUN_TEST(mu_test_furi_string_getters);
    MU_RUN_TEST(mu_test_furi_string_setters);
    MU_RUN_TEST(mu_test_furi_string_appends);
    MU_RUN_TEST(mu_test_furi_string_compare);
    MU_RUN_TEST(mu_test_furi_string_search);
    MU_RUN_TEST(mu_test_furi_string_equality);
    MU_RUN_TEST(mu_test_furi_string_replace);
    MU_RUN_TEST(mu_test_furi_string_start_end);
    MU_RUN_TEST(mu_test_furi_string_trim);
    MU_RUN_TEST(mu_test_furi_string_utf8);
}

int run_minunit_test_furi_string(void) {
    MU_RUN_SUITE(test_suite);

    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_furi_string)
