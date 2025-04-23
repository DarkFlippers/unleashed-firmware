/**
 * Displays a dialog with up to three options.
 * 
 * <img src="../images/dialog.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let dialogView = require("gui/dialog");
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
 *   - `header`: Text displayed in bold at the top of the screen
 *   - `text`: Text displayed in the middle of the string
 *   - `left`: Text for the left button
 *   - `center`: Text for the center button
 *   - `right`: Text for the right button
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    text: string,
    left: string,
    center: string,
    right: string,
}
type Child = never;
declare class Dialog extends View<Props, Child> {
    input: Contract<"left" | "center" | "right">;
}
declare class DialogFactory extends ViewFactory<Props, Child, Dialog> { }
declare const factory: DialogFactory;
export = factory;
