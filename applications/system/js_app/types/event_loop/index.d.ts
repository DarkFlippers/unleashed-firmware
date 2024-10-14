type Lit = undefined | null | {};

/**
 * Subscription control interface
 */
export interface Subscription {
    /**
     * Cancels the subscription, preventing any future events managed by the
     * subscription from firing
     */
    cancel(): void;
}

/**
 * Opaque event source identifier
 */
export type Contract<Item = undefined> = symbol;

/**
 * A callback can be assigned to an event loop to listen to an event. It may
 * return an array with values that will be passed to it as arguments the next
 * time that it is called. The first argument is always the subscription
 * manager, and the second argument is always the item that trigged the event.
 * The type of the item is defined by the event source.
 */
export type Callback<Item, Args extends Lit[]> = (subscription: Subscription, item: Item, ...args: Args) => Args | undefined | void;

/**
 * Subscribes a callback to an event
 * @param contract Event identifier
 * @param callback Function to call when the event is triggered
 * @param args Initial arguments passed to the callback
 */
export function subscribe<Item, Args extends Lit[]>(contract: Contract<Item>, callback: Callback<Item, Args>, ...args: Args): Subscription;
/**
 * Runs the event loop until it is stopped (potentially never)
 */
export function run(): void | never;
/**
 * Stops the event loop
 */
export function stop(): void;

/**
 * Creates a timer event that can be subscribed to just like any other event
 * @param mode Either `"oneshot"` or `"periodic"`
 * @param interval Timer interval in milliseconds
 */
export function timer(mode: "oneshot" | "periodic", interval: number): Contract;

/**
 * Message queue
 */
export interface Queue<T> {
    /**
     * Message event
     */
    input: Contract<T>;
    /**
     * Sends a message to the queue
     * @param message message to send
     */
    send(message: T): void;
}

/**
 * Creates a message queue
 * @param length maximum queue capacity
 */
export function queue<T>(length: number): Queue<T>;
