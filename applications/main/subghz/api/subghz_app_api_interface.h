#pragma once

#include <flipper_application/api_hashtable/api_hashtable.h>

/*
 * Resolver interface with private application's symbols.
 * Implementation is contained in subghz_app_api_table.cpp
 */
extern const ElfApiInterface* const subghz_application_api_interface;
