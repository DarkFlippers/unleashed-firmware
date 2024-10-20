/**
 * @brief Check if there is an I2C device ready on the bus
 * @param address The device address to check
 * @param timeout Timeout in milliseconds
 */
export declare function isDeviceReady(address: number, timeout?: number): boolean;

/**
 * @brief Write data to I2C device and return success status
 * @param address The device address to write to
 * @param data The data to write to the device
 * @param timeout Timeout in milliseconds
 */
export declare function write(address: number, data: number[] | ArrayBuffer, timeout?: number): boolean;

/**
 * @brief Read data from I2C device or return undefined on failure
 * @param address The device address to read from
 * @param length How many bytes to read
 * @param timeout Timeout in milliseconds
 */
export declare function read(address: number, length: number, timeout?: number): ArrayBuffer | undefined;

/**
 * @brief Write data then read from I2C device or return undefined on failure
 * @param address The device address to talk to
 * @param writeData The data to write to the device
 * @param readLength How many bytes to read
 * @param timeout Timeout in milliseconds
 */
export declare function writeRead(address: number, writeData: number[] | ArrayBuffer, readLength: number, timeout?: number): ArrayBuffer | undefined;
