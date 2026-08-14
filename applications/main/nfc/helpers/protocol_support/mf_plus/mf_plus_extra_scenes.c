#include "mf_plus_extra_scenes.h"

#include "nfc/nfc_app_i.h"
#include "../nfc_protocol_support_gui_common.h"
#include <dolphin/dolphin.h>
#include <nfc/protocols/mf_plus/mf_plus_poller.h>
#include <nfc/protocols/mf_plus/mf_plus.h>
#include "mf_plus_render.h"

enum {
    MfPlusMoreInfoStateMenu,
    MfPlusMoreInfoStateItem, // states >= this: an item's sub-view is showing; index = state - Item
};

enum {
    SubmenuIndexViewDump,
    SubmenuIndexIso14443,
};

// ---- dict_attack -------------------------------------------------------
#undef TAG
#define TAG "NfcMfPlusDictAttack"

// The MIFARE Plus SL3 poller visits every sector once, asking for candidate keys per
// (sector, key type). Unlike MIFARE Classic it has no resume-from-prior-data path, so both the
// user and the built-in system dictionary are consumed within a SINGLE poller pass (user keys
// first) — restarting the poller between dictionaries would discard keys already recovered.

// Yield the next candidate key for the current (sector, key type): user dictionary first, then the
// system dictionary. Returns false only once both are exhausted for this request.
static bool mf_plus_scene_dict_attack_next_key(NfcMfPlusDictAttackContext* ctx, MfPlusKey* key) {
    if(!ctx->on_system_dict && ctx->user_dict != NULL) {
        if(keys_dict_get_next_key(ctx->user_dict, key->data, sizeof(MfPlusKey))) {
            return true;
        }
        // User dictionary exhausted for this request; continue with the system dictionary.
        ctx->on_system_dict = true;
    }

    if(ctx->system_dict != NULL) {
        return keys_dict_get_next_key(ctx->system_dict, key->data, sizeof(MfPlusKey));
    }

    return false;
}

