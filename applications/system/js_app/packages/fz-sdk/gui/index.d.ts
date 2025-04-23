/**
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * ```
 * 
 * This module depends on the `event_loop` module, so it _must_ only be imported
 * after `event_loop` is imported.
 * 
 * ## Conceptualizing GUI
 * ### Event loop
 * It is highly recommended to familiarize yourself with the event loop first
 * before doing GUI-related things.
 * 
 * ### Canvas
 * The canvas is just a drawing area with no abstractions over it. Drawing on
 * the canvas directly (i.e. not through a viewport) is useful in case you want
 * to implement a custom design element, but this is rather uncommon.
 * 
 * ### Viewport
 * A viewport is a window into a rectangular portion of the canvas. Applications
 * always access the canvas through a viewport.
 * 
 * ### View
 * In Flipper's terminology, a "View" is a fullscreen design element that
 * assumes control over the entire viewport and all input events. Different
 * types of views are available (not all of which are unfortunately currently
 * implemented in JS):
 * | View                 | Has JS adapter?       |
 * |----------------------|-----------------------|
 * | `button_menu`        | ❌                    |
 * | `button_panel`       | ❌                    |
 * | `byte_input`         | ✅                    |
 * | `dialog_ex`          | ✅ (as `dialog`)      |
 * | `empty_screen`       | ✅                    |
 * | `file_browser`       | ✅ (as `file_picker`) |
 * | `loading`            | ✅                    |
 * | `menu`               | ❌                    |
 * | `number_input`       | ❌                    |
 * | `popup`              | ❌                    |
 * | `submenu`            | ✅                    |
 * | `text_box`           | ✅                    |
 * | `text_input`         | ✅                    |
 * | `variable_item_list` | ❌                    |
 * | `widget`             | ✅                    |
 * 
 * In JS, each view has its own set of properties (or just "props"). The
 * programmer can manipulate these properties in two ways:
 *   - Instantiate a `View` using the `makeWith(props)` method, passing an
 *     object with the initial properties
 *   - Call `set(name, value)` to modify a property of an existing `View`
 * 
 * ### View Dispatcher
 * The view dispatcher holds references to all the views that an application
 * needs and switches between them as the application makes requests to do so.
 * 
 * ### Scene Manager
 * The scene manager is an optional add-on to the view dispatcher that makes
 * managing applications with complex navigation flows easier. It is currently
 * inaccessible from JS.
 * 
 * ### Approaches
 * In total, there are three different approaches that you may take when writing
 * a GUI application:
 * | Approach       | Use cases                                                                    | Available from JS |
 * |----------------|------------------------------------------------------------------------------|-------------------|
 * | ViewPort only  | Accessing the graphics API directly, without any of the nice UI abstractions | ❌                |
 * | ViewDispatcher | Common UI elements that fit with the overall look of the system              | ✅                |
 * | SceneManager   | Additional navigation flow management for complex applications               | ❌                |
 * 
 * # Example
 * An example with three different views using the ViewDispatcher approach:
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let loadingView = require("gui/loading");
 * let submenuView = require("gui/submenu");
 * let emptyView = require("gui/empty_screen");
 * 
 * // Common pattern: declare all the views in an object. This is absolutely not
 * // required, but adds clarity to the script.
 * let views = {
 *     // the view dispatcher auto-✨magically✨ remembers views as they are created
 *     loading: loadingView.make(),
 *     empty: emptyView.make(),
 *     demos: submenuView.makeWith({
 *         items: [
 *             "Hourglass screen",
 *             "Empty screen",
 *             "Exit app",
 *         ],
 *     }),
 * };
 * 
 * // go to different screens depending on what was selected
 * eventLoop.subscribe(views.demos.chosen, function (_sub, index, gui, eventLoop, views) {
 *     if (index === 0) {
 *         gui.viewDispatcher.switchTo(views.loading);
 *     } else if (index === 1) {
 *         gui.viewDispatcher.switchTo(views.empty);
 *     } else if (index === 2) {
 *         eventLoop.stop();
 *     }
 * }, gui, eventLoop, views);
 * 
 * // go to the demo chooser screen when the back key is pressed
 * eventLoop.subscribe(gui.viewDispatcher.navigation, function (_sub, _, gui, views) {
 *     gui.viewDispatcher.switchTo(views.demos);
 * }, gui, views);
 * 
 * // run UI
 * gui.viewDispatcher.switchTo(views.demos);
 * eventLoop.run();
 * ```
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { Contract } from "../event_loop";

type Properties = { [K: string]: any };

export declare class View<Props extends Properties, Child> {
    /**
     * Assign value to property by name
     * @param property Name of the property
     * @param value Value to assign
     * @version Added in JS SDK 0.1
     */
    set<P extends keyof Props>(property: P, value: Props[P]): void;
    /**
     * Adds a child to the View
     * @param child Child to add
     * @version Added in JS SDK 0.2, extra feature `"gui-widget"`
     */
    addChild<C extends Child>(child: C): void;
    /**
     * Removes all children from the View
     * @version Added in JS SDK 0.2, extra feature `"gui-widget"`
     */
    resetChildren(): void;
    /**
     * Removes all previous children from the View and assigns new children
     * @param children The list of children to assign
     * @version Added in JS SDK 0.2, extra feature `"gui-widget"`
     */
    setChildren(children: Child[]): void;
}

export declare class ViewFactory<Props extends Properties, Child, V extends View<Props, Child>> {
    /**
     * Create view instance with default values, can be changed later with set()
     * @version Added in JS SDK 0.1
     */
    make(): V;
    /**
     * Create view instance with custom values, can be changed later with set()
     * @param initial Dictionary of property names to values
     * @param children Optional list of children to add to the view
     * @version Added in JS SDK 0.1; amended in JS SDK 0.2, extra feature `"gui-widget"`
     */
    makeWith(initial: Partial<Props>, children?: Child[]): V;
}

/**
 * @version Added in JS SDK 0.1
 */
declare class ViewDispatcher {
    /**
     * Event source for `sendCustom` events
     * @version Added in JS SDK 0.1
     */
    custom: Contract<number>;
    /**
     * Event source for navigation events (back key presses)
     * @version Added in JS SDK 0.1
     */
    navigation: Contract;
    /**
     * View object currently shown
     * @version Added in JS SDK 0.1
     */
    currentView: View<any, any>;
    /**
     * Sends a number to the custom event handler
     * @param event number to send
     * @version Added in JS SDK 0.1
     */
    sendCustom(event: number): void;
    /**
     * Switches to a view
     * @param assoc View-ViewDispatcher association as returned by `add`
     * @version Added in JS SDK 0.1
     */
    switchTo(assoc: View<any, any>): void;
    /**
     * Sends this ViewDispatcher to the front or back, above or below all other
     * GUI viewports
     * @param direction Either `"front"` or `"back"`
     * @version Added in JS SDK 0.1
     */
    sendTo(direction: "front" | "back"): void;
}

/**
 * @version Added in JS SDK 0.1
 */
export const viewDispatcher: ViewDispatcher;
