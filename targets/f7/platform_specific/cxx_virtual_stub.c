#include "cxx_virtual_stub.h"
#include <furi.h>

void __cxa_pure_virtual(void) {
    furi_crash("C++ pure virtual call");
}
