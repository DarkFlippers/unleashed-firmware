let submenu = require("submenu");

submenu.addItem("Item 1", 0);
submenu.addItem("Item 2", 1);
submenu.addItem("Item 3", 2);

submenu.setHeader("Select an option:");

let result = submenu.show();
// Returns undefined when pressing back
print("Result:", result);
