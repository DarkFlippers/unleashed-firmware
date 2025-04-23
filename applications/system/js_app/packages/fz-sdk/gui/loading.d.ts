/**
 * Displays an animated hourglass icon. Suppresses all `navigation` events,
 * making it impossible for the user to exit the view by pressing the back key.
 * 
 * <img src="../images/loading.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let loadingView = require("gui/loading");
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
declare class Loading extends View<Props, Child> { }
declare class LoadingFactory extends ViewFactory<Props, Child, Loading> { }
declare const factory: LoadingFactory;
export = factory;
