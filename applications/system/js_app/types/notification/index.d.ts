/**
 * @brief Signals success to the user via the color LED, speaker and vibration
 *        motor
 */
export declare function success(): void;

/**
 * @brief Signals failure to the user via the color LED, speaker and vibration
 *        motor
 */
export declare function error(): void;

export type Color = "red" | "green" | "blue" | "yellow" | "cyan" | "magenta";

/**
 * @brief Displays a basic color on the color LED
 * @param color The color to display, see `Color`
 * @param duration The duration, either `"short"` (10ms) or `"long"` (100ms)
 */
export declare function blink(color: Color, duration: "short" | "long"): void;
