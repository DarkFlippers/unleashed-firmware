/**
 * Displays a combination of custom elements on one screen.
 * 
 * <img src="../images/widget.png" width="200" alt="Sample screenshot of the view" />
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gui = require("gui");
 * let emptyView = require("gui/widget");
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
 * # Children
 * This view has the elements as its children.
 * 
 * @version Added in JS SDK 0.2, extra feature `"gui-widget"`
 * @module
 */

import type { View, ViewFactory } from ".";
import type { IconData } from "./icon";
import type { Contract } from "../event_loop";

type Position = { x: number, y: number };
type Size = { w: number, h: number };
type Alignment = { align: `${"t" | "c" | "b"}${"l" | "m" | "r"}` };
type Font = { font: "primary" | "secondary" | "keyboard" | "big_numbers" };
type Text = { text: string };

type StringMultilineElement = { element: "string_multiline" } & Position & Alignment & Font & Text;
type StringElement = { element: "string" } & Position & Alignment & Font & Text;
type TextBoxElement = { element: "text_box", stripToDots: boolean } & Position & Size & Alignment & Text;
type TextScrollElement = { element: "text_scroll" } & Position & Size & Text;
type ButtonElement = { element: "button", button: "left" | "center" | "right" } & Text;
type IconElement = { element: "icon", iconData: IconData } & Position;
type RectElement = { element: "rect", radius: number, fill: boolean } & Position & Size; /** @version Amended in JS SDK 0.3, extra feature `"gui-widget-extras"` */
type CircleElement = { element: "circle", radius: number, fill: boolean } & Position; /** @version Added in JS SDK 0.3, extra feature `"gui-widget-extras"` */
type LineElement = { element: "line", x1: number, y1: number, x2: number, y2: number }; /** @version Added in JS SDK 0.3, extra feature `"gui-widget-extras"` */

type Element = StringMultilineElement
    | StringElement
    | TextBoxElement
    | TextScrollElement
    | ButtonElement
    | IconElement
    | RectElement
    | CircleElement
    | LineElement;

type Props = {};
type Child = Element;
declare class Widget extends View<Props, Child> {
    /**
     * Event source for buttons. Only gets fired if there's a corresponding
     * button element.
     */
    button: Contract<"left" | "center" | "right">;
}
declare class WidgetFactory extends ViewFactory<Props, Child, Widget> { }
declare const factory: WidgetFactory;
export = factory;
