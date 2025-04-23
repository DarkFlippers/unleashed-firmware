/**
 * Displays a scrollable read-only text field.
 *
 * <img src="text_box.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let textBoxView = require("gui/text_box");
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
 *   - `text`: Text in the text box
 *   - `font`: The font to display the text in (`"text"` or `"hex"`)
 *   - `focus`: The initial focus of the text box (`"start"` or `"end"`)
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    text: string,
    font: "text" | "hex",
    focus: "start" | "end",
}
type Child = never;
declare class TextBox extends View<Props, Child> {
    chosen: Contract<number>;
}
declare class TextBoxFactory extends ViewFactory<Props, Child, TextBox> { }
declare const factory: TextBoxFactory;
export = factory;
