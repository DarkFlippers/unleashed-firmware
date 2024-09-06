/**
 * @file infrared_remote.h
 * @brief Infrared remote library.
 *
 * An infrared remote contains zero or more infrared signals which
 * have a (possibly non-unique) name each.
 *
 * The current implementation does load only the names into the memory,
 * while the signals themselves are loaded on-demand one by one. In theory,
 * this should allow for quite large remotes with relatively bulky signals.
 */
#pragma once

#include "infrared_signal.h"

/**
 * @brief InfraredRemote opaque type declaration.
 */
typedef struct InfraredRemote InfraredRemote;

/**
 * @brief Create a new InfraredRemote instance.
 *
 * @returns pointer to the created instance.
 */
InfraredRemote* infrared_remote_alloc(void);

/**
 * @brief Delete an InfraredRemote instance.
 *
 * @param[in,out] remote pointer to the instance to be deleted.
 */
void infrared_remote_free(InfraredRemote* remote);

/**
 * @brief Reset an InfraredRemote instance.
 *
 * Resetting a remote clears its signal name list and
 * the associated file path.
 *
 * @param[in,out] remote pointer to the instance to be deleted.
 */
void infrared_remote_reset(InfraredRemote* remote);

/**
 * @brief Get an InfraredRemote instance's name.
 *
 * The name is deduced from the file path.
 *
 * The return value remains valid unless one of the following functions is called:
 * - infrared_remote_reset()
 * - infrared_remote_load()
 * - infrared_remote_create()
 *
 * @param[in] remote pointer to the instance to be queried.
 * @returns pointer to a zero-terminated string containing the name.
 */
const char* infrared_remote_get_name(const InfraredRemote* remote);

/**
 * @brief Get an InfraredRemote instance's file path.
 *
 * Same return value validity considerations as infrared_remote_get_name().
 *
 * @param[in] remote pointer to the instance to be queried.
 * @returns pointer to a zero-terminated string containing the path.
 */
const char* infrared_remote_get_path(const InfraredRemote* remote);

/**
 * @brief Get the number of signals listed in an InfraredRemote instance.
 *
 * @param[in] remote pointer to the instance to be queried.
 * @returns number of signals, zero or more
 */
size_t infrared_remote_get_signal_count(const InfraredRemote* remote);

/**
 * @brief Get the name of a signal listed in an InfraredRemote instance.
 *
 * @param[in] remote pointer to the instance to be queried.
 * @param[in] index index of the signal in question. Must be less than the total signal count.
 */
const char* infrared_remote_get_signal_name(const InfraredRemote* remote, size_t index);

/**
 * @brief Get the index of a signal listed in an InfraredRemote instance by its name.
 *
 * @param[in] remote pointer to the instance to be queried.
 * @param[in] name pointer to a zero-terminated string containig the name of the signal in question.
 * @param[out] index pointer to the variable to hold the signal index.
 * @returns true if a signal with the given name was found, false otherwise.
 */
bool infrared_remote_get_signal_index(
    const InfraredRemote* remote,
    const char* name,
    size_t* index);

/**
 * @brief Load a signal listed in an InfraredRemote instance.
 *
 * As mentioned above, the signals are loaded on-demand. The user code must call this function
 * each time it wants to interact with a new signal.
 *
 * @param[in] remote pointer to the instance to load from.
 * @param[out] signal pointer to the signal to load into. Must be allocated.
 * @param[in] index index of the signal to be loaded. Must be less than the total signal count.
 * @return InfraredErrorCodeNone if the signal was successfully loaded, otherwise error code.
 */
InfraredErrorCode
    infrared_remote_load_signal(const InfraredRemote* remote, InfraredSignal* signal, size_t index);

/**
 * @brief Append a signal to the file associated with an InfraredRemote instance.
 *
 * The file path must be somehow initialised first by calling either infrared_remote_load() or
 * infrared_remote_create(). As the name suggests, the signal will be put in the end of the file.
 *
 * @param[in,out] remote pointer to the instance to append to.
 * @param[in] signal pointer to the signal to be appended.
 * @param[in] name pointer to a zero-terminated string containing the name of the signal.
 * @returns InfraredErrorCodeNone if the signal was successfully appended, otherwise error code.
 */
InfraredErrorCode infrared_remote_append_signal(
    InfraredRemote* remote,
    const InfraredSignal* signal,
    const char* name);

