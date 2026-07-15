/**
 * Displays a number input keyboard.
 * 
 * <img src="../images/number_input.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let numberInputView = require("gui/number_input");
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
 *   - `header`: Text displayed at the top of the screen
 *   - `minValue`: Minimum allowed numeric value
 *   - `maxValue`: Maximum allowed numeric value
 *   - `defaultValue`: Default numeric value
 * 
 * @version Added in JS SDK 0.4
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    minValue: number,
    maxValue: number,
    defaultValue: number,
}
type Child = never;
declare class NumberInput extends View<Props, Child> {
    input: Contract<number>;
}
declare class NumberInputFactory extends ViewFactory<Props, Child, NumberInput> { }
declare const factory: NumberInputFactory;
export = factory;
