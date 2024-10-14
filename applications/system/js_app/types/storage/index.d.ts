/**
 * File readability mode:
 *   - `"r"`: read-only
 *   - `"w"`: write-only
 *   - `"rw"`: read-write
 */
export type AccessMode = "r" | "w" | "rw";

/**
 * File creation mode:
 *   - `"open_existing"`: open file or fail if it doesn't exist
 *   - `"open_always"`: open file or create a new empty one if it doesn't exist
 *   - `"open_append"`: open file and set r/w pointer to EOF, or create a new one if it doesn't exist
 *   - `"create_new"`: create new file or fail if it exists
 *   - `"create_always"`: truncate and open file, or create a new empty one if it doesn't exist
 */
export type OpenMode = "open_existing" | "open_always" | "open_append" | "create_new" | "create_always";

/** Standard UNIX timestamp */
export type Timestamp = number;

/** File information structure */
export declare class FileInfo {
    /**
     * Full path (e.g. "/ext/test", returned by `stat`) or file name
     * (e.g. "test", returned by `readDirectory`)
     */
    path: string;
    /**
     * Is the file a directory?
     */
    isDirectory: boolean;
    /**
     * File size in bytes, or 0 in the case of directories
     */
    size: number;
    /**
     * Time of last access as a UNIX timestamp
     */
    accessTime: Timestamp;
}

/** Filesystem information structure */
export declare class FsInfo {
    /** Total size of the filesystem, in bytes */
    totalSpace: number;
    /** Free space in the filesystem, in bytes */
    freeSpace: number;
}

// file operations

/** File class */
export declare class File {
    /**
     * Closes the file. After this method is called, all other operations
     * related to this file become unavailable.
     * @returns `true` on success, `false` on failure
     */
    close(): boolean;
    /**
     * Is the file currently open?
     */
    isOpen(): boolean;
    /**
     * Reads bytes from a file opened in read-only or read-write mode
     * @param mode The data type to interpret the bytes as: a `string` decoded
     *             from ASCII data (`"ascii"`), or an `ArrayBuf` (`"binary"`)
     * @param bytes How many bytes to read from the file
     * @returns an `ArrayBuf` if the mode is `"binary"`, a `string` if the mode
     *          is `ascii`. The number of bytes that was actually read may be
     *          fewer than requested.
     */
    read<T extends ArrayBuffer | string>(mode: T extends ArrayBuffer ? "binary" : "ascii", bytes: number): T;
    /**
     * Writes bytes to a file opened in write-only or read-write mode
     * @param data The data to write: a string that will be ASCII-encoded, or an
     *             ArrayBuf
     * @returns the amount of bytes that was actually written
     */
    write(data: ArrayBuffer | string): number;
    /**
     * Moves the R/W pointer forward
     * @param bytes How many bytes to move the pointer forward by
     * @returns `true` on success, `false` on failure
     */
    seekRelative(bytes: number): boolean;
    /**
     * Moves the R/W pointer to an absolute position inside the file
     * @param bytes The position inside the file
     * @returns `true` on success, `false` on failure
     */
    seekAbsolute(bytes: number): boolean;
    /**
     * Gets the absolute position of the R/W pointer in bytes
     */
    tell(): number;
    /**
     * Discards the data after the current position of the R/W pointer in a file
     * opened in either write-only or read-write mode.
     * @returns `true` on success, `false` on failure
     */
    truncate(): boolean;
    /**
     * Reads the total size of the file in bytes
     */
    size(): number;
    /**
     * Detects whether the R/W pointer has reached the end of the file
     */
    eof(): boolean;
    /**
     * Copies bytes from the R/W pointer in the current file to the R/W pointer
     * in another file
     * @param dest The file to copy the bytes into
     * @param bytes The number of bytes to copy
     * @returns `true` on success, `false` on failure
     */
    copyTo(dest: File, bytes: number): boolean;
}

