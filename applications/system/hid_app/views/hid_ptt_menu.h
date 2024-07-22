#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidPushToTalkMenu HidPushToTalkMenu;

typedef void (*PushToTalkMenuItemCallback)(
    void* context,
    uint32_t listIndex,
    FuriString* listLabel,
    uint32_t itemIndex,
    FuriString* itemLabel);

HidPushToTalkMenu* hid_ptt_menu_alloc(Hid* bt_hid);

void hid_ptt_menu_free(HidPushToTalkMenu* hid_ptt_menu);

View* hid_ptt_menu_get_view(HidPushToTalkMenu* hid_ptt_menu);

void ptt_menu_add_item_to_list(
    HidPushToTalkMenu* hid_ptt_menu,
    uint32_t list_index,
    const char* label,
    uint32_t index,
    PushToTalkMenuItemCallback callback,
    void* callback_context);

void ptt_menu_add_list(HidPushToTalkMenu* hid_ptt_menu, const char* label, uint32_t index);
