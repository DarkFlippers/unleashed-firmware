/**
 * @file pipe.h
 * Pipe convenience module
 * 
 * Pipes are used to send bytes between two threads in both directions. The two
 * threads are referred to as Alice and Bob and their abilities regarding what
 * they can do with the pipe are equal.
 * 
 * It is also possible to use both sides of the pipe within one thread.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <stddef.h>

/**
 * @brief The role of a pipe side
 * 
 * Both roles are equal, as they can both read and write the data. This status
 * might be helpful in determining the role of a thread w.r.t. another thread in
 * an application that builds on the pipe.
 */
typedef enum {
    PipeRoleAlice,
    PipeRoleBob,
} PipeRole;

/**
 * @brief The state of a pipe
 * 
 *   - `PipeStateOpen`: Both pipe sides are in place, meaning data that is sent
 *     down the pipe _might_ be read by the peer, and new data sent by the peer
 *     _might_ arrive.
 *   - `PipeStateBroken`: The other side of the pipe has been freed, meaning
 *     data that is written will never reach its destination, and no new data
 *     will appear in the buffer.
 * 
 * A broken pipe can never become open again, because there's no way to connect
 * a side of a pipe to another side of a pipe.
 */
typedef enum {
    PipeStateOpen,
    PipeStateBroken,
} PipeState;

typedef struct PipeSide PipeSide;

typedef struct {
    PipeSide* alices_side;
    PipeSide* bobs_side;
} PipeSideBundle;

typedef struct {
    size_t capacity;
    size_t trigger_level;
} PipeSideReceiveSettings;

/**
 * @brief Allocates two connected sides of one pipe.
 * 
 * Creating a pair of sides using this function is the only way to connect two
 * pipe sides together. Two unrelated orphaned sides may never be connected back
 * together.
 * 
 * The capacity and trigger level for both directions are the same when the pipe
 * is created using this function. Use `pipe_alloc_ex` if you want more
 * control.
 * 
 * @param capacity Maximum number of bytes buffered in one direction
 * @param trigger_level Number of bytes that need to be available in the buffer
 *                      in order for a blocked thread to unblock
 * @returns Bundle with both sides of the pipe
 */
PipeSideBundle pipe_alloc(size_t capacity, size_t trigger_level);

/**
 * @brief Allocates two connected sides of one pipe.
 * 
 * Creating a pair of sides using this function is the only way to connect two
 * pipe sides together. Two unrelated orphaned sides may never be connected back
 * together.
 * 
 * The capacity and trigger level may be different for the two directions when
 * the pipe is created using this function. Use `pipe_alloc` if you don't
 * need control this fine.
 * 
 * @param alice `capacity` and `trigger_level` settings for Alice's receiving
 *              buffer
 * @param bob `capacity` and `trigger_level` settings for Bob's receiving buffer
 * @returns Bundle with both sides of the pipe
 */
PipeSideBundle pipe_alloc_ex(PipeSideReceiveSettings alice, PipeSideReceiveSettings bob);

/**
 * @brief Gets the role of a pipe side.
 * 
 * The roles (Alice and Bob) are equal, as both can send and receive data. This
 * status might be helpful in determining the role of a thread w.r.t. another
 * thread.
 * 
 * @param [in] pipe Pipe side to query
 * @returns Role of provided pipe side
 */
PipeRole pipe_role(PipeSide* pipe);

/**
 * @brief Gets the state of a pipe.
 * 
 * When the state is `PipeStateOpen`, both sides are active and may send or
 * receive data. When the state is `PipeStateBroken`, only one side is active
 * (the one that this method has been called on). If you find yourself in that
 * state, the data that you send will never be heard by anyone, and the data you
 * receive are leftovers in the buffer.
 * 
 * @param [in] pipe Pipe side to query
 * @returns State of the pipe
 */
PipeState pipe_state(PipeSide* pipe);

/**
 * @brief Frees a side of a pipe.
 * 
 * When only one of the sides is freed, the pipe is transitioned from the "Open"
 * state into the "Broken" state. When both sides are freed, the underlying data
 * structures are freed too.
 * 
 * @param [in] pipe Pipe side to free
 */
void pipe_free(PipeSide* pipe);

/**
 * @brief Connects the pipe to the `stdin` and `stdout` of the current thread.
 * 
 * After performing this operation, you can use `getc`, `puts`, etc. to send and
 * receive data to and from the pipe. If the pipe becomes broken, C stdlib calls
 * will return `EOF` wherever possible.
 * 
 * You can disconnect the pipe by manually calling
 * `furi_thread_set_stdout_callback` and `furi_thread_set_stdin_callback` with
 * `NULL`.
 * 
 * @param [in] pipe Pipe side to connect to the stdio
 */
void pipe_install_as_stdio(PipeSide* pipe);

/**
 * @brief Sets the state check period for `send` and `receive` operations
 * 
 * @note This value is set to 100 ms when the pipe is created
 * 
 * `send` and `receive` will check the state of the pipe if exactly 0 bytes were
 * sent or received during any given `check_period`. Read the documentation for
 * `pipe_send` and `pipe_receive` for more info.
 * 
 * @param [in] pipe Pipe side to set the check period of
 * @param [in] check_period Period in ticks
 */
void pipe_set_state_check_period(PipeSide* pipe, FuriWait check_period);

