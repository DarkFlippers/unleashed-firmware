/**
 * Displays a list of buttons.
 * 
 * <img src="../images/button_menu.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let buttonMenuView = require("gui/button_menu");
 * ```
 * 
 * This module depends on the `gui` module, which in turn depends on the
 * `event_loop` module, so they _must_ be imported in this order. It is also
 * recommended to conceptualize these modules first before using this one.
 * 
 * # Example
 * For an example refer to the `gui.js` example script.
 * 
 * # View props
 *   - `header`: Textual header above the buttons
 * 
 * @version Added in JS SDK 0.4
 * @module
 */

import type { View, ViewFactory, InputType } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string;
};

type Child = { type: "common" | "control", label: string };

declare class ButtonMenu extends View<Props, Child> {
    input: Contract<{ index: number, type: InputType }>;
}
declare class ButtonMenuFactory extends ViewFactory<Props, Child, ButtonMenu> { }
declare const factory: ButtonMenuFactory;
export = factory;
