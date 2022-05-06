/**
 * @file empty_screen.h
 * GUI: EmptyScreen view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Empty screen anonymous structure */
typedef struct EmptyScreen EmptyScreen;

/** Allocate and initialize empty screen
 *
 * This empty screen used to ask simple questions like Yes/
 *
 * @return     EmptyScreen instance
 */
EmptyScreen* empty_screen_alloc();

/** Deinitialize and free empty screen
 *
 * @param      empty_screen  Empty screen instance
 */
void empty_screen_free(EmptyScreen* empty_screen);

/** Get empty screen view
 *
 * @param      empty_screen  Empty screen instance
 *
 * @return     View instance that can be used for embedding
 */
View* empty_screen_get_view(EmptyScreen* empty_screen);

#ifdef __cplusplus
}
#endif
