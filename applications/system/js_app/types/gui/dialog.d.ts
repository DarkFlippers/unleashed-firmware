import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    text: string,
    left: string,
    center: string,
    right: string,
}
declare class Dialog extends View<Props> {
    input: Contract<"left" | "center" | "right">;
}
declare class DialogFactory extends ViewFactory<Props, Dialog> { }
declare const factory: DialogFactory;
export = factory;
