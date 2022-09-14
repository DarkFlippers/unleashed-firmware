#pragma once
#include <stdint.h>
#include <m-list.h>
#include "views/bubble_animation_view.h"
#include <m-string.h>

/** Main structure to handle animation data.
 * Contains all, including animation playing data (BubbleAnimation),
 * data for random animation selection (StorageAnimationMeta) and
 * flag of location internal/external */
typedef struct StorageAnimation StorageAnimation;

typedef struct {
    const char* name;
    uint8_t min_butthurt;
    uint8_t max_butthurt;
    uint8_t min_level;
    uint8_t max_level;
    uint8_t weight;
} StorageAnimationManifestInfo;

/** Container to return available animations list */
LIST_DEF(StorageAnimationList, StorageAnimation*, M_PTR_OPLIST)
#define M_OPL_StorageAnimationList_t() LIST_OPLIST(StorageAnimationList)

/**
 * Fill list of available animations.
 * List will contain all idle animations on inner flash
 * and all available on SD-card, mentioned in manifest.txt.
 * Performs caching of animation. If fail - falls back to
 * inner animation.
 * List has to be initialized.
 *
 * @list        list to fill with animations data
 */
void animation_storage_fill_animation_list(StorageAnimationList_t* list);

/**
 * Get bubble animation of storage animation.
 * Bubble Animation is a structure which describes animation
 * independent of it's place of storage and meta data.
 * It contain all what is need to be played.
 * If storage_animation is not cached - caches it.
 *
 * @storage_animation       animation from which extract bubble animation
 * @return                  bubble_animation, NULL if failed to cache data.
 */
const BubbleAnimation* animation_storage_get_bubble_animation(StorageAnimation* storage_animation);

/**
 * Performs caching animation data (Bubble Animation)
 * if this is not done yet.
 *
 * @storage_animation       animation to cache
 */
void animation_storage_cache_animation(StorageAnimation* storage_animation);

/**
 * Find animation by name.
 * Search through the inner flash, and SD-card if has.
 *
 * @name        name of animation
 * @return      found animation. NULL if nothing found.
 */
StorageAnimation* animation_storage_find_animation(const char* name);

/**
 * Get meta information of storage animation.
 * This information allows to randomly select animation.
 * Also it contains name. Never returns NULL.
 *
 * @storage_animation       item of whom we have to extract meta.
 * @return                  meta itself
 */
StorageAnimationManifestInfo* animation_storage_get_meta(StorageAnimation* storage_animation);

/**
 * Free storage_animation, which previously acquired
 * by Animation Storage.
 *
 * @storage_animation   item to free. NULL-ed after all.
 */
void animation_storage_free_storage_animation(StorageAnimation** storage_animation);

/**
 * Has to be called at least 1 time to initialize runtime structures
 * of animations in inner flash.
 */
void animation_storage_initialize_internal_animations(void);