/**
 * @brief Receives data from the pipe.
 * 
 * This function will try to receive all of the requested bytes from the pipe.
 * If at some point during the operation the pipe becomes broken, this function
 * will return prematurely, in which case the return value will be less than the
 * requested `length`.
 * 
 * @param [in] pipe The pipe side to read data out of
 * @param [out] data The buffer to fill with data
 * @param length Maximum length of data to read
 * @returns The number of bytes actually written into the provided buffer
 */
size_t pipe_receive(PipeSide* pipe, void* data, size_t length);

/**
 * @brief Sends data into the pipe.
 * 
 * This function will try to send all of the requested bytes to the pipe.
 * If at some point during the operation the pipe becomes broken, this function
 * will return prematurely, in which case the return value will be less than the
 * requested `length`.
 * 
 * @param [in] pipe The pipe side to send data into
 * @param [out] data The buffer to get data from
 * @param length Maximum length of data to send
 * @returns The number of bytes actually read from the provided buffer
 */
size_t pipe_send(PipeSide* pipe, const void* data, size_t length);

/**
 * @brief Determines how many bytes there are in the pipe available to be read.
 * 
 * @param [in] pipe Pipe side to query
 * @returns Number of bytes available to be read out from that side of the pipe
 */
size_t pipe_bytes_available(PipeSide* pipe);

/**
 * @brief Determines how many space there is in the pipe for data to be written
 * into.
 * 
 * @param [in] pipe Pipe side to query
 * @returns Number of bytes available to be written into that side of the pipe
 */
size_t pipe_spaces_available(PipeSide* pipe);

/**
 * @brief Attaches a `PipeSide` to a `FuriEventLoop`, allowing to attach
 * callbacks to the PipeSide.
 * 
 * @param [in] pipe       Pipe side to attach to the event loop
 * @param [in] event_loop Event loop to attach the pipe side to
 */
void pipe_attach_to_event_loop(PipeSide* pipe, FuriEventLoop* event_loop);

/**
 * @brief Detaches a `PipeSide` from the `FuriEventLoop` that it was previously
 * attached to.
 * 
 * @param [in] pipe Pipe side to detach to the event loop
 */
void pipe_detach_from_event_loop(PipeSide* pipe);

/**
 * @brief Callback for when data arrives to a `PipeSide`.
 * 
 * @param [in]    pipe    Pipe side that called the callback
 * @param [inout] context Custom context
 */
typedef void (*PipeSideDataArrivedCallback)(PipeSide* pipe, void* context);

/**
 * @brief Callback for when data is read out of the opposite `PipeSide`.
 * 
 * @param [in]    pipe    Pipe side that called the callback
 * @param [inout] context Custom context
 */
typedef void (*PipeSideSpaceFreedCallback)(PipeSide* pipe, void* context);

/**
 * @brief Callback for when the opposite `PipeSide` is freed, making the pipe
 * broken.
 * 
 * @param [in]    pipe    Pipe side that called the callback
 * @param [inout] context Custom context
 */
typedef void (*PipeSideBrokenCallback)(PipeSide* pipe, void* context);

/**
 * @brief Sets the custom context for all callbacks.
 * 
 * @param [in]    pipe    Pipe side to set the context of
 * @param [inout] context Custom context that will be passed to callbacks
 */
void pipe_set_callback_context(PipeSide* pipe, void* context);

/**
 * @brief Sets the callback for when data arrives.
 * 
 * @param [in] pipe     Pipe side to assign the callback to
 * @param [in] callback Callback to assign to the pipe side. Set to NULL to
 *                      unsubscribe.
 * @param [in] event    Additional event loop flags (e.g. `Edge`, `Once`, etc.).
 *                      Non-flag values of the enum are not allowed.
 * 
 * @warning Attach the pipe side to an event loop first using
 * `pipe_attach_to_event_loop`.
 */
void pipe_set_data_arrived_callback(
    PipeSide* pipe,
    PipeSideDataArrivedCallback callback,
    FuriEventLoopEvent event);

/**
 * @brief Sets the callback for when data is read out of the opposite `PipeSide`.
 * 
 * @param [in] pipe     Pipe side to assign the callback to
 * @param [in] callback Callback to assign to the pipe side. Set to NULL to
 *                      unsubscribe.
 * @param [in] event    Additional event loop flags (e.g. `Edge`, `Once`, etc.).
 *                      Non-flag values of the enum are not allowed.
 * 
 * @warning Attach the pipe side to an event loop first using
 * `pipe_attach_to_event_loop`.
 */
void pipe_set_space_freed_callback(
    PipeSide* pipe,
    PipeSideSpaceFreedCallback callback,
    FuriEventLoopEvent event);

/**
 * @brief Sets the callback for when the opposite `PipeSide` is freed, making
 * the pipe broken.
 * 
 * @param [in] pipe     Pipe side to assign the callback to
 * @param [in] callback Callback to assign to the pipe side. Set to NULL to
 *                      unsubscribe.
 * @param [in] event    Additional event loop flags (e.g. `Edge`, `Once`, etc.).
 *                      Non-flag values of the enum are not allowed.
 * 
 * @warning Attach the pipe side to an event loop first using
 * `pipe_attach_to_event_loop`.
 */
void pipe_set_broken_callback(
    PipeSide* pipe,
    PipeSideBrokenCallback callback,
    FuriEventLoopEvent event);

#ifdef __cplusplus
}
#endif
