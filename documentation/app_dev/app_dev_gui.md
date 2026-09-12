# GUI in Apps {#app_dev_gui}

This page introduces basic GUI concepts for Flipper Zero development and reviews existing GUI application approaches with their pros and cons:

- [Dialogs API](#app_dev_gui_dialogs)

- [ViewPort](#app_dev_gui_viewport)

- [Views & Scenes](#app_dev_gui_views_scenes)

# Basic GUI Concepts

## EventLoop

An asynchronous event handling mechanism that eliminates the need for polling or waiting for events (e.g., button presses, packet reception). Instead, event handler functions are assigned and invoked upon event occurrence, allowing the main loop to perform other tasks concurrently.

## Canvas

The **canvas** is just a raw drawing area with no abstractions over it. Drawing on the canvas directly is useful for implementing custom design elements (e.g., in games), but this is rather uncommon.

## ViewPort

**ViewPort** defines the behavior and rendering  of a screen area with specific position and size.

Multiple ViewPorts form a Drawing Stack, rendered bottom to top. This allows ViewPorts to overlap each other.

Displaying a specific ViewPort can be enabled or disabled after its creation.

## Model, View, and View Dispatcher

The **Model** and **View** concepts help separate GUI from data, simplifying development, maintenance and testing.

The **Model** contains the data; technically, it is a structure in memory.

The **View** implements the logic for interface rendering and event handling. The View uses the Model: it retrieves data from the Model for display and modifies the data when processing events.

To switch between multiple views, there is a separate entity — the **View Dispatcher**, which holds references to all the views that an application needs and switches between them as requested by the application.

## View Modules

View modules are fullscreen templates (View+Model) included in the firmware for typical Flipper Zero UIs. Available view modules:

- `button_menu`
- `button_panel`
- `byte_input`
- `dialog_ex`
- `empty_screen`
- `file_browser`
- `loading`
- `menu`
- `number_input`
- `popup`
- `submenu`
- `text_box`
- `text_input`
- `variable_item_list`
- `widget`

View module source code: https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/services/gui/modules

## Scene & Scene Manager

A **Scene** manages the behavior of an application within a single screen. For example, the [standard GPIO application](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/main/gpio) include the following screens:

- `gpio_scene_start` (start menu)
- `gpio_scene_usb_uart` (USB-UART bridge mode interface)
- `gpio_scene_usb_uart_cfg` (USB-UART bridge mode settings interface)
- `gpio_scene_usb_uart_close` (confirmation dialog for exiting USB-UART bridge)
- `gpio_scene_test` (GPIO manual control)

From the main menu, you can navigate to the manual control scene or enter USB-UART bridge mode. Within USB-UART bridge mode, you can access mode settings or exit the mode via a confirmation dialog.

Each scene implementation includes:

- `on_enter` function: used to initialize the scene’s views.
- `on_exit` function: used to de-initialize the scene and free resources.
- `on_event` function: handles incoming external events.

An external event handler can trigger a UI switch to another scene. For example, selecting an item in the main menu triggers a transition to the corresponding scene via the **Scene Manager**, which enables switching between scenes and simplifies navigation in applications with complex navigation flows.

---

# Approaches to GUI Development

In this section, we will review existing GUI development approaches, starting from the simplest and progressing to more complex ones. The complexity of the interface you want to create determines the number of GUI abstractions required.

## Using Dialogs {#app_dev_gui_dialogs}

The simplest approach to building a GUI application. It is suitable when the entire interface can be implemented using only dialogs.

You can find the Dialog API functions in the header file [dialogs.h](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/services/dialogs/dialogs.h).

\image html app_dev_gui_dialogs.jpg width=700

**Pros:**

- Simple and easy to use

**Cons:**

- Cannot build complex GUIs (only Dialogs are available)
- Dialogs operate synchronously, meaning that until the user presses a button in the dialog, no other application code will execute

### **Example**

```c
#include <furi.h>
#include <dialogs/dialogs.h>
#include <test_app_icons.h>

int32_t test_app(void* p) {
    UNUSED(p);

    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Hello world", 64, 20, AlignCenter, AlignTop);
    dialog_message_set_text(message, "I'm test_app!", 64, 32, AlignCenter, AlignTop);
    dialog_message_show(dialogs, message);
    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);

    return 0;
}
```

## Using ViewPort {#app_dev_gui_viewport}

This approach enables drawing on the screen and handling all button input. A single fullscreen ViewPort is created to render the display and process all events (e.g., button presses).

\image html app_dev_gui_viewport.png width=700

**Pros:**

- Easy: uses only one ViewPort instance without more complex GUI abstractions
- Fast and game-friendly
- Full control over what happens on the screen

**Cons:**

- No navigation helpers
- No reusable Views

### Examples

- Many of the [debug tools](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/debug)
- [BLE pairing dialog](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/services/bt/bt_service/bt.c)
- [Dolphin Passport](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/settings/dolphin_passport/passport.c)
- Most of the games

## Using Views and Scenes {#app_dev_gui_views_scenes}

This is the main approach for developing applications with complex, hierarchical UIs. Most Flipper Zero applications use this method.

\image html app_dev_gui_views_and_scenes.jpg width=700

**Pros:**

- Supports complex navigation
- View recycling
- A lot of reusable view modules available

**Cons:**

- Takes time to sink in
- Complex implementation

### Examples

- [Flipper Zero Desktop](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/services/desktop)
- [iButton app](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/main/ibutton)
- [Infrared app](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/main/infrared)
- [GPIO app](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/main/gpio)
- [NFC app](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/main/nfc)
- [RFID app](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/main/lfrfid)
- [Sub-GHz app](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/main/subghz)

# Learn more (external resources)

- [A Visual Guide to Flipper Zero GUI Modules by Christopher Hranj](https://brodan.biz/blog/a-visual-guide-to-flipper-zero-gui-components/)
- [Scenes Demo Application Tutorial by Derek Jamison](https://github.com/jamisonderek/flipper-zero-tutorials/tree/main/ui/basic_scenes#scenes-demo-application-tutorial)
- YouTube tutorials by Derek Jamison:
    - [Using ViewPort for applications](https://www.youtube.com/watch?v=5QwwihbxkC8)
    - [Creating UI using Scenes](https://www.youtube.com/watch?v=YbskaB6caqk&list=PLM1cyTMe-PYJaMQ6TWeK1mAWxORdjYJZ5&index=8)
    - [SceneManager Best Practice](https://www.youtube.com/watch?v=vQAk7fC_no4)