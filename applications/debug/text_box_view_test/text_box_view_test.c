#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_box.h>
#include <gui/view_stack.h>

#define TAG "TextBoxViewTest"

typedef struct {
    TextBoxFont font;
    TextBoxFocus focus;
    const char* text;
} TextBoxViewTestContent;

static const TextBoxViewTestContent text_box_view_test_content_arr[] = {
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusStart,
        .text = "Hello, let's test text box. Press Right and Left to switch content",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusEnd,
        .text = "First test to add dynamically lines with EndFocus set\nLine 0",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusEnd,
        .text = "First test to add dynamically lines with EndFocus set\nLine 0\nLine 1",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusEnd,
        .text = "First test to add dynamically lines with EndFocus set\nLine 0\nLine 1\nLine 2",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusEnd,
        .text =
            "First test to add dynamically lines with EndFocus set\nLine 0\nLine 1\nLine 2\nLine 3",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusEnd,
        .text =
            "First test to add dynamically lines with EndFocus set\nLine 0\nLine 1\nLine 2\nLine 3\nLine 4",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusStart,
        .text =
            "Verify that symbols don't overlap borders: llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllend",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusStart,
        .text =
            "\n\n\n Start from several newline chars. Verify that scrolling doesn't break.\n\n\n\n\nThe end",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusStart,
        .text =
            "Let's test big text.\n\n The ARM Cortex-M is a group of 32-bit RISC ARM processor cores licensed by ARM Limited. These cores are optimized for low-cost and energy-efficient integrated circuits, which have been embedded in tens of billions of consumer devices.[1] Though they are most often the main component of microcontroller chips, sometimes they are embedded inside other types of chips too. The Cortex-M family consists of Cortex-M0,[2] Cortex-M0+,[3] Cortex-M1,[4] Cortex-M3,[5] Cortex-M4,[6] Cortex-M7,[7] Cortex-M23,[8] Cortex-M33,[9] Cortex-M35P,[10] Cortex-M52,[11] Cortex-M55,[12] Cortex-M85.[13] A floating-point unit (FPU) option is available for Cortex-M4 / M7 / M33 / M35P / M52 / M55 / M85 cores, and when included in the silicon these cores are sometimes known as \"Cortex-MxF\", where 'x' is the core variant.\n\nThe ARM Cortex-M family are ARM microprocessor cores that are designed for use in microcontrollers, ASICs, ASSPs, FPGAs, and SoCs. Cortex-M cores are commonly used as dedicated microcontroller chips, but also are hidden inside of SoC chips as power management controllers, I/O controllers, system controllers, touch screen controllers, smart battery controllers, and sensor controllers. The main difference from Cortex-A cores is that Cortex-M cores have no memory management unit (MMU) for virtual memory, considered essential for full-fledged operating systems. Cortex-M programs instead run bare metal or on one of the many real-time operating systems which support a Cortex-M.Though 8-bit microcontrollers were very popular in the past, Cortex-M has slowly been chipping away at the 8-bit market as the prices of low-end Cortex-M chips have moved downward. Cortex-M have become a popular replacements for 8-bit chips in applications that benefit from 32-bit math operations, and replacing older legacy ARM cores such as ARM7 and ARM9.",
    },
    {
        .font = TextBoxFontText,
        .focus = TextBoxFocusEnd,
        .text =
            "The same but with EndFocus\n\n The ARM Cortex-M is a group of 32-bit RISC ARM processor cores licensed by ARM Limited. These cores are optimized for low-cost and energy-efficient integrated circuits, which have been embedded in tens of billions of consumer devices.[1] Though they are most often the main component of microcontroller chips, sometimes they are embedded inside other types of chips too. The Cortex-M family consists of Cortex-M0,[2] Cortex-M0+,[3] Cortex-M1,[4] Cortex-M3,[5] Cortex-M4,[6] Cortex-M7,[7] Cortex-M23,[8] Cortex-M33,[9] Cortex-M35P,[10] Cortex-M52,[11] Cortex-M55,[12] Cortex-M85.[13] A floating-point unit (FPU) option is available for Cortex-M4 / M7 / M33 / M35P / M52 / M55 / M85 cores, and when included in the silicon these cores are sometimes known as \"Cortex-MxF\", where 'x' is the core variant.\n\nThe ARM Cortex-M family are ARM microprocessor cores that are designed for use in microcontrollers, ASICs, ASSPs, FPGAs, and SoCs. Cortex-M cores are commonly used as dedicated microcontroller chips, but also are hidden inside of SoC chips as power management controllers, I/O controllers, system controllers, touch screen controllers, smart battery controllers, and sensor controllers. The main difference from Cortex-A cores is that Cortex-M cores have no memory management unit (MMU) for virtual memory, considered essential for full-fledged operating systems. Cortex-M programs instead run bare metal or on one of the many real-time operating systems which support a Cortex-M.Though 8-bit microcontrollers were very popular in the past, Cortex-M has slowly been chipping away at the 8-bit market as the prices of low-end Cortex-M chips have moved downward. Cortex-M have become a popular replacements for 8-bit chips in applications that benefit from 32-bit math operations, and replacing older legacy ARM cores such as ARM7 and ARM9.",
    },
    {
        .font = TextBoxFontHex,
        .focus = TextBoxFocusEnd,
        .text =
            "0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999\n0000 0000 0000 0000\n1111 1111 1111 1111\n2222 2222 2222 2222\n3333 3333 3333 3333\n4444 4444 4444 4444\n5555 5555 5555 5555\n6666 6666 6666 6666\n7777 7777 7777 7777\n8888 8888 8888 8888\n9999 9999 9999 9999",
    },
};

