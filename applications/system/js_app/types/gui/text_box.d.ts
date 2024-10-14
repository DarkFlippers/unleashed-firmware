import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    text: string,
    font: "text" | "hex",
    focus: "start" | "end",
}
declare class TextBox extends View<Props> {
    chosen: Contract<number>;
}
declare class TextBoxFactory extends ViewFactory<Props, TextBox> { }
declare const factory: TextBoxFactory;
export = factory;
