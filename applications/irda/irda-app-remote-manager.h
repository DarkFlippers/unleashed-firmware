#pragma once
#include <irda_worker.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <irda.h>
#include <storage/storage.h>
#include "irda-app-signal.h"

class IrdaAppRemoteButton {
    friend class IrdaAppRemoteManager;
    std::string name;
    IrdaAppSignal signal;

public:
    IrdaAppRemoteButton(const char* name, const IrdaAppSignal& signal)
        : name(name)
        , signal(signal) {
    }
    ~IrdaAppRemoteButton() {
    }
};

class IrdaAppRemote {
    friend class IrdaAppRemoteManager;
    std::vector<IrdaAppRemoteButton> buttons;
    std::string name;

public:
    IrdaAppRemote(const std::string& name)
        : name(name) {
    }

    IrdaAppRemote& operator=(std::string& new_name) noexcept {
        name = new_name;
        buttons.clear();
        return *this;
    }
};

class IrdaAppRemoteManager {
    static const char* irda_directory;
    static const char* irda_extension;
    std::unique_ptr<IrdaAppRemote> remote;
    std::string make_full_name(const std::string& remote_name) const;
    std::string make_remote_name(const std::string& full_name) const;

public:
    bool add_remote_with_button(const char* button_name, const IrdaAppSignal& signal);
    bool add_button(const char* button_name, const IrdaAppSignal& signal);

    int find_remote_name(const std::vector<std::string>& strings);
    bool rename_button(uint32_t index, const char* str);
    bool rename_remote(const char* str);
    std::string find_vacant_remote_name(const std::string& name);

    std::vector<std::string> get_button_list() const;
    std::string get_button_name(uint32_t index);
    std::string get_remote_name();
    size_t get_number_of_buttons();
    const IrdaAppSignal& get_button_data(size_t index) const;
    bool delete_button(uint32_t index);
    bool delete_remote();
    void reset_remote();

    bool store();
    bool load(const std::string& name);
};
