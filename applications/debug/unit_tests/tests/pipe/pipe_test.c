#include "../test.h" // IWYU pragma: keep

#include <furi.h>
#include <lib/toolbox/pipe.h>

#define PIPE_SIZE      128U
#define PIPE_TRG_LEVEL 1U

MU_TEST(pipe_test_trivial) {
    PipeSideBundle bundle = pipe_alloc(PIPE_SIZE, PIPE_TRG_LEVEL);
    PipeSide* alice = bundle.alices_side;
    PipeSide* bob = bundle.bobs_side;

    mu_assert_int_eq(PipeRoleAlice, pipe_role(alice));
    mu_assert_int_eq(PipeRoleBob, pipe_role(bob));
    mu_assert_int_eq(PipeStateOpen, pipe_state(alice));
    mu_assert_int_eq(PipeStateOpen, pipe_state(bob));

    mu_assert_int_eq(PIPE_SIZE, pipe_spaces_available(alice));
    mu_assert_int_eq(PIPE_SIZE, pipe_spaces_available(bob));
    mu_assert_int_eq(0, pipe_bytes_available(alice));
    mu_assert_int_eq(0, pipe_bytes_available(bob));

    for(uint8_t i = 0;; ++i) {
        mu_assert_int_eq(PIPE_SIZE - i, pipe_spaces_available(alice));
        mu_assert_int_eq(i, pipe_bytes_available(bob));

        if(pipe_spaces_available(alice) == 0) break;
        furi_check(pipe_send(alice, &i, sizeof(uint8_t)) == sizeof(uint8_t));

        mu_assert_int_eq(PIPE_SIZE - i, pipe_spaces_available(bob));
        mu_assert_int_eq(i, pipe_bytes_available(alice));

        furi_check(pipe_send(bob, &i, sizeof(uint8_t)) == sizeof(uint8_t));
    }

    pipe_free(alice);
    mu_assert_int_eq(PipeStateBroken, pipe_state(bob));

    for(uint8_t i = 0;; ++i) {
        mu_assert_int_eq(PIPE_SIZE - i, pipe_bytes_available(bob));

        if(pipe_bytes_available(bob) == 0) break;
        uint8_t value;
        furi_check(pipe_receive(bob, &value, sizeof(uint8_t)) == sizeof(uint8_t));

        mu_assert_int_eq(i, value);
    }

    pipe_free(bob);
}

typedef enum {
    TestFlagDataArrived = 1 << 0,
    TestFlagSpaceFreed = 1 << 1,
    TestFlagBecameBroken = 1 << 2,
} TestFlag;

typedef struct {
    TestFlag flag;
    FuriEventLoop* event_loop;
} AncillaryThreadContext;

static void on_data_arrived(PipeSide* pipe, void* context) {
    AncillaryThreadContext* ctx = context;
    ctx->flag |= TestFlagDataArrived;
    uint8_t input;
    size_t size = pipe_receive(pipe, &input, sizeof(input));
    pipe_send(pipe, &input, size);
}

static void on_space_freed(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    AncillaryThreadContext* ctx = context;
    ctx->flag |= TestFlagSpaceFreed;
}

static void on_became_broken(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    AncillaryThreadContext* ctx = context;
    ctx->flag |= TestFlagBecameBroken;
    furi_event_loop_stop(ctx->event_loop);
}

static int32_t ancillary_thread(void* context) {
    PipeSide* pipe = context;
    AncillaryThreadContext thread_ctx = {
        .flag = 0,
        .event_loop = furi_event_loop_alloc(),
    };

    pipe_attach_to_event_loop(pipe, thread_ctx.event_loop);
    pipe_set_callback_context(pipe, &thread_ctx);
    pipe_set_data_arrived_callback(pipe, on_data_arrived, 0);
    pipe_set_space_freed_callback(pipe, on_space_freed, FuriEventLoopEventFlagEdge);
    pipe_set_broken_callback(pipe, on_became_broken, 0);

    furi_event_loop_run(thread_ctx.event_loop);

    pipe_detach_from_event_loop(pipe);
    pipe_free(pipe);
    furi_event_loop_free(thread_ctx.event_loop);
    return thread_ctx.flag;
}

MU_TEST(pipe_test_event_loop) {
    PipeSideBundle bundle = pipe_alloc(PIPE_SIZE, PIPE_TRG_LEVEL);
    PipeSide* alice = bundle.alices_side;
    PipeSide* bob = bundle.bobs_side;

    FuriThread* thread = furi_thread_alloc_ex("PipeTestAnc", 2048, ancillary_thread, bob);
    furi_thread_start(thread);

    const char* message = "Hello!";
    pipe_send(alice, message, strlen(message));

    char buffer_1[16];
    size_t size = pipe_receive(alice, buffer_1, strlen(message));
    buffer_1[size] = 0;

    pipe_free(alice);
    furi_thread_join(thread);

    mu_assert_string_eq(message, buffer_1);
    mu_assert_int_eq(
        TestFlagDataArrived | TestFlagSpaceFreed | TestFlagBecameBroken,
        furi_thread_get_return_code(thread));

    furi_thread_free(thread);
}

MU_TEST_SUITE(test_pipe) {
    MU_RUN_TEST(pipe_test_trivial);
    MU_RUN_TEST(pipe_test_event_loop);
}

int run_minunit_test_pipe(void) {
    MU_RUN_SUITE(test_pipe);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_pipe)
