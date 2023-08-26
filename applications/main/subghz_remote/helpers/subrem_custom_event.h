#pragma once

typedef enum {
    SubRemEditMenuStateUP = 0,
    SubRemEditMenuStateDOWN,
    SubRemEditMenuStateLEFT,
    SubRemEditMenuStateRIGHT,
    SubRemEditMenuStateOK,
} SubRemEditMenuState;

typedef enum {
    // StartSubmenuIndex
    SubmenuIndexSubRemOpenMapFile = 0,
    SubmenuIndexSubRemEditMapFile,
    SubmenuIndexSubRemNewMapFile,
#if FURI_DEBUG
    SubmenuIndexSubRemRemoteView,
#endif
    // SubmenuIndexSubRemAbout,

    // EditSubmenuIndex
    EditSubmenuIndexEditLabel,
    EditSubmenuIndexEditFile,

    // SubRemCustomEvent
    SubRemCustomEventViewRemoteStartUP = 100,
    SubRemCustomEventViewRemoteStartDOWN,
    SubRemCustomEventViewRemoteStartLEFT,
    SubRemCustomEventViewRemoteStartRIGHT,
    SubRemCustomEventViewRemoteStartOK,
    SubRemCustomEventViewRemoteBack,
    SubRemCustomEventViewRemoteStop,
    SubRemCustomEventViewRemoteForcedStop,

    SubRemCustomEventViewEditMenuBack,
    SubRemCustomEventViewEditMenuUP,
    SubRemCustomEventViewEditMenuDOWN,
    SubRemCustomEventViewEditMenuEdit,
    SubRemCustomEventViewEditMenuSave,

    SubRemCustomEventSceneEditsubmenu,
    SubRemCustomEventSceneEditLabelInputDone,
    SubRemCustomEventSceneEditLabelWidgetAcces,
    SubRemCustomEventSceneEditLabelWidgetBack,

    SubRemCustomEventSceneEditOpenSubErrorPopup,

    SubRemCustomEventSceneEditPreviewSaved,

    SubRemCustomEventSceneNewName,
} SubRemCustomEvent;