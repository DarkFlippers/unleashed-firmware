/**
 * A list of selectable entries consisting of an icon and a label.
 * 
 * <img src="../images/menu.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let submenuView = require("gui/menu");
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
 * This view doesn't have any props.
 * 
 * @version Added in JS SDK 0.1
 * @version API changed in JS SDK 0.4
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";
import type { IconData } from "./icon";

type Props = {};
type Child = { icon: IconData, label: string };
declare class Submenu extends View<Props, Child> {
    chosen: Contract<number>;
}
declare class SubmenuFactory extends ViewFactory<Props, Child, Submenu> { }
declare const factory: SubmenuFactory;
export = factory;