/**
 * Opens a file
 * @param path The path to the file
 * @param accessMode `"r"`, `"w"` or `"rw"`; see `AccessMode`
 * @param openMode `"open_existing"`, `"open_always"`, `"open_append"`,
 *                 `"create_new"` or `"create_always"`; see `OpenMode`
 * @returns a `File` on success, or `undefined` on failure
 */
export declare function openFile(path: string, accessMode: AccessMode, openMode: OpenMode): File | undefined;
/**
 * Detects whether a file exists
 * @param path The path to the file
 * @returns `true` on success, `false` on failure
 */
export declare function fileExists(path: string): boolean;

// directory operations

/**
 * Reads the list of files in a directory
 * @param path The path to the directory
 * @returns Array of `FileInfo` structures with directory entries,
 *          or `undefined` on failure
 */
export declare function readDirectory(path: string): FileInfo[] | undefined;
/**
 * Detects whether a directory exists
 * @param path The path to the directory
 */
export declare function directoryExists(path: string): boolean;
/**
 * Creates an empty directory
 * @param path The path to the new directory
 * @returns `true` on success, `false` on failure
 */
export declare function makeDirectory(path: string): boolean;

// common (file/dir) operations

/**
 * Detects whether a file or a directory exists
 * @param path The path to the file or directory
 */
export declare function fileOrDirExists(path: string): boolean;
/**
 * Acquires metadata about a file or directory
 * @param path The path to the file or directory
 * @returns A `FileInfo` structure or `undefined` on failure
 */
export declare function stat(path: string): FileInfo | undefined;
/**
 * Removes a file or an empty directory
 * @param path The path to the file or directory
 * @returns `true` on success, `false` on failure
 */
export declare function remove(path: string): boolean;
/**
 * Removes a file or recursively removes a possibly non-empty directory
 * @param path The path to the file or directory
 * @returns `true` on success, `false` on failure
 */
export declare function rmrf(path: string): boolean;
/**
 * Renames or moves a file or directory
 * @param oldPath The old path to the file or directory
 * @param newPath The new path that the file or directory will become accessible
 *                under
 * @returns `true` on success, `false` on failure
 */
export declare function rename(oldPath: string, newPath: string): boolean;
/**
 * Copies a file or recursively copies a possibly non-empty directory
 * @param oldPath The original path to the file or directory
 * @param newPath The new path that the copy of the file or directory will be
 *                accessible under
 */
export declare function copy(oldPath: string, newPath: string): boolean;
/**
 * Fetches generic information about a filesystem
 * @param filesystem The path to the filesystem (e.g. `"/ext"` or `"/int"`)
 */
export declare function fsInfo(filesystem: string): FsInfo | undefined;
/**
 * Chooses the next available filename with a numeric suffix in a directory
 * 
 * ```
 * "/ext/example_dir/example_file123.txt"
 *  \______________/ \__________/\_/\__/
 *       dirPath       fileName   |  |
 *                                |  +---- fileExt
 *                                +------- selected by this function
 * ```
 * 
 * @param dirPath The directory to look in
 * @param fileName The base of the filename (the part before the numeric suffix)
 * @param fileExt The extension of the filename (the part after the numeric suffix)
 * @param maxLen The maximum length of the filename with the numeric suffix
 * @returns The base of the filename with the next available numeric suffix,
 *          without the extension or the base directory.
 */
export declare function nextAvailableFilename(dirPath: string, fileName: string, fileExt: string, maxLen: number): string;

// path operations that do not access the filesystem

/**
 * Determines whether the two paths are equivalent. Respects filesystem-defined
 * path equivalence rules.
 */
export declare function arePathsEqual(path1: string, path2: string): boolean;
/**
 * Determines whether a path is a subpath of another path. Respects
 * filesystem-defined path equivalence rules.
 * @param parentPath The parent path
 * @param childPath The child path
 */
export declare function isSubpathOf(parentPath: string, childPath: string): boolean;
