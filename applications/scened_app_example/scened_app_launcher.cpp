#include "scened_app.h"

// app enter function
extern "C" int32_t scened_app(void* p) {
    UNUSED(p);
    ScenedApp* app = new ScenedApp();
    app->run();
    delete app;

    return 0;
}
