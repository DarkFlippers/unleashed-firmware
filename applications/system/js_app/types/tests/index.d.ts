/**
 * Unit test module. Only available if the firmware has been configured with
 * `FIRMWARE_APP_SET=unit_tests`.
 */

export function fail(message: string): never;
export function assert_eq<T>(expected: T, result: T): void | never;
export function assert_float_close(expected: number, result: number, epsilon: number): void | never;
