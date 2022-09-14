#pragma once
#include "view_modules/generic_view_module.h"
#include <map>
#include <core/check.h>
#include <gui/view_dispatcher.h>
#include <callback-connector.h>
#include "typeindex_no_rtti.hpp"

/**
 * @brief Controller for switching application views and handling inputs and events
 * 
 * @tparam TApp application class
 * @tparam TViewModules variadic list of ViewModules
 */
template <typename TApp, typename... TViewModules>
class ViewController {
public:
    ViewController() {
        event_queue = furi_message_queue_alloc(10, sizeof(typename TApp::Event));

        view_dispatcher = view_dispatcher_alloc();
        previous_view_callback_pointer = cbc::obtain_connector(
            this, &ViewController<TApp, TViewModules...>::previous_view_callback);

        [](...) {
        }((this->add_view(ext::make_type_index<TViewModules>().hash_code(), new TViewModules()),
           0)...);

        gui = static_cast<Gui*>(furi_record_open("gui"));
    };

    ~ViewController() {
        for(auto& it : holder) {
            view_dispatcher_remove_view(view_dispatcher, static_cast<uint32_t>(it.first));
            delete it.second;
        }

        view_dispatcher_free(view_dispatcher);
        furi_message_queue_free(event_queue);
    }

    /**
     * @brief Get ViewModule pointer
     * 
     * @tparam T Concrete ViewModule class
     * @return T* ViewModule pointer
     */
    template <typename T>
    T* get() {
        uint32_t view_index = ext::make_type_index<T>().hash_code();
        furi_check(holder.count(view_index) != 0);
        return static_cast<T*>(holder[view_index]);
    }

    /**
     * @brief Get ViewModule pointer by cast
     * 
     * @tparam T Concrete ViewModule class
     * @return T* ViewModule pointer
     */
    template <typename T>
    operator T*() {
        uint32_t view_index = ext::make_type_index<T>().hash_code();
        furi_check(holder.count(view_index) != 0);
        return static_cast<T*>(holder[view_index]);
    }

    /**
     * @brief Switch view to ViewModule
     * 
     * @tparam T Concrete ViewModule class
     * @return T* ViewModule pointer
     */
    template <typename T>
    void switch_to() {
        uint32_t view_index = ext::make_type_index<T>().hash_code();
        furi_check(holder.count(view_index) != 0);
        view_dispatcher_switch_to_view(view_dispatcher, view_index);
    }

    /**
     * @brief Receive event from app event queue
     * 
     * @param event event pointer
     */
    void receive_event(typename TApp::Event* event) {
        if(furi_message_queue_get(event_queue, event, 100) != FuriStatusOk) {
            event->type = TApp::EventType::Tick;
        }
    }

    /**
     * @brief Send event to app event queue
     * 
     * @param event event pointer
     */
    void send_event(typename TApp::Event* event) {
        FuriStatus result = furi_message_queue_put(event_queue, event, FuriWaitForever);
        furi_check(result == FuriStatusOk);
    }

    void attach_to_gui(ViewDispatcherType type) {
        view_dispatcher_attach_to_gui(view_dispatcher, gui, type);
    }

private:
    /**
     * @brief ViewModulesHolder
     * 
     */
    std::map<size_t, GenericViewModule*> holder;

    /**
     * @brief App event queue
     * 
     */
    FuriMessageQueue* event_queue;

    /**
     * @brief Main ViewDispatcher pointer
     * 
     */
    ViewDispatcher* view_dispatcher;

    /**
     * @brief Gui record pointer
     * 
     */
    Gui* gui;

    /**
     * @brief Previous view callback fn pointer
     * 
     */
    ViewNavigationCallback previous_view_callback_pointer;

    /**
     * @brief Previous view callback fn
     * 
     * @param context not used
     * @return uint32_t VIEW_IGNORE
     */
    uint32_t previous_view_callback(void* context) {
        (void)context;

        typename TApp::Event event;
        event.type = TApp::EventType::Back;

        if(event_queue != NULL) {
            send_event(&event);
        }

        return VIEW_IGNORE;
    }

    /**
     * @brief Add ViewModule to holder
     * 
     * @param view_index view index in holder
     * @param view_module view module pointer
     */
    void add_view(size_t view_index, GenericViewModule* view_module) {
        furi_check(holder.count(view_index) == 0);
        holder[view_index] = view_module;

        View* view = view_module->get_view();
        view_dispatcher_add_view(view_dispatcher, static_cast<uint32_t>(view_index), view);
        view_set_previous_callback(view, previous_view_callback_pointer);
    }
};
