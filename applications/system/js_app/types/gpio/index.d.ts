import type { Contract } from "../event_loop";

export interface Mode {
    direction: "in" | "out";
    outMode?: "push_pull" | "open_drain";
    inMode?: "analog" | "plain_digital" | "interrupt" | "event";
    edge?: "rising" | "falling" | "both";
    pull?: "up" | "down";
}

export interface Pin {
    /**
     * Configures a pin. This may be done several times.
     * @param mode Pin configuration object
     */
    init(mode: Mode): void;
    /**
     * Sets the output value of a pin if it's been configured with
     * `direction: "out"`.
     * @param value Logic value to output
     */
    write(value: boolean): void;
    /**
     * Gets the input value of a pin if it's been configured with
     * `direction: "in"`, but not `inMode: "analog"`.
     */
    read(): boolean;
    /**
     * Gets the input voltage of a pin in millivolts if it's been configured
     * with `direction: "in"` and `inMode: "analog"`
     */
    read_analog(): number;
    /**
     * Returns an `event_loop` event that can be used to listen to interrupts,
     * as configured by `init`
     */
    interrupt(): Contract;
}

/**
 * Returns an object that can be used to manage a GPIO pin. For the list of
 * available pins, see https://docs.flipper.net/gpio-and-modules#miFsS
 * @param pin Pin name (e.g. `"PC3"`) or number (e.g. `7`)
 */
export function get(pin: string | number): Pin;
