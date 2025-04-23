#include <furi.h>
#include <errno.h>
#include <stdio.h>
#include "../test.h" // IWYU pragma: keep

#define TAG "StdioTest"

#define CONTEXT_MAGIC ((void*)0xDEADBEEF)

// stdin

static char mock_in[256];
static size_t mock_in_len, mock_in_pos;

static void set_mock_in(const char* str) {
    size_t len = strlen(str);
    strcpy(mock_in, str);
    mock_in_len = len;
    mock_in_pos = 0;
}

static size_t mock_in_cb(char* buffer, size_t size, FuriWait wait, void* context) {
    UNUSED(wait);
    furi_check(context == CONTEXT_MAGIC);
    size_t remaining = mock_in_len - mock_in_pos;
    size = MIN(remaining, size);
    memcpy(buffer, mock_in + mock_in_pos, size);
    mock_in_pos += size;
    return size;
}

void test_stdin(void) {
    FuriThreadStdinReadCallback in_cb;
    void* in_ctx;
    furi_thread_get_stdin_callback(&in_cb, &in_ctx);
    furi_thread_set_stdin_callback(mock_in_cb, CONTEXT_MAGIC);
    char buf[256];

    // plain in
    set_mock_in("Hello, World!\n");
    fgets(buf, sizeof(buf), stdin);
    mu_assert_string_eq("Hello, World!\n", buf);
    mu_assert_int_eq(EOF, getchar());

    // ungetc
    ungetc('i', stdin);
    ungetc('H', stdin);
    fgets(buf, sizeof(buf), stdin);
    mu_assert_string_eq("Hi", buf);
    mu_assert_int_eq(EOF, getchar());

    // ungetc + plain in
    set_mock_in(" World");
    ungetc('i', stdin);
    ungetc('H', stdin);
    fgets(buf, sizeof(buf), stdin);
    mu_assert_string_eq("Hi World", buf);
    mu_assert_int_eq(EOF, getchar());

    // partial plain in
    set_mock_in("Hello, World!\n");
    fgets(buf, strlen("Hello") + 1, stdin);
    mu_assert_string_eq("Hello", buf);
    mu_assert_int_eq(',', getchar());
    fgets(buf, sizeof(buf), stdin);
    mu_assert_string_eq(" World!\n", buf);

    furi_thread_set_stdin_callback(in_cb, in_ctx);
}

// stdout

static FuriString* mock_out;
static FuriThreadStdoutWriteCallback original_out_cb;
static void* original_out_ctx;

static void mock_out_cb(const char* data, size_t size, void* context) {
    furi_check(context == CONTEXT_MAGIC);
    // there's no furi_string_cat_strn :(
    for(size_t i = 0; i < size; i++) {
        furi_string_push_back(mock_out, data[i]);
    }
}

static void assert_and_clear_mock_out(const char* expected) {
    // return the original stdout callback for the duration of the check
    // if the check fails, we don't want the error to end up in our buffer,
    // we want to be able to see it!
    furi_thread_set_stdout_callback(original_out_cb, original_out_ctx);
    mu_assert_string_eq(expected, furi_string_get_cstr(mock_out));
    furi_thread_set_stdout_callback(mock_out_cb, CONTEXT_MAGIC);

    furi_string_reset(mock_out);
}

void test_stdout(void) {
    furi_thread_get_stdout_callback(&original_out_cb, &original_out_ctx);
    furi_thread_set_stdout_callback(mock_out_cb, CONTEXT_MAGIC);
    mock_out = furi_string_alloc();

    puts("Hello, World!");
    assert_and_clear_mock_out("Hello, World!\n");

    printf("He");
    printf("llo!");
    fflush(stdout);
    assert_and_clear_mock_out("Hello!");

    furi_string_free(mock_out);
    furi_thread_set_stdout_callback(original_out_cb, original_out_ctx);
}
