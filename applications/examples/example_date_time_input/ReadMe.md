# Date/Time Input {#example_date_time_input}

Simple view that allows the user to adjust a date and/or time.

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_date_time_input).

## General principle

Callbacks can be defined for every time a value is edited (useful for application-specific bounds checking or validation) and for when the user is done editing (back button is pressed). The provided DateTime object is used both as the initial value and as the place where the result is stored.

The fields which the user is allowed to edit can be defined using `date_time_input_set_editable_fields()`. Disabled fields are shown but aren't able to be selected and don't have an outer box. If all fields are disabled, the view is read-only and no cursor will be shown.
