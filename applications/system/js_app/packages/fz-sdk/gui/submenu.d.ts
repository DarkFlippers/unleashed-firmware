/**
 * Displays a scrollable list of clickable textual entries.
 * 
 * <img src="../images/submenu.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let submenuView = require("gui/submenu");
 * ```
 * 
 * This module depends on the `gui` module, which in turn depends on the
 * `event_loop` module, so they _must_ be imported in this order. It is also
 * recommended to conceptualize these modules first before using this one.
 * 
 * # Example
 * For an example refer to the GUI example.
 * 
 * # View props
 *   - `header`: Text displayed at the top of the screen in bold
 *   - `items`: Array of selectable textual items
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    items: string[],
};
type Child = never;
declare class Submenu extends View<Props, Child> {
    chosen: Contract<number>;
}
declare class SubmenuFactory extends ViewFactory<Props, Child, Submenu> { }
declare const factory: SubmenuFactory;
export = factory;
