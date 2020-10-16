#include "heap.h"
#include "errno.h"

/*
Flipper devices inc.

Local fw build entry point.
*/

int app();

int main() {
    // this function is not thread-safe, so it must be called in single thread context
    if(!prvHeapInit()){
        return ENOMEM;
    }

    return app();
}