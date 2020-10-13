#pragma once

#include "flipper.h"

/*
== Flipper universal registry implementation (FURI) ==

## Requirements

* start daemon app
* kill app
* start child thread (kill when parent app was killed)
* switch between UI apps
*/

/*
Create record.
creates new record in registry and store pointer into it
*/
bool furi_create(const char* name, void* ptr);

/*
Open record.
get stored pointer by its name
*/
void* furi_open(const char* name);
