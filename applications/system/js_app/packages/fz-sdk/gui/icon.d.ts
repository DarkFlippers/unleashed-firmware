export type BuiltinIcon = "DolphinWait_59x54" | "js_script_10px"
    | "off_19x20" | "off_hover_19x20"
    | "power_19x20" | "power_hover_19x20"
    | "Settings_14";

export type IconData = symbol & { "__tag__": "icon" };
// introducing a nominal type in a hacky way; the `__tag__` property doesn't really exist.

/**
 * Gets a built-in firmware icon for use in GUI
 * @param icon Name of the icon
 * @version Added in JS SDK 0.2, extra feature `"gui-widget"`
 * @version Baseline since JS SDK 1.0
 */
export declare function getBuiltin(icon: BuiltinIcon): IconData;

/**
 * Loads a .fxbm icon (XBM Flipper sprite, from flipperzero-game-engine) for use in GUI
 * @param path Path to the .fxbm file
 * @version Added in JS SDK 0.3, extra feature `"gui-widget-extras"`
 * @version Baseline since JS SDK 1.0
 */
export declare function loadFxbm(path: string): IconData;