static NfcCommand nfc_mf_plus_dict_attack_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfPlus);

    NfcCommand command = NfcCommandContinue;
    NfcApp* instance = context;
    MfPlusPollerEvent* mfp_event = event.event_data;
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;

    if(mfp_event->type == MfPlusPollerEventTypeRequestMode) {
        mfp_event->data->mode_request.mode = MfPlusPollerModeRead;
        // The poller has parsed the version/type by now, so the card geometry is known.
        const MfPlusData* data = nfc_poller_get_data(instance->poller);
        ctx->sectors_total = mf_plus_get_sector_count(data->size);
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfp_event->type == MfPlusPollerEventTypeRequestKey) {
        const bool is_admin = mfp_event->data->key_request.is_admin;
        const uint8_t sector = mfp_event->data->key_request.sector;
        const uint8_t key_type = mfp_event->data->key_request.key_type;
        const uint8_t admin_type = mfp_event->data->key_request.admin_type;

        // A change of target (sector key A/B, or which admin key) since the last request means the
        // poller advanced; restart the combined key stream from the top of the user dictionary.
        bool target_changed;
        if(!ctx->request_seen || is_admin != ctx->last_is_admin) {
            target_changed = true;
        } else if(is_admin) {
            target_changed = admin_type != ctx->last_admin_type;
        } else {
            target_changed = sector != ctx->last_sector || key_type != ctx->last_key_type;
        }
        if(target_changed) {
            if(ctx->user_dict != NULL) keys_dict_rewind(ctx->user_dict);
            if(ctx->system_dict != NULL) keys_dict_rewind(ctx->system_dict);
            ctx->on_system_dict = (ctx->user_dict == NULL);
            ctx->dict_keys_current = 0;
            ctx->cache_key_fed = false;
            ctx->request_seen = true;
            ctx->last_is_admin = is_admin;
            ctx->last_sector = sector;
            ctx->last_key_type = key_type;
            ctx->last_admin_type = admin_type;
        }
        // Admin keys aren't sectors; leave the sector display untouched during the admin phase.
        if(!is_admin) ctx->current_sector = sector;

        MfPlusKey key = {};
        bool cache_hit = false;
        // Offer the cached key for this target first, once. Cache hits don't advance the dictionary
        // progress counter -- they aren't part of dict_keys_total. If the cached key fails to
        // authenticate the poller re-requests the same target, and cache_key_fed then routes it to
        // the dictionaries (covers a re-keyed card).
        if(!ctx->cache_key_fed) {
            ctx->cache_key_fed = true;
            cache_hit =
                is_admin ?
                    mf_plus_key_cache_get_admin_key(ctx->key_cache, admin_type, &key) :
                    mf_plus_key_cache_get_sector_key(ctx->key_cache, sector, key_type, &key);
        }
        if(cache_hit) {
            mfp_event->data->key_request.key = key;
            mfp_event->data->key_request.key_provided = true;
        } else if(mf_plus_scene_dict_attack_next_key(ctx, &key)) {
            mfp_event->data->key_request.key = key;
            mfp_event->data->key_request.key_provided = true;
            ctx->dict_keys_current++;
            if(ctx->dict_keys_current % 10 == 0) {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
            }
        } else {
            mfp_event->data->key_request.key_provided = false;
        }
    } else if(mfp_event->type == MfPlusPollerEventTypeDataUpdate) {
        ctx->current_sector = mfp_event->data->data_update.current_sector;
        ctx->sectors_read = mfp_event->data->data_update.sectors_read;
        ctx->keys_found = mfp_event->data->data_update.keys_found;
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfp_event->type == MfPlusPollerEventTypeReadSuccess) {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackComplete);
        command = NfcCommandStop;
    } else if(mfp_event->type == MfPlusPollerEventTypeReadFailed) {
        // A lost card or comms fault is NOT the same as an incomplete dictionary: log it and route
        // to a distinct failure path so the scan is never graded as a clean read. The finish
        // handler still preserves whatever was recovered before the fault.
        FURI_LOG_W(
            TAG,
            "Read aborted (poller error %d) after %u/%u sectors",
            mfp_event->data->error,
            ctx->sectors_read,
            ctx->sectors_total);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
    } else {
        FURI_LOG_E(TAG, "Unhandled poller event %d", mfp_event->type);
    }

    return command;
}

static void mf_plus_scene_dict_attack_result_callback(DictAttackEvent event, void* context) {
    furi_assert(context);
    NfcApp* instance = context;
    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

static void mf_plus_scene_dict_attack_update_view(NfcApp* instance) {
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;
    dict_attack_set_sectors_total(instance->dict_attack, ctx->sectors_total);
    dict_attack_set_sectors_read(instance->dict_attack, ctx->sectors_read);
    dict_attack_set_keys_found(instance->dict_attack, ctx->keys_found);
    dict_attack_set_current_sector(instance->dict_attack, ctx->current_sector);
    dict_attack_set_current_dict_key(instance->dict_attack, ctx->dict_keys_current);
}

static void mf_plus_scene_dict_attack_setup_dicts(NfcApp* instance) {
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;

    // The user dictionary is optional and lazily created by the key-management scenes; drop the
    // handle when the file is missing or holds no keys so the pass falls straight to the system one.
    if(keys_dict_check_presence(NFC_APP_MF_PLUS_DICT_USER_PATH)) {
        ctx->user_dict = keys_dict_alloc(
            NFC_APP_MF_PLUS_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfPlusKey));
        if(keys_dict_get_total_keys(ctx->user_dict) == 0) {
            keys_dict_free(ctx->user_dict);
            ctx->user_dict = NULL;
        }
    }
    // A missing system dictionary is not fatal (the attack just finds nothing), but it means a
    // broken install, so make it diagnosable instead of a silent "found no keys".
    if(!keys_dict_check_presence(NFC_APP_MF_PLUS_DICT_SYSTEM_PATH)) {
        FURI_LOG_W(TAG, "System dictionary %s is missing", NFC_APP_MF_PLUS_DICT_SYSTEM_PATH);
    }
    ctx->system_dict = keys_dict_alloc(
        NFC_APP_MF_PLUS_DICT_SYSTEM_PATH, KeysDictModeOpenExisting, sizeof(MfPlusKey));

    ctx->dict_keys_total =
        (ctx->user_dict != NULL ? keys_dict_get_total_keys(ctx->user_dict) : 0) +
        keys_dict_get_total_keys(ctx->system_dict);
    ctx->dict_keys_current = 0;
    ctx->on_system_dict = (ctx->user_dict == NULL);
    ctx->request_seen = false;

    // Per-UID key cache: if this exact card was saved before, its recovered keys authenticate on the
    // first try per sector, so the pass flies through instead of walking the dictionaries. A miss
    // (no cache file / different card) leaves the cache empty and the attack runs exactly as before.
    ctx->key_cache = mf_plus_key_cache_alloc();
    ctx->cache_key_fed = false;
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    size_t uid_len = 0;
    const uint8_t* uid = mf_plus_get_uid(data, &uid_len);
    if(mf_plus_key_cache_load(ctx->key_cache, uid, uid_len)) {
        FURI_LOG_I(TAG, "Key cache hit; seeding dictionary attack with saved keys");
    }

    // The dict_attack view is shared; its type persists from whichever scene last used it (e.g. the
    // Ultralight-C scene never resets it), so set the sector-oriented layout explicitly on entry.
    dict_attack_set_type(instance->dict_attack, DictAttackTypeMfClassic);
    dict_attack_set_header(instance->dict_attack, "MF Plus Dictionary");
    dict_attack_set_total_dict_keys(instance->dict_attack, ctx->dict_keys_total);
    dict_attack_set_current_dict_key(instance->dict_attack, 0);
    dict_attack_set_callback(
        instance->dict_attack, mf_plus_scene_dict_attack_result_callback, instance);
}

