/**
 * Things from this module are automatically available to you without having to
 * explicitly import anything.
 * 
 * # SDK versioning and features
 * 
 * ## Motivation
 * It is very important that you check that features are implemented before you
 * use them. By adding the necessary checks, you ensure that your users get a
 * clear warning instead of a cryptic error message when running the script.
 * 
 * This system has been designed in collaboration with our community in order to
 * make things better for everybody involved. You can find out more in this
 * discussion: https://github.com/flipperdevices/flipperzero-firmware/pull/3961
 * 
 * ## Community agreement
 * Each interpreter implementation (aka "JS SDK", aka "JS API"), including
 * those found in third-party firmware distributions, defines two markers for
 * signaling what it supports: the **SDK version** and the
 * **extra feature set**.
 * 
 * The **SDK version** consists of two semver-like integer components: the major
 * version and the minor version. Like semver, the major version is bumped when
 * a breaking change is introduced (i.e. one that would require correction of
 * apps by their developers), and the minor version is bumped when a new
 * non-breaking feature is introduced. Because we have adopted TypeScript,
 * the https://www.semver-ts.org/ standard is used to determine whether a change
 * is breaking or not. The basis of `semver-ts` is the "no new red squiggles"
 * rule.
 * 
 * Every major version is associated with a set of **extra features** that are
 * present in some firmware distributions but not others. Distributions may
 * cross-port features between each other, until at some point they get ported
 * into the upstream firmware distribution. With the next major release of the
 * JS SDK, all extra features present in the upstream distribution are now
 * declared **baseline features**, and thus no longer recognized as "extra
 * features".
 * 
 * Before using a feature, you must check that the interpreter that you're
 * running on actually supports it. If you don't, the portability of your
 * application will suffer.
 * 
 * ## Implementation
 * Use the following functions to check version compatibility:
 *   - `checkSdkCompatibility` when your script absolutely cannot function on an
 *     incompatible interpreter
 *   - `isSdkCompatible` when your script can leverage multiple interpreter
 *     editions to its advantage
 *   - `sdkCompatibilityStatus` when you need a detailed status on compatibility
 * 
 * Use the following functions to check feature compatibility:
 *   - `checkSdkFeatures` when your script absolutely cannot function on an
 *     incompatible interpreter
 *   - `doesSdkSupport` when your script can leverage multiple interpreter
 *     editions to its advantage
 * 
 * ## Automatic version enforcement
 * The SDK will automatically insert a call to `checkSdkCompatibility` in the
 * beginning of the resulting script. If you would like to disable this check
 * and instead use other manual compatibility checking facilities, edit your
 * `fz-sdk.config.json5`.
 * 
 * # Standard library
 * Standard library features are mostly unimplemented. This module defines,
 * among other things, the features that _are_ implemented.
 * 
 * @version Added in JS SDK 0.1
 * @module
 */

/**
 * @brief Checks compatibility between the script and the JS SDK that the
 *        firmware provides
 * 
 * @note You're looking at JS SDK v0.3
 * 
 * @param expectedMajor JS SDK major version expected by the script
 * @param expectedMinor JS SDK minor version expected by the script
 * @returns Compatibility status:
 *   - `"compatible"` if the script and the JS SDK are compatible
 *   - `"firmwareTooOld"` if the expected major version is larger than the
 *     version of the firmware, or if the expected minor version is larger than
 *     the version of the firmware
 *   - `"firmwareTooNew"` if the expected major version is lower than the
 *     version of the firmware
 * @version Added in JS SDK 0.1
 */
declare function sdkCompatibilityStatus(expectedMajor: number, expectedMinor: number):
    "compatible" | "firmwareTooOld" | "firmwareTooNew";

/**
 * @brief Checks compatibility between the script and the JS SDK that the
 *        firmware provides in a boolean fashion
 * 
 * @note You're looking at JS SDK v0.3
 * 
 * @param expectedMajor JS SDK major version expected by the script
 * @param expectedMinor JS SDK minor version expected by the script
 * @returns `true` if the two are compatible, `false` otherwise
 * @version Added in JS SDK 0.1
 */
