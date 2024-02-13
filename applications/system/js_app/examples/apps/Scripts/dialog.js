let dialog = require("dialog");

let result1 = dialog.message("Dialog demo", "Press OK to start");
print(result1);

let dialog_params = ({
    header: "Test_header",
    text: "Test_text",
    button_left: "Left",
    button_right: "Right",
    button_center: "OK"
});

let result2 = dialog.custom(dialog_params);
if (result2 === "") {
    print("Back is pressed");
} else {
    print(result2, "is pressed");
}
