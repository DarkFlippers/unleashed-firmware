import type { Contract } from "../event_loop";

type Properties = { [K: string]: any };

export declare class View<Props extends Properties> {
    set<P extends keyof Props>(property: P, value: Props[P]): void;
}

export declare class ViewFactory<Props extends Properties, V extends View<Props>> {
    make(): V;
    makeWith(initial: Partial<Props>): V;
}

declare class ViewDispatcher {
    /**
     * Event source for `sendCustom` events
     */
    custom: Contract<number>;
    /**
     * Event source for navigation events (back key presses)
     */
    navigation: Contract;
    /**
     * Sends a number to the custom event handler
     * @param event number to send
     */
    sendCustom(event: number): void;
    /**
     * Switches to a view
     * @param assoc View-ViewDispatcher association as returned by `add`
     */
    switchTo(assoc: View<any>): void;
    /**
     * Sends this ViewDispatcher to the front or back, above or below all other
     * GUI viewports
     * @param direction Either `"front"` or `"back"`
     */
    sendTo(direction: "front" | "back"): void;
}

export const viewDispatcher: ViewDispatcher;
