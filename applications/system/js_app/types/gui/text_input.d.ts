import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    minLength: number,
    maxLength: number,
    defaultText: string,
    defaultTextClear: boolean,
}
declare class TextInput extends View<Props> {
    input: Contract<string>;
}
declare class TextInputFactory extends ViewFactory<Props, TextInput> { }
declare const factory: TextInputFactory;
export = factory;
