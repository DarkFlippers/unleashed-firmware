#include "../t5577_app.h"

/** A list line: a kind, a block and the one word worth reading off that frame. */
#define T5577_SNIFF_LABEL_MAX 24

static void t5577_app_scene_sniff_frames_callback(void* context, uint32_t index) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

// The password where there is one, because that is the word worth trying, and the data where
// there is not. An ambiguous frame has both and shows its password reading here; the pair is
// spelled out on the frame's own screen.
static void t5577_app_scene_sniff_frames_label(const T5577SniffFrame* frame, char* label) {
    switch(frame->kind) {
    case T5577SniffFrameReset:
        strlcpy(label, "Reset", T5577_SNIFF_LABEL_MAX);
        return;
    case T5577SniffFrameRegularRead:
        strlcpy(label, "Back to read mode", T5577_SNIFF_LABEL_MAX);
        return;
    case T5577SniffFramePlainRead:
        snprintf(label, T5577_SNIFF_LABEL_MAX, "Read b%u", frame->block);
        return;
    case T5577SniffFramePlainWrite:
        snprintf(
            label,
            T5577_SNIFF_LABEL_MAX,
            "Write b%u %08lX",
            frame->block,
            (unsigned long)frame->data);
        return;
    case T5577SniffFramePasswordedRead:
        snprintf(
            label,
            T5577_SNIFF_LABEL_MAX,
            "Read pw b%u %08lX",
            frame->block,
            (unsigned long)frame->password);
        return;
    case T5577SniffFramePasswordedWrite:
        snprintf(
            label,
            T5577_SNIFF_LABEL_MAX,
            "Write pw b%u %08lX",
            frame->block,
            (unsigned long)frame->password);
        return;
    case T5577SniffFrameAmbiguous:
        snprintf(
            label,
            T5577_SNIFF_LABEL_MAX,
            "? b%u %08lX",
            frame->block,
            (unsigned long)frame->password);
        return;
    case T5577SniffFrameUnknown:
        // The length is the whole of what is known about it, and it is the number that says
        // whether the framing is wrong or the sender is using a mode this cannot read. The
        // count stops at the store's cap, so at the cap it is a floor and says so.
        snprintf(
            label,
            T5577_SNIFF_LABEL_MAX,
            "%u%s symbols",
            frame->symbols,
            frame->symbols >= T5577_SNIFF_SYMBOLS_MAX ? "+" : "");
        return;
    }

    strlcpy(label, "?", T5577_SNIFF_LABEL_MAX);
}

void t5577_app_scene_sniff_frames_on_enter(void* context) {
    T5577App* app = context;

    const T5577SniffResult* result = &app->sniff_result;

    submenu_reset(app->submenu);

    // How much of the list is worth reading. A sender idles either side of whatever it came
    // to do - a cloner's write run was twelve frames of forty-six - so a bare count reads as a
    // wall of findings when most of it is the coil listening to a quiet room. The dropped count
    // belongs here too: the list silently stops at T5577_SNIFF_FRAMES_MAX, and a short list is
    // otherwise indistinguishable from a short capture.
    uint8_t decoded = 0;

    for(uint8_t i = 0; i < result->frames; i++) {
        if(result->frame[i].kind != T5577SniffFrameUnknown) decoded++;
    }

    // Its own size: T5577_SNIFF_LABEL_MAX documents a list line, and this is not one.
    char header[24];

    if(result->dropped > 0) {
        snprintf(
            header,
            sizeof(header),
            "%u/%u read +%u lost",
            decoded,
            result->frames,
            result->dropped);
    } else {
        snprintf(header, sizeof(header), "%u of %u decoded", decoded, result->frames);
    }

    submenu_set_header(app->submenu, header);

    for(uint8_t i = 0; i < result->frames; i++) {
        char label[T5577_SNIFF_LABEL_MAX];

        t5577_app_scene_sniff_frames_label(&result->frame[i], label);
        submenu_add_item(app->submenu, label, i, t5577_app_scene_sniff_frames_callback, app);
    }

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, T5577AppSceneSniffFrames));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewSubmenu);
}

bool t5577_app_scene_sniff_frames_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    // Only this list's own indices get here as a choice, and a frame that is not in the result
    // is not one of them.
    if(event.event >= app->sniff_result.frames) return false;

    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneSniffFrames, event.event);
    app->sniff_pick = (uint8_t)event.event;
    scene_manager_next_scene(app->scene_manager, T5577AppSceneSniffFrame);

    return true;
}

void t5577_app_scene_sniff_frames_on_exit(void* context) {
    T5577App* app = context;
    submenu_reset(app->submenu);
}
