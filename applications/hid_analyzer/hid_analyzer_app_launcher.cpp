#include "hid_analyzer_app.h"

// app enter function
extern "C" int32_t hid_analyzer_app(void* args) {
    HIDApp* app = new HIDApp();
    app->run(args);
    delete app;

    return 0;
}
