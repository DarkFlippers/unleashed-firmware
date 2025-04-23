# Storage module {#js_storage}

The module allows you to access files and directories on the Flipper Zero filesystems. Call the `require` function to load the module before first using its methods:

```js
let storage = require("storage");
```

## Paths

To work with files and folders, you'll need to specify paths to them. File paths have the following structure:

```
/ext/example_subdir_1/example_subdir_2/example_file.txt
\____________________________________/ \__________/ \_/
                dirPath                  fileName  fileExt
```

* **dirPath** — directory path starting with `/int/` (small storage in the MCU's flash memory) or `/ext/` (microSD card storage). Specify the sub-directories containing the file using `/` as a separator between directory names.
* **fileName** — file name.
* **fileExt** — file extension (separated from the file name by a period).

---

# Structures

## FsInfo 

Filesystem information structure.

**Fields**

- totalSpace: Total size of the filesystem, in bytes
- freeSpace: Free space in the filesystem, in bytes

<br>

## FileInfo

File information structure.

**Fields**

- path: Full path (e.g. "/ext/test", returned by `stat`) or file name (e.g. "test", returned by `readDirectory`)
- isDirectory: Returns `true` if path leads to a directory (not a file)
- size: File size in bytes, or 0 in the case of directories
- accessTime: Time of last access as a UNIX timestamp

---

# Classes

## File

This class implements methods for working with file. To get an object of the File class, use the `openFile` method.

<br>

### close()

Closes the file. After this method is called, all other operations related to this file become unavailable.

**Returns**

`true` on success, `false` on failure.

<br>

### isOpen()

**Returns**

`true` if file is currently opened, `false` otherwise.

<br>

### read()

Reads bytes from a file opened in read-only or read-write mode.

**Parameters**

- mode: The data type to interpret the bytes as: a `string` decoded from ASCII data (`"ascii"`), or an `ArrayBuf` (`"binary"`)
- bytes: How many bytes to read from the file

**Returns**

An `ArrayBuf` if the mode is `"binary"`, a `string` if the mode is `ascii`. The number of bytes that was actually read may be fewer than requested.

<br>

### write()

Writes bytes to a file opened in write-only or read-write mode.

**Parameters**

- data: The data to write: a string that will be ASCII-encoded, or an ArrayBuf

**Returns**

The amount of bytes that was actually written.

<br>

### seekRelative()

Moves the R/W pointer forward.

**Parameters**

- bytes: How many bytes to move the pointer forward by

**Returns**

`true` on success, `false` on failure.

<br>

### seekAbsolute()

Moves the R/W pointer to an absolute position inside the file.

**Parameters**

- bytes: The position inside the file

**Returns**

`true` on success, `false` on failure.

<br>

### tell()

Gets the absolute position of the R/W pointer in bytes.

**Returns**

The absolute current position in the file.

<br>

### truncate()

Discards the data after the current position of the R/W pointer in a file opened in either write-only or read-write mode.

**Returns**

`true` on success, `false` on failure.

<br>

### size()

**Returns**

The total size of the file in bytes.

<br>

### eof()

Detects whether the R/W pointer has reached the end of the file.

**Returns**

`true` if end of file reached, `false` otherwise.

<br>

### copyTo()

Copies bytes from the R/W pointer in the current file to the R/W pointer in another file.

**Parameters**

- dest: The file to copy the bytes into
- bytes: The number of bytes to copy

**Returns**

`true` on success, `false` on failure.

---

# Methods

## openFile()

Opens a file.

**Parameters**

- path: The path to the file
- accessMode: How to access the file (`"r"`, `"w"` or `"rw"`)
- openMode: How to open the file (`"open_existing"`, `"open_always"`, `"open_append"`, `"create_new"` or `"create_always"`)

**Returns**

A `File` object on success, `undefined` otherwise.

<br>

## fileExists()

Detects whether a file exists.

**Parameters**

- path: The path to the file

**Returns**

`true` if file exists, `false` otherwise.

<br>

## arePathsEqual()

Determines whether the two paths are equivalent. Respects filesystem-defined path equivalence rules.

**Parameters**

- path1: The first path for comparison
- path2: The second path for comparison

**Returns**

`true` if path1 and path2 are equals, `false` otherwise.

<br>

## isSubpathOf()

Determines whether a path is a subpath of another path. Respects filesystem-defined path equivalence rules.

**Parameters**

- parentPath: The parent path
- childPath: The child path

**Returns**

`true` if path1 and path2 are equals, `false` otherwise.

<br>

## fileOrDirExists()

Detects whether a file or a directory exists.

**Parameters**

- path: The path to the file or directory

**Returns**

`true` if file/directory exists, `false` otherwise.

**Example**
```js
if (storage.fileOrDirExists("/ext/test_dir")) {
    print("Test directory exists");
}
```

<br>

## fsInfo()

Fetches generic information about a filesystem.

**Parameters**

- filesystem: The path to the filesystem (e.g. `"/ext"` or `"/int"`)

**Returns**

A `fsInfo` structure or `undefined` on failure.

**Example**
```js
let fsinfo = storage.fsInfo("/ext");
if (fsinfo === undefined) {
    print("Filesystem access error");
} else {
    print("Free space on the /ext filesystem:", fsinfo.freeSpace);
}
```

<br>

## stat()

Acquires metadata about a file or directory.

**Parameters**

- path: The path to the file or directory

**Returns**

A `FileInfo` structure or `undefined` on failure.

**Example**
```js
let finfo = storage.stat("/ext/test_file.txt");
if (finfo === undefined) {
    print("File not found");
} else {
    print("File size:", finfo.size);
}
```

<br>

## directoryExists()

Detects whether a directory exists.

**Parameters**

- path: The path to the directory

**Returns**

`true` if directory exists, `false` otherwise.

<br>

## makeDirectory()

Creates an empty directory.

**Parameters**

- path: The path to the new directory

**Returns**

`true` on success, `false` on failure.

<br>

## readDirectory()

Reads the list of files in a directory.

**Parameters**

- path: The path to the directory

**Returns**

Array of `FileInfo` structures with directory entries, or `undefined` on failure.

<br>

## nextAvailableFilename()

Chooses the next available filename with a numeric suffix in a directory.
```
/ext/example_dir/example_file123.txt
\______________/ \__________/\_/ \_/
    dirPath        fileName   |   |
                              |   +--- fileExt
                              +------- suffix selected by this function
```

**Parameters**

- dirPath: The directory to look in
- fileName: The base of the filename (the part before the numeric suffix)
- fileExt: The extension of the filename (the part after the numeric suffix)
- maxLen: The maximum length of the filename with the numeric suffix

**Returns**

The base of the filename with the next available numeric suffix, without the extension or the base directory.

<br>

## copy()

Copies a file or recursively copies a possibly non-empty directory.

**Parameters**

- oldPath: The original path to the file or directory
- newPath: The new path that the copy of the file or directory will be accessible under

**Returns**

`true` on success, `false` on failure.

**Example**
```js
if (storage.copy("/ext/src_file.txt", "/ext/dst_file.txt")) {
    print("File copied");
}
```

<br>

## rename()

Renames or moves a file or directory.

**Parameters**

- oldPath: The old path to the file or directory
- newPath: The new path that the file or directory will become accessible

**Returns**

`true` on success, `false` on failure.

**Example**
```js
if (storage.rename("/ext/src_file.txt", "/ext/dst_file.txt")) {
    print("File moved");
}
```

<br>

## remove()

Removes a file or an empty directory.

**Parameters**

- path: The path to the file or directory

**Returns**

`true` on success, `false` on failure.

**Example**
```js
let path = "/ext/test_file.txt";

file = storage.openFile(path, "w", "create_always");
file.write("Test");
file.close();
print("File created");

if (storage.remove(path)) {
    print("File removed");
}
```

<br>

## rmrf()

Removes a file or recursively removes a possibly non-empty directory.

**Parameters**

- path: The path to the file or directory

**Returns**

`true` on success, `false` on failure.

**Example**
```js
let path = "/ext/test_dir";

if (storage.rmrf(path)) {
    print("Directory removed");
}
```
