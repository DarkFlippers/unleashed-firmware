/**
 * Displays a list of settings-like variable items.
 * 
 * <img src="../images/vi_list.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let viListView = require("gui/vi_list");
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
 * This view doesn't have any props
 * 
 * @version Added in JS SDK 0.4
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {};

type Child = { label: string, variants: string[] };

declare class ViList extends View<Props, Child> {
    valueUpdate: Contract<{ itemIndex: number, valueIndex: number }>;
}
declare class ViListFactory extends ViewFactory<Props, Child, ViList> { }
declare const factory: ViListFactory;
export = factory;
