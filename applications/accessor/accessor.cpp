#include "accessor-app.h"

// app enter function
extern "C" int32_t accessor_app(void* p) {
    AccessorApp* app = new AccessorApp();
    app->run();
    delete app;

    return 255;
}