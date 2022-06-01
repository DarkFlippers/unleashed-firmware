/**
  * @file infrared_app_remote_manager.h
  * Infrared: Remote manager class.
  * It holds remote, can load/save/rename remote,
  * add/remove/rename buttons.
  */
#pragma once

#include "infrared_app_signal.h"

#include "m-string.h"
#include <infrared_worker.h>
#include <infrared.h>

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

/** Class to handle remote button */
class InfraredAppRemoteButton {
    /** Allow field access */
    friend class InfraredAppRemoteManager;
    /** Name of signal */
    std::string name;
    /** Signal data */
    InfraredAppSignal signal;

public:
    /** Initialize remote button
     *
     * @param name - button name
     * @param signal - signal to copy for remote button
     */
    InfraredAppRemoteButton(const char* name, const InfraredAppSignal& signal)
        : name(name)
        , signal(signal) {
    }

    /** Initialize remote button
     *
     * @param name - button name
     * @param signal - signal to move for remote button
     */
    InfraredAppRemoteButton(const char* name, InfraredAppSignal&& signal)
        : name(name)
        , signal(std::move(signal)) {
    }

    /** Deinitialize remote button */
    ~InfraredAppRemoteButton() {
    }
};

/** Class to handle remote */
class InfraredAppRemote {
    /** Allow field access */
    friend class InfraredAppRemoteManager;
    /** Button container */
    std::vector<InfraredAppRemoteButton> buttons;
    /** Name of remote */
    std::string name;
    /** Path to remote file */
    string_t path;

public:
    /** Initialize new remote
     * 
     * @param path - remote file path
     */
    InfraredAppRemote(string_t file_path) {
        string_init_set(path, file_path);
    }

    ~InfraredAppRemote() {
        string_clear(path);
    }
};

/** Class to handle remote manager */
class InfraredAppRemoteManager {
    /** Remote instance. There can be 1 remote loaded at a time. */
    std::unique_ptr<InfraredAppRemote> remote;

public:
    /** Restriction to button name length. Buttons larger are ignored. */
    static constexpr const uint32_t max_button_name_length = 22;

    /** Restriction to remote name length. Remotes larger are ignored. */
    static constexpr const uint32_t max_remote_name_length = 22;

    /** Construct button from signal, and create remote
     *
     * @param button_name - name of button to create
     * @param signal - signal to create button from
     * @retval true for success, false otherwise
     * */
    bool add_remote_with_button(const char* button_name, const InfraredAppSignal& signal);

    /** Add button to current remote
     *
     * @param button_name - name of button to create
     * @param signal - signal to create button from
     * @retval true for success, false otherwise
     * */
    bool add_button(const char* button_name, const InfraredAppSignal& signal);

    /** Rename button in current remote
     *
     * @param index - index of button to rename
     * @param str - new button name
     */
    bool rename_button(uint32_t index, const char* str);

    /** Rename current remote
     *
     * @param str - new remote name
     */
    bool rename_remote(const char* str);

    /** Find vacant remote name. If suggested name is occupied,
     * incremented digit(2,3,4,etc) added to name and check repeated.
     *
     * @param name - suggested remote name
     * @param path - remote file path
     */
    void find_vacant_remote_name(string_t name, string_t path);

    /** Get button list
     *
     * @retval container of button names
     */
    std::vector<std::string> get_button_list() const;

    /** Get button name by index
     *
     * @param index - index of button to get name from
     * @retval button name
     */
    std::string get_button_name(uint32_t index);

    /** Get remote name
     *
     * @retval remote name
     */
    std::string get_remote_name();

    /** Get number of buttons
     *
     * @retval number of buttons
     */
    size_t get_number_of_buttons();

    /** Get button's signal
     *
     * @param index - index of interested button
     * @retval signal
     */
    const InfraredAppSignal& get_button_data(size_t index) const;

    /** Delete button
     *
     * @param index - index of interested button
     * @retval true if success, false otherwise
     */
    bool delete_button(uint32_t index);

    /** Delete remote
     *
     * @retval true if success, false otherwise
     */
    bool delete_remote();

    /** Clean all loaded info in current remote */
    void reset_remote();

    /** Store current remote data on disk
     *
     * @retval true if success, false otherwise
     */
    bool store();

    /** Load data from disk into current remote
     *
     * @param path - path to remote file
     * @retval true if success, false otherwise
     */
    bool load(string_t path);
};