/**
 * @brief Insert a signal to the file associated with an InfraredRemote instance.
 *
 * Same behaviour as infrared_remote_append_signal(), but the user code can decide where to
 * put the signal in the file.
 *
 * Index values equal to or greater than the total signal count will result in behaviour
 * identical to infrared_remote_append_signal().
 *
 * @param[in,out] remote pointer to the instance to insert to.
 * @param[in] signal pointer to the signal to be inserted.
 * @param[in] name pointer to a zero-terminated string containing the name of the signal.
 * @param[in] index the index under which the signal shall be inserted.
 * @returns InfraredErrorCodeNone if the signal was successfully inserted, otherwise error
 * code describing what error happened ORed with index pointing which signal caused an error.
 */
InfraredErrorCode infrared_remote_insert_signal(
    InfraredRemote* remote,
    const InfraredSignal* signal,
    const char* name,
    size_t index);

/**
 * @brief Rename a signal in the file associated with an InfraredRemote instance.
 *
 * Only changes the signal's name, but neither its position nor contents.
 *
 * @param[in,out] remote pointer to the instance to be modified.
 * @param[in] index index of the signal to be renamed. Must be less than the total signal count.
 * @param[in] new_name pointer to a zero-terminated string containig the signal's new name.
 * @returns InfraredErrorCodeNone if the signal was successfully renamed, otherwise error code.
 */
InfraredErrorCode
    infrared_remote_rename_signal(InfraredRemote* remote, size_t index, const char* new_name);

/**
 * @brief Change a signal's position in the file associated with an InfraredRemote instance.
 *
 * Only changes the signal's position (index), but neither its name nor contents.
 *
 * @param[in,out] remote pointer to the instance to be modified.
 * @param[in] index index of the signal to be moved. Must be less than the total signal count.
 * @param[in] new_index index of the signal to be moved. Must be less than the total signal count.
 * @returns InfraredErrorCodeNone if the signal was moved successfully, otherwise error
 * code describing what error happened ORed with index pointing which signal caused an error.
 */
InfraredErrorCode
    infrared_remote_move_signal(InfraredRemote* remote, size_t index, size_t new_index);

/**
 * @brief Delete a signal in the file associated with an InfraredRemote instance.
 *
 * @param[in,out] remote pointer to the instance to be modified.
 * @param[in] index index of the signal to be deleted. Must be less than the total signal count.
 * @returns InfraredErrorCodeNone if the signal was successfully deleted, otherwise error
 * code describing what error happened ORed with index pointing which signal caused an error.
 */
InfraredErrorCode infrared_remote_delete_signal(InfraredRemote* remote, size_t index);

/**
 * @brief Create a new file and associate it with an InfraredRemote instance.
 *
 * The instance will be reset and given a new empty file with just the header.
 *
 * @param[in,out] remote pointer to the instance to be assigned with a new file.
 * @param[in] path pointer to a zero-terminated string containing the full file path.
 * @returns InfraredErrorCodeNone if the file was successfully created, otherwise error code.
 */
InfraredErrorCode infrared_remote_create(InfraredRemote* remote, const char* path);

/**
 * @brief Associate an InfraredRemote instance with a file and load the signal names from it.
 *
 * The instance will be reset and fill its signal name list from the given file.
 * The file must already exist and be valid.
 *
 * @param[in,out] remote pointer to the instance to be assigned with an existing file.
 * @param[in] path pointer to a zero-terminated string containing the full file path.
 * @returns InfraredErrorCodeNone if the file was successfully loaded, otherwise error code.
 */
InfraredErrorCode infrared_remote_load(InfraredRemote* remote, const char* path);

/**
 * @brief Rename the file associated with an InfraredRemote instance.
 *
 * Only renames the file, no signals are added, moved or deleted.
 *
 * @param[in,out] remote pointer to the instance to be modified.
 * @param[in] new_path pointer to a zero-terminated string containing the new full file path.
 * @returns InfraredErrorCodeNone if the file was successfully renamed, otherwise error code.
 */
InfraredErrorCode infrared_remote_rename(InfraredRemote* remote, const char* new_path);

/**
 * @brief Remove the file associated with an InfraredRemote instance.
 *
 * This operation is irreversible and fully deletes the remote file
 * from the underlying filesystem.
 * After calling this function, the instance becomes invalid until
 * infrared_remote_create() or infrared_remote_load() are successfully executed.
 *
 * @param[in,out] remote pointer to the instance to be modified.
 * @returns InfraredErrorCodeNone if the file was successfully removed, otherwise error code.
 */
InfraredErrorCode infrared_remote_remove(InfraredRemote* remote);
