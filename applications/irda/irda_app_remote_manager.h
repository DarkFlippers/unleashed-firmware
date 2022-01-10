#pragma once

#include "irda_app_signal.h"

#include <irda_worker.h>
#include <irda.h>

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

class IrdaAppRemoteButton {
    friend class IrdaAppRemoteManager;
    std::string name;
    IrdaAppSignal signal;

public:
    IrdaAppRemoteButton(const char* name, const IrdaAppSignal& signal)
        : name(name)
        , signal(signal) {
    }

    IrdaAppRemoteButton(const char* name, IrdaAppSignal&& signal)
        : name(name)
        , signal(std::move(signal)) {
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
    std::unique_ptr<IrdaAppRemote> remote;
    std::string make_full_name(const std::string& remote_name) const;
    std::string make_remote_name(const std::string& full_name) const;

public:
    static constexpr const uint32_t max_button_name_length = 22;
    static constexpr const uint32_t max_remote_name_length = 22;
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
