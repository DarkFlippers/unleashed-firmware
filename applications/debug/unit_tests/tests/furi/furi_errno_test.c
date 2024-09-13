#include <furi.h>
#include <errno.h>
#include "../test.h" // IWYU pragma: keep

#define TAG        "ErrnoTest"
#define THREAD_CNT 16
#define ITER_CNT   1000

static int32_t errno_fuzzer(void* context) {
    int start_value = (int)context;
    int32_t fails = 0;

    for(int i = start_value; i < start_value + ITER_CNT; i++) {
        errno = i;
        furi_thread_yield();
        if(errno != i) fails++;
    }

    for(int i = 0; i < ITER_CNT; i++) {
        errno = 0;
        furi_thread_yield();
        UNUSED(strtol("123456", NULL, 10)); // -V530
        furi_thread_yield();
        if(errno != 0) fails++;

        errno = 0;
        furi_thread_yield();
        UNUSED(strtol("123456123456123456123456123456123456123456123456", NULL, 10)); // -V530
        furi_thread_yield();
        if(errno != ERANGE) fails++;
    }

    return fails;
}

void test_errno_saving(void) {
    FuriThread* threads[THREAD_CNT];

    for(int i = 0; i < THREAD_CNT; i++) {
        int start_value = i * ITER_CNT;
        threads[i] = furi_thread_alloc_ex("ErrnoFuzzer", 1024, errno_fuzzer, (void*)start_value);
        furi_thread_set_priority(threads[i], FuriThreadPriorityNormal);
        furi_thread_start(threads[i]);
    }

    for(int i = 0; i < THREAD_CNT; i++) {
        furi_thread_join(threads[i]);
        mu_assert_int_eq(0, furi_thread_get_return_code(threads[i]));
        furi_thread_free(threads[i]);
    }
}