static void mf_plus_scene_dict_attack_on_enter(NfcApp* instance) {
    mf_plus_scene_dict_attack_setup_dicts(instance);
    mf_plus_scene_dict_attack_update_view(instance);
    dict_attack_set_card_state(instance->dict_attack, true);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfPlus);
    nfc_poller_start(instance->poller, nfc_mf_plus_dict_attack_worker_callback, instance);
}

static void mf_plus_scene_dict_attack_finish(NfcApp* instance, bool aborted) {
    // Merge rather than replace: this poller is never seeded with the card we arrived with, so
    // adopting it outright drops anything this pass did not recover -- everything when Skip is
    // pressed before the poller even activates, which it can be from the first frame.
    MfPlusData* merged = mf_plus_alloc();
    nfc_device_copy_data(instance->nfc_device, NfcProtocolMfPlus, merged);
    mf_plus_merge_update(merged, nfc_poller_get_data(instance->poller));
    nfc_device_set_data(instance->nfc_device, NfcProtocolMfPlus, merged);
    mf_plus_free(merged);

    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    // A clean pass that captured every block is a full success; an aborted scan (comms fault / lost
    // card) or a partial recovery is semi-success. mf_plus_is_card_read checks captured blocks, not
    // the poller's sector counter, which also counts sectors that aborted mid-read.
    bool fully_read = !aborted && mf_plus_is_card_read(data);
    notification_message(
        instance->notifications, fully_read ? &sequence_success : &sequence_semi_success);
    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
    dolphin_deed(DolphinDeedNfcReadSuccess);
}

static bool mf_plus_scene_dict_attack_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventDictAttackComplete ||
           event.event == NfcCustomEventDictAttackSkip) {
            mf_plus_scene_dict_attack_finish(instance, false);
            consumed = true;
        } else if(event.event == NfcCustomEventPollerFailure) {
            mf_plus_scene_dict_attack_finish(instance, true);
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackDataUpdate) {
            mf_plus_scene_dict_attack_update_view(instance);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }

    return consumed;
}

