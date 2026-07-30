let storage = require("storage");
let path = "/ext/storage.test";

print("File exists:", storage.fileExists(path));

print("Writing...");
let file = storage.openFile(path, "w", "create_always");
file.write("Hello ");
file.close();

print("File exists:", storage.fileExists(path));

file = storage.openFile(path, "w", "open_append");
file.write("World!");
file.close();

print("Reading...");
file = storage.openFile(path, "r", "open_existing");
let text = file.read("ascii", 128);
file.close();
print(text);

print("Removing...")
storage.remove(path);

print("Done")

// You don't need to close the file after each operation, this is just to show some different ways to use the API
// There's also many more functions and options, check type definitions in firmware repo