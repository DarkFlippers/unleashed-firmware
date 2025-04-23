/**
 * Displays nothing.
 * 
 * <img src="../images/empty.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let emptyView = require("gui/empty_screen");
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
 * This view does not have any props.
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { View, ViewFactory } from ".";

type Props = {};
type Child = never;
declare class EmptyScreen extends View<Props, Child> { }
declare class EmptyScreenFactory extends ViewFactory<Props, Child, EmptyScreen> { }
declare const factory: EmptyScreenFactory;
export = factory;
