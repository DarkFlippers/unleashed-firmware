/**
 * Displays a byte input keyboard.
 * 
 * <img src="../images/byte_input.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let byteInputView = require("gui/byte_input");
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
 *   - `length`: Length of data to edit
 *   - `defaultData`: Data to show by default
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    length: number,
    defaultData: Uint8Array | ArrayBuffer,
}
type Child = never;
declare class ByteInput extends View<Props, Child> {
    input: Contract<string>;
}
declare class ByteInputFactory extends ViewFactory<Props, Child, ByteInput> { }
declare const factory: ByteInputFactory;
export = factory;
