#include "target.h"

int main() {
    // Initialize hardware
    target_init();
    // Check if dfu requested
    if(target_is_dfu_requested()) {
        target_switch2dfu();
    }
    // Switch to OS
    target_switch2os();
    // Never should get here
    return 0;
}