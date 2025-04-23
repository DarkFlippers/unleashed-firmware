#include "pipe.h"
#include <furi.h>

#define PIPE_DEFAULT_STATE_CHECK_PERIOD furi_ms_to_ticks(100)

/**
 * Data shared between both sides.
 */
typedef struct {
    FuriSemaphore* instance_count; // <! 1 = both sides, 0 = only one side
    FuriMutex* state_transition;
} PipeShared;

/**
 * There are two PipeSides per pipe.
 */
struct PipeSide {
    PipeRole role;
    PipeShared* shared;
    FuriStreamBuffer* sending;
    FuriStreamBuffer* receiving;

    FuriEventLoop* event_loop;
    void* callback_context;
    PipeSideDataArrivedCallback on_data_arrived;
    PipeSideSpaceFreedCallback on_space_freed;
    PipeSideBrokenCallback on_pipe_broken;
    FuriWait state_check_period;
};

PipeSideBundle pipe_alloc(size_t capacity, size_t trigger_level) {
    PipeSideReceiveSettings settings = {
        .capacity = capacity,
        .trigger_level = trigger_level,
    };
    return pipe_alloc_ex(settings, settings);
}

PipeSideBundle pipe_alloc_ex(PipeSideReceiveSettings alice, PipeSideReceiveSettings bob) {
    // the underlying primitives are shared
    FuriStreamBuffer* alice_to_bob = furi_stream_buffer_alloc(bob.capacity, bob.trigger_level);
    FuriStreamBuffer* bob_to_alice = furi_stream_buffer_alloc(alice.capacity, alice.trigger_level);

    PipeShared* shared = malloc(sizeof(PipeShared));
    *shared = (PipeShared){
        .instance_count = furi_semaphore_alloc(1, 1),
        .state_transition = furi_mutex_alloc(FuriMutexTypeNormal),
    };

    PipeSide* alices_side = malloc(sizeof(PipeSide));
    PipeSide* bobs_side = malloc(sizeof(PipeSide));

    *alices_side = (PipeSide){
        .role = PipeRoleAlice,
        .shared = shared,
        .sending = alice_to_bob,
        .receiving = bob_to_alice,
        .state_check_period = PIPE_DEFAULT_STATE_CHECK_PERIOD,
    };
    *bobs_side = (PipeSide){
        .role = PipeRoleBob,
        .shared = shared,
        .sending = bob_to_alice,
        .receiving = alice_to_bob,
        .state_check_period = PIPE_DEFAULT_STATE_CHECK_PERIOD,
    };

    return (PipeSideBundle){.alices_side = alices_side, .bobs_side = bobs_side};
}

PipeRole pipe_role(PipeSide* pipe) {
    furi_check(pipe);
    return pipe->role;
}

PipeState pipe_state(PipeSide* pipe) {
    furi_check(pipe);
    uint32_t count = furi_semaphore_get_count(pipe->shared->instance_count);
    return (count == 1) ? PipeStateOpen : PipeStateBroken;
}

void pipe_free(PipeSide* pipe) {
    furi_check(pipe);
    furi_check(!pipe->event_loop);

    furi_mutex_acquire(pipe->shared->state_transition, FuriWaitForever);
    FuriStatus status = furi_semaphore_acquire(pipe->shared->instance_count, 0);

    if(status == FuriStatusOk) {
        // the other side is still intact
        furi_mutex_release(pipe->shared->state_transition);
        free(pipe);
    } else {
        // the other side is gone too
        furi_stream_buffer_free(pipe->sending);
        furi_stream_buffer_free(pipe->receiving);
        furi_semaphore_free(pipe->shared->instance_count);
        furi_mutex_free(pipe->shared->state_transition);
        free(pipe->shared);
        free(pipe);
    }
}

static void pipe_stdout_cb(const char* data, size_t size, void* context) {
    furi_assert(context);
    PipeSide* pipe = context;
    pipe_send(pipe, data, size);
}

static size_t pipe_stdin_cb(char* data, size_t size, FuriWait timeout, void* context) {
    UNUSED(timeout);
    furi_assert(context);
    PipeSide* pipe = context;
    return pipe_receive(pipe, data, size);
}

void pipe_install_as_stdio(PipeSide* pipe) {
    furi_check(pipe);
    furi_thread_set_stdout_callback(pipe_stdout_cb, pipe);
    furi_thread_set_stdin_callback(pipe_stdin_cb, pipe);
}

void pipe_set_state_check_period(PipeSide* pipe, FuriWait check_period) {
    furi_check(pipe);
    pipe->state_check_period = check_period;
}

