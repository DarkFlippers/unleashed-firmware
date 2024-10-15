/**
 * @brief Special key codes that this module recognizes
 */
export type ModifierKey = "CTRL" | "SHIFT" | "ALT" | "GUI";

export type MainKey =
    "DOWN" | "LEFT" | "RIGHT" | "UP" |

    "ENTER" | "PAUSE" | "CAPSLOCK" | "DELETE" | "BACKSPACE" | "END" | "ESC" |
    "HOME" | "INSERT" | "NUMLOCK" | "PAGEUP" | "PAGEDOWN" | "PRINTSCREEN" |
    "SCROLLLOCK" | "SPACE" | "TAB" | "MENU" |

    "F1" | "F2" | "F3" | "F4" | "F5" | "F6" | "F7" | "F8" | "F9" | "F10" |
    "F11" | "F12" | "F13" | "F14" | "F15" | "F16" | "F17" | "F18" | "F19" |
    "F20" | "F21" | "F22" | "F23" | "F24" |

    "\n" | " " | "!" | "\"" | "#" | "$" | "%" | "&" | "'" | "(" | ")" | "*" |
    "+" | "," | "-" | "." | "/" | ":" | ";" | "<" | ">" | "=" | "?" | "@" | "[" |
    "]" | "\\" | "^" | "_" | "`" | "{" | "}" | "|" | "~" |

    "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |

    "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" |
    "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" |
    "Y" | "Z" |

    "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" |
    "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" |
    "y" | "z";

export type KeyCode = MainKey | ModifierKey | number;

/**
 * @brief Initializes the module
 * @param settings USB device settings. Omit to select default parameters
 */
export declare function setup(settings?: { vid: number, pid: number, mfrName?: string, prodName?: string }): void;

/**
 * @brief Tells whether the virtual USB HID device has successfully connected
 */
export declare function isConnected(): boolean;

/**
 * @brief Presses one or multiple keys at once, then releases them
 * @param keys The arguments represent a set of keys to. Out of that set, only
 *             one of the keys may represent a "main key" (see `MainKey`), with
 *             the rest being modifier keys (see `ModifierKey`).
 */
export declare function press(...keys: KeyCode[]): void;

/**
 * @brief Presses one or multiple keys at once without releasing them
 * @param keys The arguments represent a set of keys to. Out of that set, only
 *             one of the keys may represent a "main key" (see `MainKey`), with
 *             the rest being modifier keys (see `ModifierKey`).
 */
export declare function hold(...keys: KeyCode[]): void;

/**
 * @brief Releases one or multiple keys at once
 * @param keys The arguments represent a set of keys to. Out of that set, only
 *             one of the keys may represent a "main key" (see `MainKey`), with
 *             the rest being modifier keys (see `ModifierKey`).
 */
export declare function release(...keys: KeyCode[]): void;

/**
 * @brief Prints a string by repeatedly pressing and releasing keys
 * @param string The string to print
 * @param delay How many milliseconds to wait between key presses
 */
export declare function print(string: string, delay?: number): void;

/**
 * @brief Prints a string by repeatedly pressing and releasing keys. Presses
 *        "Enter" after printing the string
 * @param string The string to print
 * @param delay How many milliseconds to wait between key presses
 */
export declare function println(string: string, delay?: number): void;
