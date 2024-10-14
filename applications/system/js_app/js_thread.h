#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsThread JsThread;

typedef enum {
    JsThreadEventDone,
    JsThreadEventError,
    JsThreadEventPrint,
    JsThreadEventErrorTrace,
} JsThreadEvent;

typedef void (*JsThreadCallback)(JsThreadEvent event, const char* msg, void* context);

JsThread* js_thread_run(const char* script_path, JsThreadCallback callback, void* context);

void js_thread_stop(JsThread* worker);

#ifdef __cplusplus
}
#endif