size_t pipe_receive(PipeSide* pipe, void* data, size_t length) {
    furi_check(pipe);

    size_t received = 0;
    while(length) {
        size_t received_this_time =
            furi_stream_buffer_receive(pipe->receiving, data, length, pipe->state_check_period);
        if(!received_this_time && pipe_state(pipe) == PipeStateBroken) break;

        received += received_this_time;
        length -= received_this_time;
        data += received_this_time;
    }

    return received;
}

size_t pipe_send(PipeSide* pipe, const void* data, size_t length) {
    furi_check(pipe);

    size_t sent = 0;
    while(length) {
        size_t sent_this_time =
            furi_stream_buffer_send(pipe->sending, data, length, pipe->state_check_period);
        if(!sent_this_time && pipe_state(pipe) == PipeStateBroken) break;

        sent += sent_this_time;
        length -= sent_this_time;
        data += sent_this_time;
    }

    return sent;
}

size_t pipe_bytes_available(PipeSide* pipe) {
    furi_check(pipe);
    return furi_stream_buffer_bytes_available(pipe->receiving);
}

size_t pipe_spaces_available(PipeSide* pipe) {
    furi_check(pipe);
    return furi_stream_buffer_spaces_available(pipe->sending);
}

static void pipe_receiving_buffer_callback(FuriEventLoopObject* buffer, void* context) {
    UNUSED(buffer);
    PipeSide* pipe = context;
    furi_assert(pipe);
    if(pipe->on_data_arrived) pipe->on_data_arrived(pipe, pipe->callback_context);
}

static void pipe_sending_buffer_callback(FuriEventLoopObject* buffer, void* context) {
    UNUSED(buffer);
    PipeSide* pipe = context;
    furi_assert(pipe);
    if(pipe->on_space_freed) pipe->on_space_freed(pipe, pipe->callback_context);
}

static void pipe_semaphore_callback(FuriEventLoopObject* semaphore, void* context) {
    UNUSED(semaphore);
    PipeSide* pipe = context;
    furi_assert(pipe);
    if(pipe->on_pipe_broken) pipe->on_pipe_broken(pipe, pipe->callback_context);
}

void pipe_attach_to_event_loop(PipeSide* pipe, FuriEventLoop* event_loop) {
    furi_check(pipe);
    furi_check(event_loop);
    furi_check(!pipe->event_loop);

    pipe->event_loop = event_loop;
}

void pipe_detach_from_event_loop(PipeSide* pipe) {
    furi_check(pipe);
    furi_check(pipe->event_loop);

    furi_event_loop_maybe_unsubscribe(pipe->event_loop, pipe->receiving);
    furi_event_loop_maybe_unsubscribe(pipe->event_loop, pipe->sending);
    furi_event_loop_maybe_unsubscribe(pipe->event_loop, pipe->shared->instance_count);

    pipe->event_loop = NULL;
}

void pipe_set_callback_context(PipeSide* pipe, void* context) {
    furi_check(pipe);
    pipe->callback_context = context;
}

void pipe_set_data_arrived_callback(
    PipeSide* pipe,
    PipeSideDataArrivedCallback callback,
    FuriEventLoopEvent event) {
    furi_check(pipe);
    furi_check(pipe->event_loop);
    furi_check((event & FuriEventLoopEventMask) == 0);

    furi_event_loop_maybe_unsubscribe(pipe->event_loop, pipe->receiving);
    pipe->on_data_arrived = callback;
    if(callback)
        furi_event_loop_subscribe_stream_buffer(
            pipe->event_loop,
            pipe->receiving,
            FuriEventLoopEventIn | event,
            pipe_receiving_buffer_callback,
            pipe);
}

void pipe_set_space_freed_callback(
    PipeSide* pipe,
    PipeSideSpaceFreedCallback callback,
    FuriEventLoopEvent event) {
    furi_check(pipe);
    furi_check(pipe->event_loop);
    furi_check((event & FuriEventLoopEventMask) == 0);

    furi_event_loop_maybe_unsubscribe(pipe->event_loop, pipe->sending);
    pipe->on_space_freed = callback;
    if(callback)
        furi_event_loop_subscribe_stream_buffer(
            pipe->event_loop,
            pipe->sending,
            FuriEventLoopEventOut | event,
            pipe_sending_buffer_callback,
            pipe);
}

void pipe_set_broken_callback(
    PipeSide* pipe,
    PipeSideBrokenCallback callback,
    FuriEventLoopEvent event) {
    furi_check(pipe);
    furi_check(pipe->event_loop);
    furi_check((event & FuriEventLoopEventMask) == 0);

    furi_event_loop_maybe_unsubscribe(pipe->event_loop, pipe->shared->instance_count);
    pipe->on_pipe_broken = callback;
    if(callback)
        furi_event_loop_subscribe_semaphore(
            pipe->event_loop,
            pipe->shared->instance_count,
            FuriEventLoopEventOut | event,
            pipe_semaphore_callback,
            pipe);
}
