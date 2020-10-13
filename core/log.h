#pragma once

#include "flipper.h"

FuriRecordSubscriber* get_default_log();
void fuprintf(FuriRecordSubscriber* f, const char* format, ...);
