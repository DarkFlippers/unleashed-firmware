#include "scened-app.h"

// app enter function
extern "C" int32_t scened_app(void* p) {
    ScenedApp* app = new ScenedApp();
    app->run();
    delete app;

    return 0;
}
