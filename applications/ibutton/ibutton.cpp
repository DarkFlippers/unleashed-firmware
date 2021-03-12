#include "ibutton-app.h"

// app enter function
extern "C" int32_t app_ibutton(void* p) {
    iButtonApp* app = new iButtonApp();
    app->run();
    delete app;

    return 255;
}