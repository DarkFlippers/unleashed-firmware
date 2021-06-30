#include "lfrfid-app.h"

// app enter function
extern "C" int32_t lfrfid_app(void* args) {
    LfRfidApp* app = new LfRfidApp();
    app->run(args);
    delete app;

    return 0;
}
