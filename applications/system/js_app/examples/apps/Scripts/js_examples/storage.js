let storage = require("storage");
let path = "/ext/storage.test";

function arraybuf_to_string(arraybuf) {
    let string = "";
    let data_view = Uint8Array(arraybuf);
    for (let i = 0; i < data_view.length; i++) {
        string += chr(data_view[i]);
    }
    return string;
}

print("File exists:", storage.exists(path));

print("Writing...");
// write(path, data, offset)
// If offset is specified, the file is not cleared, content is kept and data is written at specified offset
// Takes both strings and array buffers
storage.write(path, "Hello ");

print("File exists:", storage.exists(path));

// Append will create the file even if it doesnt exist!
// Takes both strings and array buffers
storage.append(path, "World!");

print("Reading...");
// read(path, size, offset)
// If no size specified, total filesize is used
// If offset is specified, size is capped at (filesize - offset)
let data = storage.read(path);
// read returns an array buffer, to allow proper usage of raw binary data
print(arraybuf_to_string(data));

print("Removing...")
storage.remove(path);

print("Done")

// There's also:
// storage.copy(old_path, new_path);
// storage.move(old_path, new_path);
// storage.mkdir(path);