typedef struct {
    TextBox* text_box;
    ViewDispatcher* view_dispatcher;
    size_t current_content_i;
} TextBoxViewTest;

static void text_box_update_view(TextBoxViewTest* instance) {
    // Intentional incorrect way to reset text box to verify that state resets if text changes
    text_box_set_text(instance->text_box, "");

    const TextBoxViewTestContent* content =
        &text_box_view_test_content_arr[instance->current_content_i];
    text_box_set_font(instance->text_box, content->font);
    text_box_set_focus(instance->text_box, content->focus);
    text_box_set_text(instance->text_box, content->text);
}

static bool text_box_switch_view_input_callback(InputEvent* event, void* context) {
    bool consumed = false;
    TextBoxViewTest* instance = context;
    size_t contents_cnt = COUNT_OF(text_box_view_test_content_arr);

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight) {
            if(instance->current_content_i < contents_cnt - 1) {
                instance->current_content_i++;
                text_box_update_view(instance);
                consumed = true;
            }
        } else if(event->key == InputKeyLeft) {
            if(instance->current_content_i > 0) {
                instance->current_content_i--;
                text_box_update_view(instance);
                consumed = true;
            }
        } else if(event->key == InputKeyBack) {
            view_dispatcher_stop(instance->view_dispatcher);
        }
    }

    return consumed;
}

int32_t text_box_view_test_app(void* p) {
    UNUSED(p);

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewDispatcher* view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    TextBoxViewTest instance = {
        .text_box = text_box_alloc(),
        .current_content_i = 0,
        .view_dispatcher = view_dispatcher,
    };

    text_box_update_view(&instance);

    View* text_box_switch_view = view_alloc();
    view_set_input_callback(text_box_switch_view, text_box_switch_view_input_callback);
    view_set_context(text_box_switch_view, &instance);

    ViewStack* view_stack = view_stack_alloc();
    view_stack_add_view(view_stack, text_box_switch_view);
    view_stack_add_view(view_stack, text_box_get_view(instance.text_box));

    view_dispatcher_add_view(view_dispatcher, 0, view_stack_get_view(view_stack));
    view_dispatcher_switch_to_view(view_dispatcher, 0);

    view_dispatcher_run(view_dispatcher);

    view_dispatcher_remove_view(view_dispatcher, 0);
    view_dispatcher_free(view_dispatcher);
    view_stack_free(view_stack);
    view_free(text_box_switch_view);
    text_box_free(instance.text_box);

    furi_record_close(RECORD_GUI);

    return 0;
}
