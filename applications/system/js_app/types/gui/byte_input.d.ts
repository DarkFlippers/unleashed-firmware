import type { View, ViewFactory } from ".";
import type { Contract } from "../event_loop";

type Props = {
    header: string,
    length: number,
    defaultData: Uint8Array | ArrayBuffer,
}
declare class ByteInput extends View<Props> {
    input: Contract<string>;
}
declare class ByteInputFactory extends ViewFactory<Props, ByteInput> { }
declare const factory: ByteInputFactory;
export = factory;
