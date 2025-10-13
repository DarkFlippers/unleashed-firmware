/**
 * Like a Dialog, but with a built-in timer.
 * 
 * <img src="../images/popup.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let popupView = require("gui/popup");
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
 *   - `timeout`: Timeout, in milliseconds, after which the event will fire. The
 *     timer starts counting down when this property is assigned.
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    text: string,
    timeout: number,
}
type Child = never;
declare class Popup extends View<Props, Child> {
    timeout: Contract;
}
declare class PopupFactory extends ViewFactory<Props, Child, Popup> { }
declare const factory: PopupFactory;
export = factory;
