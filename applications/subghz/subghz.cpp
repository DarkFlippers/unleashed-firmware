#include "subghz-app.h"

// app enter function
extern "C" int32_t app_subghz(void* p) {
    SubghzApp* app = new SubghzApp();
    app->run();
    delete app;

    return 255;
}