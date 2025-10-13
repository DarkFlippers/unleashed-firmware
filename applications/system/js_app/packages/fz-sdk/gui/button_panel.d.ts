/**
 * Displays a button matrix.
 * 
 * <img src="../images/button_panel.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let buttonPanelView = require("gui/button_panel");
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
 *   - `matrixSizeX`: Width of imaginary grid used for navigation
 *   - `matrixSizeY`: Height of imaginary grid used for navigation
 * 
 * @version Added in JS SDK 0.4
 * @module
 */

import type { View, ViewFactory, Font, InputType } from ".";
import type { Contract } from "../event_loop";
import { IconData } from "./icon";

type Props = {
    matrixSizeX: number,
    matrixSizeY: number,
};

type Position = { x: number, y: number };

type ButtonChild = { type: "button", matrixX: number, matrixY: number, icon: IconData, iconSelected: IconData } & Position;
type LabelChild = { type: "label", font: Font, text: string } & Position;
type IconChild = { type: "icon", icon: IconData };

type Child = ButtonChild | LabelChild | IconChild;

declare class ButtonPanel extends View<Props, Child> {
    input: Contract<{ index: number, type: InputType }>;
}
declare class ButtonPanelFactory extends ViewFactory<Props, Child, ButtonPanel> { }
declare const factory: ButtonPanelFactory;
export = factory;