static void mf_plus_scene_dict_attack_on_exit(NfcApp* instance) {
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    dict_attack_reset(instance->dict_attack);

    if(ctx->user_dict != NULL) {
        keys_dict_free(ctx->user_dict);
        ctx->user_dict = NULL;
    }
    if(ctx->system_dict != NULL) {
        keys_dict_free(ctx->system_dict);
        ctx->system_dict = NULL;
    }
    if(ctx->key_cache != NULL) {
        mf_plus_key_cache_free(ctx->key_cache);
        ctx->key_cache = NULL;
    }

    ctx->on_system_dict = false;
    ctx->sectors_total = 0;
    ctx->sectors_read = 0;
    ctx->current_sector = 0;
    ctx->keys_found = 0;
    ctx->dict_keys_total = 0;
    ctx->dict_keys_current = 0;
    ctx->request_seen = false;
    ctx->last_is_admin = false;
    ctx->last_sector = 0;
    ctx->last_key_type = 0;
    ctx->last_admin_type = 0;
    ctx->cache_key_fed = false;

    nfc_blink_stop(instance);
}

// ---- show_keys ---------------------------------------------------------
#undef TAG
#define TAG "NfcMfPlusShowKeys"

static void mf_plus_scene_show_keys_callback(GuiButtonType button, InputType type, void* context) {
    NfcApp* instance = context;
    if(button == GuiButtonTypeLeft && type == InputTypeShort) {
        scene_manager_previous_scene(instance->scene_manager);
    }
}

// Append a 16-byte AES key as compact uppercase hex (no separators -- 16 bytes are long enough that
// per-byte spaces would wrap the scroll line several times).
static void mf_plus_scene_show_keys_cat_key(FuriString* str, const MfPlusKey* key) {
    for(uint8_t i = 0; i < MF_PLUS_KEY_SIZE; i++) {
        furi_string_cat_printf(str, "%02X", key->data[i]);
    }
}

