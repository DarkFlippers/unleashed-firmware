import type { View, ViewFactory } from ".";

type Props = {};
declare class Loading extends View<Props> { }
declare class LoadingFactory extends ViewFactory<Props, Loading> { }
declare const factory: LoadingFactory;
export = factory;
