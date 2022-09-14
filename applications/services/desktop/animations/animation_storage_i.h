#pragma once
#include "animation_storage.h"
#include "animation_manager.h"

struct StorageAnimation {
    const BubbleAnimation* animation;
    bool external;
    StorageAnimationManifestInfo manifest_info;
};
