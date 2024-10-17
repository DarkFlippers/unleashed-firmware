/**
 * @brief Displays a file picker and returns the selected file, or undefined if cancelled
 * @param basePath The path to start at
 * @param extension The file extension(s) to show (like `.sub`, `.iso|.img`, `*`)
 */
export declare function pickFile(basePath: string, extension: string): string | undefined;
