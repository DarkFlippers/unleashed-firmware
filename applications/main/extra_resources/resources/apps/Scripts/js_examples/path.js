let storage = require("storage");

print("script has __dirname of" + __dirname);
print("script has __filename of" + __filename);
if (storage.fileExists(__dirname + "/math.js")) {
    print("math.js exist here.");
} else {
    print("math.js does not exist here.");
}
