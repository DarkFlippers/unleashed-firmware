// Host harness: runs the real MU_TEST body for t5577_restore_password, extracted verbatim from
// applications/debug/unit_tests/tests/lfrfid/lfrfid_protocols.c by run.py, on every run.
#include <furi.h>
#include <lfrfid/tools/t5577_file.h>

static int failures = 0;

#define MU_TEST(name) static void name(void)

#define mu_check(x)                                                     \
    do {                                                                \
        if(!(x)) {                                                      \
            printf("FAIL %s:%d  mu_check(%s)\n", __FILE__, __LINE__, #x); \
            failures++;                                                 \
        }                                                               \
    } while(0)

#define mu_assert_int_eq(e, a)                                               \
    do {                                                                     \
        long long _e = (long long)(e), _a = (long long)(a);                  \
        if(_e != _a) {                                                       \
            printf(                                                          \
                "FAIL %s:%d  expected %lld (0x%llX) got %lld (0x%llX)\n",     \
                __FILE__, __LINE__, _e, (unsigned long long)_e, _a,          \
                (unsigned long long)_a);                                      \
            failures++;                                                      \
        }                                                                    \
    } while(0)

#include "mu_body.generated.inc"

int main(void) {
    test_lfrfid_t5577_restore_password();
    printf("%s (%d failures)\n", failures ? "FAILED" : "all passed", failures);
    return failures != 0;
}
