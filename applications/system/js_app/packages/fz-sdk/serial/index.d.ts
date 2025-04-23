/**
 * Module for accessing the serial port
 * @version Added in JS SDK 0.1
 * @module
 */

export interface Framing {
    /**
     * @note 6 data bits can only be selected when parity is enabled (even or
     * odd)
     * @note 9 data bits can only be selected when parity is disabled (none)
     */
    dataBits: "6" | "7" | "8" | "9";
    parity: "none" | "even" | "odd";
    /**
     * @note LPUART only supports whole stop bit lengths (i.e. 1 and 2 but not
     * 0.5 and 1.5)
     */
    stopBits: "0.5" | "1" | "1.5" | "2";
}

/**
 * @brief Initializes the serial port
 * 
 * Automatically disables Expansion module service to prevent interference.
 * 
 * @param port The port to initialize (`"lpuart"` or `"usart"`)
 * @param baudRate Baud rate
 * @param framing See `Framing` type
 * @version Added in JS SDK 0.1
 * @version Added `framing` parameter in JS SDK 0.3, extra feature `"serial-framing"`
 */
export declare function setup(port: "lpuart" | "usart", baudRate: number, framing?: Framing): void;

/**
 * @brief Writes data to the serial port
 * @param value The data to write:
 *                - Strings will get sent as ASCII.
 *                - Numbers will get sent as a single byte.
 *                - Arrays of numbers will get sent as a sequence of bytes.
 *                - `ArrayBuffer`s and `TypedArray`s will be sent as a sequence
 *                  of bytes.
 * @version Added in JS SDK 0.1
 */
export declare function write<E extends ElementType>(value: string | number | number[] | ArrayBuffer | TypedArray<E>): void;

/**
 * @brief Reads data from the serial port
 * @param length The number of bytes to read
 * @param timeout The number of time, in milliseconds, after which this function
 *                will give up and return what it read up to that point. If
 *                unset, the function will wait forever.
 * @returns The received data interpreted as ASCII, or `undefined` if 0 bytes
 *          were read.
 * @version Added in JS SDK 0.1
 */
export declare function read(length: number, timeout?: number): string | undefined;

/**
 * @brief Reads data from the serial port
 * 
 * Data is read one character after another until either a `\r` or `\n`
 * character is received, neither of which is included in the result.
 * 
 * @param timeout The number of time, in milliseconds, after which this function
 *                will give up and return what it read up to that point. If
 *                unset, the function will wait forever. The timeout only
 *                applies to characters, not entire strings.
 * @returns The received data interpreted as ASCII, or `undefined` if 0 bytes
 *          were read.
 * @version Added in JS SDK 0.1
 */
export declare function readln(timeout?: number): string;

/**
 * @brief Read any available amount of data from the serial port
 * 
 * Can be useful to avoid starving your loop with small reads.
 * 
 * @param timeout The number of time, in milliseconds, after which this function
 *                will give up and return nothing. If unset, the function will
 *                wait forever.
 * @returns The received data interpreted as ASCII, or `undefined` if 0 bytes
 *          were read.
 * @version Added in JS SDK 0.1
 */
export declare function readAny(timeout?: number): string | undefined;

/**
 * @brief Reads data from the serial port
 * @param length The number of bytes to read
 * @param timeout The number of time, in milliseconds, after which this function
 *                will give up and return what it read up to that point. If
 *                unset, the function will wait forever.
 * @returns The received data as an ArrayBuffer, or `undefined` if 0 bytes were
 *          read.
 * @version Added in JS SDK 0.1
 */
export declare function readBytes(length: number, timeout?: number): ArrayBuffer;

/**
 * @brief Reads data from the serial port, trying to match it to a pattern
 * @param patterns A single pattern or an array of patterns:
 *                   - If the argument is a single `string`, this function will
 *                     match against the given string.
 *                   - If the argument is an array of `number`s, this function
 *                     will match against the given sequence of bytes,
 *                   - If the argument is an array of `string`s, this function
 *                     will match against any string out of the ones that were
 *                     provided.
 *                   - If the argument is an array of arrays of `number`s, this
 *                     function will match against any sequence of bytes out of
 *                     the ones that were provided.
 * @param timeout The number of time, in milliseconds, after which this function
 *                will give up and return what it read up to that point. If
 *                unset, the function will wait forever. The timeout only
 *                applies to characters, not entire strings.
 * @returns The index of the matched pattern if multiple were provided, or 0 if
 *          only one was provided and it matched, or `undefined` if none of the
 *          patterns matched.
 * @version Added in JS SDK 0.1
 */
export declare function expect(patterns: string | number[] | string[] | number[][], timeout?: number): number | undefined;

/**
 * @brief Deinitializes the serial port, allowing multiple initializations per script run
 * @version Added in JS SDK 0.1
 */
export declare function end(): void;
