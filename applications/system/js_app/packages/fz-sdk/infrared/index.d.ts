/**
 * Module for using Infrared blaster/receptor
 * @version Available with JS feature `infrared-send`
 * @module
 */

/**
 * Sends an IR signal using a known protocol by Flipper Firmware
 * @param address Note that the address expects a number. If you're reading from Flipper's IR files, the address is usually in little-endian hex format. Javascript numbers are defined as big endian by default.
 * @param command Note that the command expects a number. If you're reading from Flipper's IR files, the command is usually in little-endian hex format. Javascript numbers are defined as big endian by default.
 * @param options Repeat marks the signal as a repeat signal, times indicates how many times to send the signal
 */
export declare function sendSignal(
  protocol:
    | "NEC"
    | "NECext"
    | "NEC42"
    | "NEC42ext"
    | "Samsung32"
    | "RC6"
    | "RC5"
    | "RC5X"
    | "SIRC"
    | "SIRC15"
    | "SIRC20"
    | "Kaseikyo"
    | "RCA",
  address: number,
  command: number,
  options?: { repeat?: boolean; times?: number },
): void;

/**
 * Sends a signal from an unknown protocol
 * @param startFromMark defaults to true
 */
export declare function sendRawSignal(
  timings: number[],
  startFromMark?: boolean,
  advancedSettings?: { frequency: number; dutyCycle: number },
): void;
