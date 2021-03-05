#include <gui/view.h>

class SubghzViewSpectrumSettings {
public:
    SubghzViewSpectrumSettings();
    ~SubghzViewSpectrumSettings();

    View* get_view();

    // ok callback methods
    typedef void (*OkCallback)(void* context);
    void set_ok_callback(OkCallback callback, void* context);
    void call_ok_callback();

    // model data getters/setters
    void set_start_freq(uint32_t start_freq);
    uint32_t get_start_freq();

private:
    View* view;

    // ok callback data
    OkCallback ok_callback = nullptr;
    void* ok_callback_context = nullptr;
};
