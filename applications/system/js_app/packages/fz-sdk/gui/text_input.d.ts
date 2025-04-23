/**
 * Displays a text input keyboard.
 * 
 * <img src="../images/text_input.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let textInputView = require("gui/text_input");
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
 *   - `minLength`: Minimum allowed text length
 *   - `maxLength`: Maximum allowed text length
 *   - `defaultText`: Text to show by default
 *   - `defaultTextClear`: Whether to clear the default text on next character typed
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    minLength: number,
    maxLength: number,
    defaultText: string,
    defaultTextClear: boolean,
}
type Child = never;
declare class TextInput extends View<Props, Child> {
    input: Contract<string>;
}
declare class TextInputFactory extends ViewFactory<Props, Child, TextInput> { }
declare const factory: TextInputFactory;
export = factory;
