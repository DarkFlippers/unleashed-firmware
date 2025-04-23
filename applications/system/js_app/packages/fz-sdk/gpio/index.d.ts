/**
 * Module for accessing the GPIO (General Purpose Input/Output) ports
 * 
 * ```js
 * let eventLoop = require("event_loop");
 * let gpio = require("gpio");
 * ```
 * 
 * This module depends on the `event_loop` module, so it _must_ only be imported
 * after `event_loop` is imported.
 * 
 * # Example
 * ```js
 * let eventLoop = require("event_loop");
 * let gpio = require("gpio");
 * 
 * let led = gpio.get("pc3");
 * led.init({ direction: "out", outMode: "push_pull" });
 * 
 * led.write(true);
 * delay(1000);
 * led.write(false);
 * delay(1000);
 * ```
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

import type { Contract } from "../event_loop";

/**
 * @version Added in JS SDK 0.1
 */
export interface Mode {
    direction: "in" | "out";
    outMode?: "push_pull" | "open_drain";
    inMode?: "analog" | "plain_digital" | "interrupt" | "event";
    edge?: "rising" | "falling" | "both";
    pull?: "up" | "down";
}

/**
 * @version Added in JS SDK 0.1
 */
export interface Pin {
    /**
     * Configures a pin. This may be done several times.
     * @param mode Pin configuration object
     * @version Added in JS SDK 0.1
     */
    init(mode: Mode): void;
    /**
     * Sets the output value of a pin if it's been configured with
     * `direction: "out"`.
     * @param value Logic value to output
     * @version Added in JS SDK 0.1
     */
    write(value: boolean): void;
    /**
     * Gets the input value of a pin if it's been configured with
     * `direction: "in"`, but not `inMode: "analog"`.
     * @version Added in JS SDK 0.1
     */
    read(): boolean;
    /**
     * Gets the input voltage of a pin in millivolts if it's been configured
     * with `direction: "in"` and `inMode: "analog"`
     * @version Added in JS SDK 0.1
     */
    readAnalog(): number;
    /**
     * Returns an `event_loop` event that can be used to listen to interrupts,
     * as configured by `init`
     * @version Added in JS SDK 0.1
     */
    interrupt(): Contract;
    /**
     * Determines whether this pin supports PWM. If `false`, all other
     * PWM-related methods on this pin will throw an error when called.
     * @note On Flipper Zero only pins PA4 and PA7 support PWM
     * @version Added in JS SDK 0.2, extra feature `"gpio-pwm"`
     */
    isPwmSupported(): boolean;
    /**
     * Sets PWM parameters and starts the PWM. Configures the pin with
     * `{ direction: "out", outMode: "push_pull" }`. Throws an error if PWM is
     * not supported on this pin.
     * @param freq Frequency in Hz
     * @param duty Duty cycle in %
     * @version Added in JS SDK 0.2, extra feature `"gpio-pwm"`
     */
    pwmWrite(freq: number, duty: number): void;
    /**
     * Determines whether PWM is running. Throws an error if PWM is not
     * supported on this pin.
     * @version Added in JS SDK 0.2, extra feature `"gpio-pwm"`
     */
    isPwmRunning(): boolean;
    /**
     * Stops PWM. Does not restore previous pin configuration. Throws an error
     * if PWM is not supported on this pin.
     * @version Added in JS SDK 0.2, extra feature `"gpio-pwm"`
     */
    pwmStop(): void;
}

/**
 * Returns an object that can be used to manage a GPIO pin. For the list of
 * available pins, see https://docs.flipper.net/gpio-and-modules#miFsS
 * @param pin Pin name (e.g. `"PC3"`) or number (e.g. `7`)
 * @version Added in JS SDK 0.1
 */
export function get(pin: string | number): Pin;
