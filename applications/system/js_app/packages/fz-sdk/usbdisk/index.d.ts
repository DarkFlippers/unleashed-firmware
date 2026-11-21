/**
 * Module for USB mass storage emulation
 * @version Available with JS feature `usbdisk`
 * @module
 */

/**
 * @brief Create an empty disk image
 *
 * The image is grown without writing its contents, so a large one takes far
 * less time than its size suggests, and the first 4 KiB are cleared so the host
 * offers to format it rather than mounting leftovers. Fails if something
 * already exists at `path`.
 *
 * @param path Where to create the image
 * @param size Image size in bytes, rounded down to a 512-byte block
 * @version Available with JS feature `usbdisk-createimage`
 */
export declare function createImage(path: string, size: number): void;

/**
 * @brief Start emulating mass storage device
 * @param path The disk image to emulate
 */
export declare function start(path: string): void;

/**
 * @brief Stop emulating mass storage device
 */
export declare function stop(): void;

/**
 * @brief Check if the mass storage device was exected
 * 
 * Useful as a loop condition with a delay, so UsbDisk keeps running until ejected
 * 
 */
export declare function wasEjected(): boolean;
