# Number Input {#example_number_input}

Simple keyboard that limits user inputs to a full number (integer). Useful to enforce correct entries without the need for intense validations after a user input. 

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_number_input).

## General principle

Definition of min/max values is required. Numbers are of type int32_t. If negative numbers are allowed within min - max, an additional button is displayed to switch the sign between + and -. 

It is also possible to define a header text, as shown in this example app with the 3 different input options. 