declare function isSdkCompatible(expectedMajor: number, expectedMinor: number): boolean;

/**
 * @brief Asks the user whether to continue executing the script if the versions
 *        are not compatible. Does nothing if they are.
 * 
 * @note You're looking at JS SDK v0.3
 * 
 * @param expectedMajor JS SDK major version expected by the script
 * @param expectedMinor JS SDK minor version expected by the script
 * @version Added in JS SDK 0.1
 */
declare function checkSdkCompatibility(expectedMajor: number, expectedMinor: number): void | never;

/**
 * @brief Checks whether all of the specified extra features are supported by
 *        the interpreter.
 * @warning This function will return `false` if a queried feature is now
 *          recognized as a baseline feature. For more info, consult the module
 *          documentation.
 * @param features Array of named features to query
 * @version Added in JS SDK 0.1
 */
declare function doesSdkSupport(features: string[]): boolean;

/**
 * @brief Checks whether all of the specified extra features are supported by
 *        the interpreter, asking the user if they want to continue running the
 *        script if they're not.
 * @warning This function will act as if the feature is not implemented for
 *          features that are now recognized as baseline features. For more
 *          info, consult the module documentation.
 * @param features Array of named features to query
 * @version Added in JS SDK 0.1
 */
declare function checkSdkFeatures(features: string[]): void | never;

/**
 * @brief Pauses JavaScript execution for a while
 * @param ms How many milliseconds to pause the execution for
 * @version Added in JS SDK 0.1
 */
declare function delay(ms: number): void;

/**
 * @brief Prints to the GUI console view
 * @param args The arguments are converted to strings, concatenated without any
 *             spaces in between and printed to the console view
 * @version Added in JS SDK 0.1
 */
declare function print(...args: any[]): void;

/**
 * @brief Converts a string to a number
 * @param text The string to convert to a number
 * @param base Integer base (`2`...`16`), default: 10
 * @version Added in JS SDK 0.1
 */
declare function parseInt(text: string, base?: number): number;

/**
 * @brief Path to the directory containing the current script
 * @version Added in JS SDK 0.1
 */
declare const __dirname: string;

/**
 * @brief Path to the current script file
 * @version Added in JS SDK 0.1
 */
declare const __filename: string;

/**
 * @brief Runs a JS file and returns value from it
 * 
 * Reads a file at the specified path and runs it as JS, returning the last evaluated value.
 * 
 * The result is cached and this filepath will not re-evaluated on future
 * load() calls for this session.
 * 
 * @param path The path to the file
 * @param scope An object to use as global scope while running this file
 * @version Added in JS SDK 0.1
 */
declare function load(path: string, scope?: object): any;

/**
 * @brief Return 1-byte string whose ASCII code is the integer `n`
 * 
 * If `n` is not numeric or outside of `0-255` range, `null` is returned
 * 
 * @param n The ASCII code to convert to string
 * @version Added in JS SDK 0.1
 */
declare function chr(n: number): string | null;

/**
 * @brief Loads a natively implemented module
 * @param module The name of the module to load
 * @version Added in JS SDK 0.1
 */
declare function require(module: string): any;

/**
 * @brief Exit JavaScript with given message
 * @param message The error message to show to user
 * @version Added in JS SDK 0.1
 */
declare function die(message: string): never;

/**
 * @brief mJS Foreign Pointer type
 * 
 * JavaScript code cannot do anything with values of `RawPointer` type except
 * acquire them from native code and pass them right back to other parts of
 * native code. These values cannot be turned into something meaningful, nor can
 * be they modified.
 * 
 * @version Added in JS SDK 0.1
 */
declare type RawPointer = symbol & { "__tag__": "raw_ptr" };
// introducing a nominal type in a hacky way; the `__tag__` property doesn't really exist.

/**
 * @brief Holds raw bytes
 * @version Added in JS SDK 0.1
 */
