#include "lfrfid-app.h"

// app enter function
extern "C" int32_t lfrfid_app(void* p) {
    LfRfidApp* app = new LfRfidApp();
    app->run();
    delete app;

    return 0;
}