static void mf_plus_scene_show_keys_on_enter(NfcApp* instance) {
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    FuriString* str = instance->text_box_store;

    furi_string_reset(str);
    nfc_append_filename_string_when_present(instance, str);
    furi_string_cat_printf(str, "\e#Found MFP Keys:");

    // Sector keys: list only sectors with a recovered key, omitting unknown ones (as MIFARE Classic
    // does), so the screen stays compact on a partially-read card.
    const uint8_t num_sectors = mf_plus_get_sector_count(data->size);
    uint8_t found_a = 0, found_b = 0;
    for(uint8_t sector = 0; sector < num_sectors; sector++) {
        const bool key_a = mf_plus_is_key_found(data, sector, MfPlusKeyTypeA);
        const bool key_b = mf_plus_is_key_found(data, sector, MfPlusKeyTypeB);
        if(!key_a && !key_b) continue;

        furi_string_cat_printf(str, "\n  -> Sector %u", sector);
        if(key_a) {
            found_a++;
            furi_string_cat_printf(str, "\n\e*A: ");
            mf_plus_scene_show_keys_cat_key(str, &data->key_a[sector]);
        }
        if(key_b) {
            found_b++;
            furi_string_cat_printf(str, "\n\e*B: ");
            mf_plus_scene_show_keys_cat_key(str, &data->key_b[sector]);
        }
    }

    // Admin keys (Card Master / Card Config / L3 Switch / SL1 Card Auth), recovered by the same dict
    // attack. Shown under their own heading, again only the ones actually recovered.
    uint8_t found_admin = 0;
    for(uint8_t type = 0; type < MfPlusAdminKeyNum; type++) {
        if(!mf_plus_is_admin_key_found(data, type)) continue;
        if(found_admin == 0) furi_string_cat_printf(str, "\n\e*Admin Keys:");
        found_admin++;
        furi_string_cat_printf(str, "\n\e*%s: ", mf_plus_get_admin_key_name(type));
        mf_plus_scene_show_keys_cat_key(str, &data->admin_key[type]);
    }

    if(found_a == 0 && found_b == 0 && found_admin == 0) {
        furi_string_cat_printf(str, "\n\nNo keys recovered yet.");
    }

    furi_string_cat_printf(
        str,
        "\nTotal keys found:\n -> %u/%u A keys\n -> %u/%u B keys\n -> %u/%u admin keys",
        found_a,
        num_sectors,
        found_b,
        num_sectors,
        found_admin,
        (uint8_t)MfPlusAdminKeyNum);

    widget_add_text_scroll_element(instance->widget, 2, 2, 124, 60, furi_string_get_cstr(str));
    widget_add_button_element(
        instance->widget, GuiButtonTypeLeft, "Back", mf_plus_scene_show_keys_callback, instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool mf_plus_scene_show_keys_on_event(NfcApp* nfc, SceneManagerEvent event) {
    UNUSED(nfc);
    UNUSED(event);
    return false;
}

static void mf_plus_scene_show_keys_on_exit(NfcApp* instance) {
    widget_reset(instance->widget);
    furi_string_reset(instance->text_box_store);
}

// ---- more_info ---------------------------------------------------------
#undef TAG
// "More info" hub, reached from the Info screen's "More" button (mf_plus's scene_more_info jumps
// straight here, DESFire-style). It splits the recovered SL3 block dump from the protocol details:
// "View Dump" is shown in a memory-light text_box within this scene (see nfc_render_mf_plus_dump for
// why a text_box and not a text-scroll widget); "ISO14443-4 Data" opens its own scene.

static void mf_plus_scene_more_info_on_enter(NfcApp* instance) {
    Submenu* submenu = instance->submenu;
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    // A dump only exists for SL3 (the only level with recovered blocks/keys); other levels still
    // have ISO14443-4/version details worth showing.
    if(data->security_level == MfPlusSecurityLevel3) {
        submenu_add_item(
            submenu,
            "View Dump",
            SubmenuIndexViewDump,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }
    submenu_add_item(
        submenu,
        "ISO14443-4 Data",
        SubmenuIndexIso14443,
        nfc_protocol_support_common_submenu_callback,
        instance);

    // Restore the selection after returning from a sub-view (e.g. the ISO14443-4 Data scene).
    const uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusMoreInfo);
    if(state >= MfPlusMoreInfoStateItem) {
        submenu_set_selected_item(submenu, state - MfPlusMoreInfoStateItem);
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneMfPlusMoreInfo, MfPlusMoreInfoStateMenu);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

static bool mf_plus_scene_more_info_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    const uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusMoreInfo);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexViewDump) {
            const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
            furi_string_reset(instance->text_box_store);
            nfc_render_mf_plus_dump(data, instance->text_box_store);
            text_box_set_font(instance->text_box, TextBoxFontHex);
            text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));
            view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfPlusMoreInfo,
                MfPlusMoreInfoStateItem + SubmenuIndexViewDump);
            consumed = true;
        } else if(event.event == SubmenuIndexIso14443) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfPlusMoreInfo,
                MfPlusMoreInfoStateItem + SubmenuIndexIso14443);
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusIso4Info);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state >= MfPlusMoreInfoStateItem) {
            // Leaving the in-scene dump text_box: return to the menu, not out of the scene.
            view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                instance->scene_manager, NfcSceneMfPlusMoreInfo, MfPlusMoreInfoStateMenu);
        } else {
            // Skip the generic more_info scene (it auto-jumps back here) and return to Info.
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneInfo);
        }
        consumed = true;
    }

    return consumed;
}

static void mf_plus_scene_more_info_on_exit(NfcApp* instance) {
    text_box_reset(instance->text_box);
    furi_string_reset(instance->text_box_store);
    submenu_reset(instance->submenu);
}

// ---- iso4_info ---------------------------------------------------------
#undef TAG
// "ISO14443-4 Data" page: the ISO14443-4 protocol details as a scrollable widget, with a "More"
// button to the GetVersion page. The ISO14443-4 text is small (a screen or two), so the text-scroll
// widget's per-line copy is harmless here -- unlike the multi-KB block dump, which uses a text_box.
static void mf_plus_scene_iso4_info_on_enter(NfcApp* instance) {
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    furi_string_reset(instance->text_box_store);
    nfc_render_mf_plus_iso14443_4(data, instance->text_box_store);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(instance->text_box_store));
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeRight,
        "More",
        nfc_protocol_support_common_widget_callback,
        instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool mf_plus_scene_iso4_info_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom && event.event == GuiButtonTypeRight) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusVersion);
        consumed = true;
    }
    return consumed;
}

