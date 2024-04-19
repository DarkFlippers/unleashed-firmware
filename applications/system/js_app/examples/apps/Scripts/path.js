let storage = require("storage");

print("script has __dirpath of" + __dirpath);
print("script has __filepath of" + __filepath);
if (storage.exists(__dirpath + "/math.js")) {
    print("math.js exist here.");
} else {
    print("math.js does not exist here.");
}