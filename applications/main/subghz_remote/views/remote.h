#pragma once

#include <gui/view.h>
#include "../helpers/subrem_custom_event.h"
#include "../helpers/subrem_presets.h"

typedef enum {
    SubRemViewRemoteStateIdle,
    SubRemViewRemoteStateLoading,
    SubRemViewRemoteStateSending,
    SubRemViewRemoteStateOFF,
} SubRemViewRemoteState;

typedef struct SubRemViewRemote SubRemViewRemote;

typedef void (*SubRemViewRemoteCallback)(SubRemCustomEvent event, void* context);

void subrem_view_remote_set_callback(
    SubRemViewRemote* subrem_view_remote,
    SubRemViewRemoteCallback callback,
    void* context);

SubRemViewRemote* subrem_view_remote_alloc();

void subrem_view_remote_free(SubRemViewRemote* subrem_view_remote);

View* subrem_view_remote_get_view(SubRemViewRemote* subrem_view_remote);

void subrem_view_remote_update_data_labels(
    SubRemViewRemote* subrem_view_remote,
    SubRemSubFilePreset** subs_presets);

void subrem_view_remote_set_state(
    SubRemViewRemote* subrem_view_remote,
    SubRemViewRemoteState state,
    uint8_t presed_btn);

void subrem_view_remote_set_radio(SubRemViewRemote* subrem_view_remote, bool external);