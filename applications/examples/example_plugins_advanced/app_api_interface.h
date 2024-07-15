#pragma once

#include <flipper_application/api_hashtable/api_hashtable.h>

/* 
 * Resolver interface with private application's symbols. 
 * Implementation is contained in app_api_table.c
 */
extern const ElfApiInterface* const application_api_interface;