static void mf_plus_scene_iso4_info_on_exit(NfcApp* instance) {
    widget_reset(instance->widget);
    furi_string_reset(instance->text_box_store);
}

// ---- version -----------------------------------------------------------
#undef TAG
// GetVersion page, reached from the "ISO14443-4 Data" page's "More" button: the GetVersion fields,
// or a note when the card doesn't answer GetVersion (e.g. an SL3 EV0 that only speaks the Plus
// command set). Kept off the ISO14443-4 Data page so that view stays a clean protocol summary.
static void mf_plus_scene_version_on_enter(NfcApp* instance) {
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    furi_string_reset(instance->text_box_store);
    nfc_render_mf_plus_version_info(data, instance->text_box_store);

    text_box_set_font(instance->text_box, TextBoxFontText);
    text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
}

static bool mf_plus_scene_version_on_event(NfcApp* nfc, SceneManagerEvent event) {
    UNUSED(nfc);
    UNUSED(event);
    return false;
}

static void mf_plus_scene_version_on_exit(NfcApp* instance) {
    text_box_reset(instance->text_box);
    furi_string_reset(instance->text_box_store);
}

// ---- update_initial ----------------------------------------------------
#undef TAG
// "Update from Initial Card": re-read the source SL3 card using the dump's already-recovered keys
// and refresh the saved data (+ shadow file). Unlike the read scene it runs no dictionary attack --
// it just feeds back the known keys.

// The read poller re-requests the same (sector, key type) / admin key after a failed authentication
// to get the next dictionary candidate. Here there is only ever one candidate -- the recovered key
// -- so an immediate repeat means that key no longer works (the card was re-keyed): answer "no key"
// to let the scan advance instead of looping. The scene state holds (last fed key id) + 1, 0 = none.
// Sector keys: id = sector*2 + key_type (0..79); admin keys: 80 + admin_type.
#define MF_PLUS_UPDATE_SECTOR_KEY_ID(sector, key_type) ((uint32_t)(sector) * 2 + (key_type))
#define MF_PLUS_UPDATE_ADMIN_KEY_ID(type)              (80u + (uint32_t)(type))

static bool mf_plus_scene_update_initial_feed_once(NfcApp* instance, uint32_t key_id) {
    const uint32_t last =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial);
    if(last == key_id + 1) {
        // Same key requested again -> its previous feed didn't authenticate. Stop feeding it.
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial, 0);
        return false;
    }
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfPlusUpdateInitial, key_id + 1);
    return true;
}

static NfcCommand
    nfc_mf_plus_update_initial_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfPlus);

    NfcCommand command = NfcCommandContinue;
    const MfPlusPollerEvent* mfp_event = event.event_data;
    NfcApp* instance = context;
    const MfPlusData* old_data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    switch(mfp_event->type) {
    case MfPlusPollerEventTypeRequestMode: {
        // Only refresh from the exact card the dump came from: match the UID before reading.
        const MfPlusData* tag_data = nfc_poller_get_data(instance->poller);
        size_t tag_uid_len = 0, old_uid_len = 0;
        const uint8_t* tag_uid = mf_plus_get_uid(tag_data, &tag_uid_len);
        const uint8_t* old_uid = mf_plus_get_uid(old_data, &old_uid_len);
        if(tag_uid_len == old_uid_len && memcmp(tag_uid, old_uid, tag_uid_len) == 0) {
            mfp_event->data->mode_request.mode = MfPlusPollerModeRead;
        } else {
            view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
            command = NfcCommandStop;
        }
        break;
    }
    case MfPlusPollerEventTypeRequestKey: {
        MfPlusPollerEventDataKeyRequest* req = &mfp_event->data->key_request;
        if(req->is_admin) {
            if(mf_plus_is_admin_key_found(old_data, req->admin_type) &&
               mf_plus_scene_update_initial_feed_once(
                   instance, MF_PLUS_UPDATE_ADMIN_KEY_ID(req->admin_type))) {
                req->key = old_data->admin_key[req->admin_type];
                req->key_provided = true;
            }
        } else if(
            mf_plus_is_key_found(old_data, req->sector, req->key_type) &&
            mf_plus_scene_update_initial_feed_once(
                instance, MF_PLUS_UPDATE_SECTOR_KEY_ID(req->sector, req->key_type))) {
            req->key = (req->key_type == MfPlusKeyTypeB) ? old_data->key_b[req->sector] :
                                                           old_data->key_a[req->sector];
            req->key_provided = true;
        }
        break;
    }
    case MfPlusPollerEventTypeReadSuccess: {
        // Merge the re-read over the saved dump instead of replacing it: the poller reports success
        // even if the best-effort admin/config/signature phase was cut short, so a wholesale replace
        // could silently drop keys/config the dump already had.
        const MfPlusData* updated_data = nfc_poller_get_data(instance->poller);
        MfPlusData* merged = mf_plus_alloc();
        mf_plus_copy(merged, old_data);
        mf_plus_merge_update(merged, updated_data);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfPlus, merged);
        mf_plus_free(merged);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerExit);
        command = NfcCommandStop;
        break;
    }
    case MfPlusPollerEventTypeReadFailed:
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
        command = NfcCommandStop;
        break;
    default:
        break;
    }

    return command;
}

