#pragma once

#include "file_browser_worker.h"

// Internal APIs used only by FileBrowser inside the firmware binary

#ifdef __cplusplus
extern "C" {
#endif

const char* file_browser_worker_get_path_current(BrowserWorker* browser);

#ifdef __cplusplus
}
#endif
