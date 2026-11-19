/**
 * @file sim800_phonebook.h
 * @brief Numbers in /ext/phonebook, presets in the app's data folder.
 *
 * Presets stay out of the phonebook folder because the Archive Phone tab and the Saved browser
 * both list it, and a preset is not a number.
 */
#pragma once

#include <furi.h>
#include <stdbool.h>

/** Create both folders. */
void sim800_storage_make_folders(void);

/** Write /ext/phonebook/<name>.phone, overwriting any file of that name. */
bool sim800_phonebook_save(const char* name, const char* number);

/** @param name may be NULL. */
bool sim800_phonebook_load(const char* path, FuriString* name, FuriString* number);

bool sim800_phonebook_delete(const char* path);

bool sim800_preset_save(const char* name, const char* text);
bool sim800_preset_load(const char* path, FuriString* text);
bool sim800_preset_delete(const char* path);

/** Fill @p names (caller's array of @p max allocated strings) with preset names, no extension. */
size_t sim800_preset_list(FuriString** names, size_t max);

/** How many entries the phonebook holds, to decide whether picking one is worth offering. */
size_t sim800_phonebook_count(void);

/** Full path of the phonebook entry called @p name. */
void sim800_phonebook_path(const char* name, FuriString* path);

/** Full path of the preset called @p name. */
void sim800_preset_path(const char* name, FuriString* path);

/** Folder and extension stripped off @p path. */
void sim800_path_to_name(const char* path, FuriString* name);
