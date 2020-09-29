#pragma once

#include "furi.h"

FuriRecordSubscriber* get_default_log();
void fuprintf(FuriRecordSubscriber* f, const char* format, ...);
