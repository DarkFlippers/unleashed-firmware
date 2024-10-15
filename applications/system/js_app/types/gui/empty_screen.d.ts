import type { View, ViewFactory } from ".";

type Props = {};
declare class EmptyScreen extends View<Props> { }
declare class EmptyScreenFactory extends ViewFactory<Props, EmptyScreen> { }
declare const factory: EmptyScreenFactory;
export = factory;