static void mf_plus_scene_update_initial_on_enter(NfcApp* instance) {
    dolphin_deed(DolphinDeedNfcEmulate);

    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial, 0);

    Popup* popup = instance->popup;
    popup_reset(popup);
    popup_set_text(popup, "Use the source\ncard only", 128, 32, AlignRight, AlignCenter);
    popup_set_icon(popup, 0, 8, &I_NFC_manual_60x50);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfPlus);
    nfc_poller_start(instance->poller, nfc_mf_plus_update_initial_worker_callback, instance);
    nfc_blink_emulate_start(instance);
}

static bool mf_plus_scene_update_initial_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWrongCard) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneUpdateInitialWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventWorkerExit) {
            if(nfc_save_shadow_file(instance)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneUpdateInitialSuccess);
            } else {
                scene_manager_next_scene(instance->scene_manager, NfcSceneUpdateInitialWrongCard);
            }
            consumed = true;
        }
    }

    return consumed;
}

static void mf_plus_scene_update_initial_on_exit(NfcApp* instance) {
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial, 0);
    popup_reset(instance->popup);
    nfc_blink_stop(instance);
}

const NfcProtocolSupportExtraScene mf_plus_extra_scenes[MfPlusExtraSceneNum] = {
    [MfPlusExtraSceneDictAttack] =
        {
            .on_enter = mf_plus_scene_dict_attack_on_enter,
            .on_event = mf_plus_scene_dict_attack_on_event,
            .on_exit = mf_plus_scene_dict_attack_on_exit,
        },
    [MfPlusExtraSceneShowKeys] =
        {
            .on_enter = mf_plus_scene_show_keys_on_enter,
            .on_event = mf_plus_scene_show_keys_on_event,
            .on_exit = mf_plus_scene_show_keys_on_exit,
        },
    [MfPlusExtraSceneMoreInfo] =
        {
            .on_enter = mf_plus_scene_more_info_on_enter,
            .on_event = mf_plus_scene_more_info_on_event,
            .on_exit = mf_plus_scene_more_info_on_exit,
        },
    [MfPlusExtraSceneIso4Info] =
        {
            .on_enter = mf_plus_scene_iso4_info_on_enter,
            .on_event = mf_plus_scene_iso4_info_on_event,
            .on_exit = mf_plus_scene_iso4_info_on_exit,
        },
    [MfPlusExtraSceneVersion] =
        {
            .on_enter = mf_plus_scene_version_on_enter,
            .on_event = mf_plus_scene_version_on_event,
            .on_exit = mf_plus_scene_version_on_exit,
        },
    [MfPlusExtraSceneUpdateInitial] =
        {
            .on_enter = mf_plus_scene_update_initial_on_enter,
            .on_event = mf_plus_scene_update_initial_on_event,
            .on_exit = mf_plus_scene_update_initial_on_exit,
        },
};
