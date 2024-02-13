#include "cxx_virtual_stub.h"
#include <furi.h>

void __cxa_pure_virtual() {
    furi_crash("C++ pure virtual call");
}
