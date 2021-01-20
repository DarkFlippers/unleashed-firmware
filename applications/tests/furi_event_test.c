#include <furi.h>
#include "minunit.h"

static void furi_concurent_app(void* p) {
    Event* event = p;

    signal_event(event);

    furiac_exit(NULL);
}

void test_furi_event() {
    mu_assert(false, "please reimplement or delete test");

    /*Event event;

    mu_check(init_event(&event));

    // The event should not be signalled right after creation
    mu_check(!wait_event_with_timeout(&event, 100));

    // Create second app
    FuriApp* second_app __attribute__((unused)) =
        furiac_start(furi_concurent_app, "furi concurent app", (void*)&event);

    // The event should be signalled now
    mu_check(wait_event_with_timeout(&event, 100));

    // The event should not be signalled once it's processed
    mu_check(!wait_event_with_timeout(&event, 100));

    mu_check(delete_event(&event));*/
}