declare class ArrayBuffer {
    /**
     * @brief The pointer to the byte buffer
     * @note Like other `RawPointer` values, this value is essentially useless
     *       to JS code.
     * @version Added in JS SDK 0.1
     */
    getPtr: RawPointer;
    /**
     * @brief The length of the buffer in bytes
     * @version Added in JS SDK 0.1
     */
    byteLength: number;
    /**
     * @brief Creates an `ArrayBuffer` that contains a sub-part of the buffer
     * @param start The index of the byte in the source buffer to be used as the
     *              start for the new buffer
     * @param end The index of the byte in the source buffer that follows the
     *            byte to be used as the last byte for the new buffer
     * @version Added in JS SDK 0.1
     */
    slice(start: number, end?: number): ArrayBuffer;
}

declare function ArrayBuffer(): ArrayBuffer;

declare type ElementType = "u8" | "i8" | "u16" | "i16" | "u32" | "i32";

declare class TypedArray<E extends ElementType> {
    /**
     * @brief The length of the buffer in bytes
     * @version Added in JS SDK 0.1
     */
    byteLength: number;
    /**
     * @brief The length of the buffer in typed elements
     * @version Added in JS SDK 0.1
     */
    length: number;
    /**
     * @brief The underlying `ArrayBuffer`
     * @version Added in JS SDK 0.1
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
     * @version Added in JS SDK 0.1
     */
    log(...args: any[]): void;
    /**
     * @brief Prints to the UART logs at the `[D]` level
     * @param args The arguments are converted to strings, concatenated without any
     *             spaces in between and printed to the logs
     * @version Added in JS SDK 0.1
     */
    debug(...args: any[]): void;
    /**
     * @brief Prints to the UART logs at the `[W]` level
     * @param args The arguments are converted to strings, concatenated without any
     *             spaces in between and printed to the logs
     * @version Added in JS SDK 0.1
     */
    warn(...args: any[]): void;
    /**
     * @brief Prints to the UART logs at the `[E]` level
     * @param args The arguments are converted to strings, concatenated without any
     *             spaces in between and printed to the logs
     * @version Added in JS SDK 0.1
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
     * @version Added in JS SDK 0.1
     */
    splice(start: number, deleteCount: number): T[];
    /**
     * @brief Adds a value to the end of the array
     * @param value The value to add
     * @returns New length of the array
     * @version Added in JS SDK 0.1
     */
    push(value: T): number;
    /**
     * @brief How many elements there are in the array
     * @version Added in JS SDK 0.1
     */
    length: number;
}

declare class String {
    /**
     * @brief How many characters there are in the string
     * @version Added in JS SDK 0.1
     */
    length: number;
    /**
     * @brief Returns the character code at an index in the string
     * @param index The index to consult
     * @version Added in JS SDK 0.1
     */
    charCodeAt(index: number): number;
    /**
     * See `charCodeAt`
     * @version Added in JS SDK 0.1
     */
    at(index: number): number;
    /**
     * @brief Return index of first occurrence of substr within the string or `-1` if not found
     * @param substr The string to search for
     * @param fromIndex The index to start searching from
     * @version Added in JS SDK 0.1
     */
    indexOf(substr: string, fromIndex?: number): number;
    /**
     * @brief Return a substring between two indices
     * @param start The index to start substring at
     * @param end The index to end substring at
     * @version Added in JS SDK 0.1
     */
    slice(start: number, end?: number): string;
    /**
     * @brief Return this string transformed to upper case
     * @version Added in JS SDK 0.1
     */
    toUpperCase(): string;
    /**
     * @brief Return this string transformed to lower case
     * @version Added in JS SDK 0.1
     */
    toLowerCase(): string;
}

declare class Boolean { }

declare class Function { }

declare class Number {
    /**
     * @brief Converts this number to a string
     * @param base Integer base (`2`...`16`), default: 10
     * @version Added in JS SDK 0.1
     */
    toString(base?: number): string;
}

declare class Object { }

declare class RegExp { }

declare interface IArguments { }

declare type Partial<O extends object> = { [K in keyof O]?: O[K] };
