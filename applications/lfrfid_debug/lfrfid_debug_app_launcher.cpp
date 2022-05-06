#include "lfrfid_debug_app.h"

// app enter function
extern "C" int32_t lfrfid_debug_app(void* p) {
    UNUSED(p);
    LfRfidDebugApp* app = new LfRfidDebugApp();
    app->run();
    delete app;

    return 0;
}
