/**
 * @brief Pauses JavaScript execution for a while
 * @param ms How many milliseconds to pause the execution for
 */
declare function delay(ms: number): void;

/**
 * @brief Prints to the GUI console view
 * @param args The arguments are converted to strings, concatenated without any
 *             spaces in between and printed to the console view
 */
declare function print(...args: any[]): void;

/**
 * @brief Converts a number to a string
 * @param value The number to convert to a string
 * @param base Integer base (`2`...`16`), default: 10
 */
declare function toString(value: number, base?: number): string;

/**
 * @brief Converts a string to a number
 * @param text The string to convert to a number
 * @param base Integer base (`2`...`16`), default: 10
 */
declare function parseInt(text: string, base?: number): number;

/**
 * @brief Path to the directory containing the current script
 */
declare const __dirname: string;

/**
 * @brief Path to the current script file
 */
declare const __filename: string;

/**
 * @brief Reads a JS value from a file
 * 
 * Reads a file at the specified path, interprets it as a JS value and returns
 * the last value pushed on the stack.
 * 
 * @param path The path to the file
 * @param scope An object to use as global scope while running this file
 */
declare function load(path: string, scope?: object): any;

/**
 * @brief Return 1-byte string whose ASCII code is the integer `n`
 * 
 * If `n` is not numeric or outside of `0-255` range, `null` is returned
 * 
 * @param n The ASCII code to convert to string
 */
declare function chr(n: number): string | null;

/**
 * @brief mJS Foreign Pointer type
 * 
 * JavaScript code cannot do anything with values of `RawPointer` type except
 * acquire them from native code and pass them right back to other parts of
 * native code. These values cannot be turned into something meaningful, nor can
 * be they modified.
 */
declare type RawPointer = symbol & { "__tag__": "raw_ptr" };
// introducing a nominal type in a hacky way; the `__tag__` property doesn't really exist.

/**
 * @brief Holds raw bytes
 */
declare class ArrayBuffer {
    /**
     * @brief The pointer to the byte buffer
     * @note Like other `RawPointer` values, this value is essentially useless
     *       to JS code.
     */
    getPtr: RawPointer;
    /**
     * @brief The length of the buffer in bytes
     */
    byteLength: number;
    /**
     * @brief Creates an `ArrayBuffer` that contains a sub-part of the buffer
     * @param start The index of the byte in the source buffer to be used as the
     *              start for the new buffer
     * @param end The index of the byte in the source buffer that follows the
     *            byte to be used as the last byte for the new buffer
     */
    slice(start: number, end?: number): ArrayBuffer;
}

declare function ArrayBuffer(): ArrayBuffer;

declare type ElementType = "u8" | "i8" | "u16" | "i16" | "u32" | "i32";

declare class TypedArray<E extends ElementType> {
    /**
     * @brief The length of the buffer in bytes
     */
    byteLength: number;
    /**
     * @brief The length of the buffer in typed elements
     */
    length: number;
    /**
     * @brief The underlying `ArrayBuffer`
     */
    buffer: ArrayBuffer;
}

declare class Uint8Array extends TypedArray<"u8"> { }
declare class Int8Array extends TypedArray<"i8"> { }
declare class Uint16Array extends TypedArray<"u16"> { }
declare class Int16Array extends TypedArray<"i16"> { }
declare class Uint32Array extends TypedArray<"u32"> { }
declare class Int32Array extends TypedArray<"i32"> { }

declare function Uint8Array(data: ArrayBuffer | number | number[]): Uint8Array;
declare function Int8Array(data: ArrayBuffer | number | number[]): Int8Array;
declare function Uint16Array(data: ArrayBuffer | number | number[]): Uint16Array;
declare function Int16Array(data: ArrayBuffer | number | number[]): Int16Array;
declare function Uint32Array(data: ArrayBuffer | number | number[]): Uint32Array;
declare function Int32Array(data: ArrayBuffer | number | number[]): Int32Array;

declare const console: {
    /**
     * @brief Prints to the UART logs at the `[I]` level
     * @param args The arguments are converted to strings, concatenated without any
     *             spaces in between and printed to the logs
     */
    log(...args: any[]): void;
    /**
     * @brief Prints to the UART logs at the `[D]` level
     * @param args The arguments are converted to strings, concatenated without any
     *             spaces in between and printed to the logs
     */
    debug(...args: any[]): void;
    /**
     * @brief Prints to the UART logs at the `[W]` level
     * @param args The arguments are converted to strings, concatenated without any
     *             spaces in between and printed to the logs
     */
    warn(...args: any[]): void;
    /**
     * @brief Prints to the UART logs at the `[E]` level
     * @param args The arguments are converted to strings, concatenated without any
     *             spaces in between and printed to the logs
     */
    error(...args: any[]): void;
};

declare class Array<T> {
    /**
     * @brief Takes items out of the array
     * 
     * Removes elements from the array and returns them in a new array
     * 
     * @param start The index to start taking elements from
     * @param deleteCount How many elements to take
     * @returns The elements that were taken out of the original array as a new
     *          array
     */
    splice(start: number, deleteCount: number): T[];
    /**
     * @brief Adds a value to the end of the array
     * @param value The value to add
     * @returns New length of the array
     */
    push(value: T): number;
    /**
     * @brief How many elements there are in the array
     */
    length: number;
}

declare class String {
    /**
     * @brief How many characters there are in the string
     */
    length: number;
    /**
     * @brief Returns the character code at an index in the string
     * @param index The index to consult
     */
    charCodeAt(index: number): number;
    /**
     * See `charCodeAt`
     */
    at(index: number): number;
    /**
     * @brief Return index of first occurence of substr within the string or `-1` if not found
     * @param substr The string to search for
     * @param fromIndex The index to start searching from
     */
    indexOf(substr: string, fromIndex?: number): number;
    /**
     * @brief Return a substring between two indices
     * @param start The index to start substring at
     * @param end The index to end substring at
     */
    slice(start: number, end?: number): string;
    /**
     * @brief Return this string transformed to upper case
     */
    toUpperCase(): string;
    /**
     * @brief Return this string transformed to lower case
     */
    toLowerCase(): string;
}

declare class Boolean { }

declare class Function { }

declare class Number { }

declare class Object { }

declare class RegExp { }

declare interface IArguments { }

declare type Partial<O extends object> = { [K in keyof O]?: O[K] };
