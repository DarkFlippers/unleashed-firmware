import type { Contract } from "../event_loop";

type Properties = { [K: string]: any };

export declare class View<Props extends Properties> {
    /**
     * Assign value to property by name
     * @param property Name of the property
     * @param value Value to assign
     */
    set<P extends keyof Props>(property: P, value: Props[P]): void;
}

export declare class ViewFactory<Props extends Properties, V extends View<Props>> {
    /**
     * Create view instance with default values, can be changed later with set()
     */
    make(): V;
    /**
     * Create view instance with custom values, can be changed later with set()
     * @param initial Dictionary of property names to values
     */
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
     * View object currently shown
     */
    currentView: View<any>;
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
