# BadUSB File Format {#badusb_file_format}

## Command syntax

BadUsb app uses extended Duckyscript syntax. It is compatible with classic USB Rubber Ducky 1.0 scripts but provides some additional commands and features, such as custom USB ID, ALT+Numpad input method, SYSRQ command, and more functional keys.

## Script file format

BadUsb app can execute only text scripts from `.txt` files, no compilation is required. Both `\n` and `\r\n` line endings are supported. Empty lines are allowed. You can use spaces or tabs for line indentation.

## Command set

### Comment line

Just a single comment line. The interpreter will ignore all text after the REM command.
| Command | Parameters   | Notes |
| ------- | ------------ | ----- |
| REM     | Comment text |       |

### Delay

Pause script execution by a defined time.
| Command       | Parameters        | Notes                               |
| ------------- | ----------------- | ----------------------------------- |
| DELAY         | Delay value in ms | Single delay                        |
| DEFAULT_DELAY | Delay value in ms | Add delay before every next command |
| DEFAULTDELAY  | Delay value in ms | Same as DEFAULT_DELAY               |

### Special keys

| Command            | Notes            |
| ------------------ | ---------------- |
| DOWNARROW / DOWN   |                  |
| LEFTARROW / LEFT   |                  |
| RIGHTARROW / RIGHT |                  |
| UPARROW / UP       |                  |
| ENTER              |                  |
| DELETE             |                  |
| BACKSPACE          |                  |
| END                |                  |
| HOME               |                  |
| ESCAPE / ESC       |                  |
| INSERT             |                  |
| PAGEUP             |                  |
| PAGEDOWN           |                  |
| CAPSLOCK           |                  |
| NUMLOCK            |                  |
| SCROLLLOCK         |                  |
| PRINTSCREEN        |                  |
| BREAK              | Pause/Break key  |
| PAUSE              | Pause/Break key  |
| SPACE              |                  |
| TAB                |                  |
| MENU               | Context menu key |
| APP                | Same as MENU     |
| Fx                 | F1-F12 keys      |

### Modifier keys

Can be combined with a special key command or a single character.
| Command        | Notes      |
| -------------- | ---------- |
| CONTROL / CTRL |            |
| SHIFT          |            |
| ALT            |            |
| WINDOWS / GUI  |            |
| CTRL-ALT       | CTRL+ALT   |
| CTRL-SHIFT     | CTRL+SHIFT |
| ALT-SHIFT      | ALT+SHIFT  |
| ALT-GUI        | ALT+WIN    |
| GUI-SHIFT      | WIN+SHIFT  |
| GUI-CTRL       | WIN+CTRL   |

## Key hold and release

Up to 5 keys can be hold simultaneously.
| Command | Parameters                      | Notes                                    |
| ------- | ------------------------------- | ---------------------------------------- |
| HOLD    | Special key or single character | Press and hold key until RELEASE command |
| RELEASE | Special key or single character | Release key                              |

## String

| Command  | Parameters  | Notes                                      |
| -------  | ----------- | -----------------                          |
| STRING   | Text string | Print text string                          |
| STRINGLN | Text string | Print text string and press enter after it |

## String delay

Delay between keypresses.
| Command              | Parameters        | Notes                                         |
| -------------------- | ----------------- | --------------------------------------------- |
| STRING_DELAY         | Delay value in ms | Applied once to next appearing STRING command |
| STRINGDELAY          | Delay value in ms | Same as STRING_DELAY                          |
| DEFAULT_STRING_DELAY | Delay value in ms | Apply to every appearing STRING command       |
| DEFAULTSTRINGDELAY   | Delay value in ms | Same as DEFAULT_STRING_DELAY                  |

### Repeat

| Command | Parameters                   | Notes                   |
| ------- | ---------------------------- | ----------------------- |
| REPEAT  | Number of additional repeats | Repeat previous command |

### ALT+Numpad input

On Windows and some Linux systems, you can print characters by holding `ALT` key and entering its code on Numpad.
| Command   | Parameters     | Notes                                                           |
| --------- | -------------- | --------------------------------------------------------------- |
| ALTCHAR   | Character code | Print single character                                          |
| ALTSTRING | Text string    | Print text string using ALT+Numpad method                       |
| ALTCODE   | Text string    | Same as ALTSTRING, presents in some Duckyscript implementations |

### SysRq

Send [SysRq command](https://en.wikipedia.org/wiki/Magic_SysRq_key)
| Command | Parameters       | Notes |
| ------- | ---------------- | ----- |
| SYSRQ   | Single character |       |

## Media keys

Some Media/Consumer Control keys can be pressed with "MEDIA" command

| Command | Parameters                | Notes |
| ------- | ------------------------- | ----- |
| MEDIA   | Media key, see list below |       |

| Key name          | Notes                         |
| ----------------- | ----------------------------- |
| POWER             |                               |
| REBOOT            |                               |
| SLEEP             |                               |
| LOGOFF            |                               |
| EXIT              |                               |
| HOME              |                               |
| BACK              |                               |
| FORWARD           |                               |
| REFRESH           |                               |
| SNAPSHOT          | Take photo in a camera app    |
| PLAY              |                               |
| PAUSE             |                               |
| PLAY_PAUSE        |                               |
| NEXT_TRACK        |                               |
| PREV_TRACK        |                               |
| STOP              |                               |
| EJECT             |                               |
| MUTE              |                               |
| VOLUME_UP         |                               |
| VOLUME_DOWN       |                               |
| FN                | Fn/Globe key on Mac keyboard  |
| BRIGHT_UP         | Increase display brightness   |
| BRIGHT_DOWN       | Decrease display brightness   |

## Fn/Globe key commands (Mac/iPad)

| Command | Parameters                      | Notes |
| ------- | ------------------------------- | ----- |
| GLOBE   | Special key or single character |       |

## Wait for button press

Will wait indefinitely for a button to be pressed
| Command               | Parameters   | Notes                                                                 |
| --------------------- | ------------ | --------------------------------------------------------------------- |
| WAIT_FOR_BUTTON_PRESS | None         | Will wait for the user to press a button to continue script execution |

## USB device ID

You can set the custom ID of the Flipper USB HID device. ID command should be in the **first line** of script, it is executed before script run.

| Command | Parameters                   | Notes |
| ------- | ---------------------------- | ----- |
| ID      | VID:PID Manufacturer:Product |       |

Example:
`ID 1234:abcd Flipper Devices:Flipper Zero`

VID and PID are hex codes and are mandatory. Manufacturer and Product are text strings and are optional.
