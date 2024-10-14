import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    items: string[],
};
declare class Submenu extends View<Props> {
    chosen: Contract<number>;
}
declare class SubmenuFactory extends ViewFactory<Props, Submenu> { }
declare const factory: SubmenuFactory;
export = factory;
