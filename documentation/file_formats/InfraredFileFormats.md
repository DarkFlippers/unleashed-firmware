# Infrared Flipper File Formats {#infrared_file_format}

## Supported protocols list for "type: parsed"

```
    NEC
    NECext
    NEC42
    NEC42ext
    Samsung32
    RC6
    RC5
    RC5X
    SIRC
    SIRC15
    SIRC20
    Kaseikyo
    RCA
```

## Infrared Remote File Format

### Example

    Filetype: IR signals file
    Version: 1
    #
    name: Button_1
    type: parsed
    protocol: NECext
    address: EE 87 00 00
    command: 5D A0 00 00
    #
    name: Button_2
    type: raw
    frequency: 38000
    duty_cycle: 0.330000
    data: 504 3432 502 483 500 484 510 502 502 482 501 485 509 1452 504 1458 509 1452 504 481 501 474 509 3420 503
    #
    name: Button_3
    type: parsed
    protocol: SIRC
    address: 01 00 00 00
    command: 15 00 00 00

### Description

Filename extension: `.ir`

This file format is used to store an infrared remote that consists of an arbitrary number of buttons.
Each button is separated from others by a comment character (`#`) for better readability.

Known protocols are represented in the `parsed` form, whereas non-recognized signals may be saved and re-transmitted as `raw` data.

#### Version history

1. Initial version.

#### Format fields

| Name       | Use    | Type   | Description                                                                                                                                   |
| ---------- | ------ | ------ | --------------------------------------------------------------------------------------------------------------------------------------------- |
| name       | both   | string | Name of the button. Only printable ASCII characters are allowed.                                                                              |
| type       | both   | string | Type of the signal. Must be `parsed` or `raw`.                                                                                                |
| protocol   | parsed | string | Name of the infrared protocol. Refer to `ir` console command for the complete list of supported protocols.                                    |
| address    | parsed | hex    | Payload address. Must be 4 bytes long.                                                                                                        |
| command    | parsed | hex    | Payload command. Must be 4 bytes long.                                                                                                        |
| frequency  | raw    | uint32 | Carrier frequency, in Hertz, usually 38000 Hz.                                                                                                |
| duty_cycle | raw    | float  | Carrier duty cycle, usually 0.33.                                                                                                             |
| data       | raw    | uint32 | Raw signal timings, in microseconds between logic level changes. Individual elements must be space-separated. Maximum timings amount is 1024. |

## Infrared Library File Format

### Examples

- [TV Universal Library](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/main/infrared/resources/infrared/assets/tv.ir)
- [A/C Universal Library](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/main/infrared/resources/infrared/assets/ac.ir)
- [Audio Universal Library](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/main/infrared/resources/infrared/assets/audio.ir)

### Description

Filename extension: `.ir`

This file format is used to store universal remote libraries. It is identical to the previous format, differing only in the `Filetype` field.
It also has predefined button names for each universal library type, so that the universal remote application can understand them.
See [Universal Remotes](../UniversalRemotes.md) for more information.

### Version history

1. Initial version.

## Infrared Test File Format

### Examples

See [Infrared Unit Tests](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/debug/unit_tests/resources/unit_tests/infrared) for various examples.

### Description

Filename extension: `.irtest`

This file format is used to store technical test data that is too large to keep directly in the firmware.
It is mostly similar to the two previous formats, with the main difference being the addition of the parsed signal arrays.

Each infrared protocol must have corresponding unit tests complete with an `.irtest` file.

Known protocols are represented in the `parsed_array` form, whereas raw data has the `raw` type.
Note: a single parsed signal must be represented as an array of size 1.

### Version history

1. Initial version.

#### Format fields

| Name       | Use          | Type   | Description                                                      |
| ---------- | ------------ | ------ | ---------------------------------------------------------------- |
| name       | both         | string | Name of the signal. Only printable ASCII characters are allowed. |
| type       | both         | string | Type of the signal. Must be `parsed_array` or `raw`.             |
| count      | parsed_array | uint32 | The number of parsed signals in an array. Must be at least 1.    |
| protocol   | parsed_array | string | Same as in previous formats.                                     |
| address    | parsed_array | hex    | Ditto.                                                           |
| command    | parsed_array | hex    | Ditto.                                                           |
| repeat     | parsed_array | bool   | Indicates whether the signal is a repeated button press.         |
| frequency  | raw          | uint32 | Same as in previous formats.                                     |
| duty_cycle | raw          | float  | Ditto.                                                           |
| data       | raw          | uint32 | Ditto.                                                           |

#### Signal names

The signal names in an `.irtest` file follow a convention `<name><test_number>`, where the name is one of:

- decoder_input
- decoder_expected
- encoder_decoder_input,

and the number is a sequential integer: 1, 2, 3, etc., which produces names like `decoder_input1`, `encoder_decoder_input3`, and so on.

| Name                  | Type         | Description                                                                                           |
| --------------------- | ------------ | ----------------------------------------------------------------------------------------------------- |
| decoder_input         | raw          | A raw signal containing the decoder input. Also used as the expected encoder output.               |
| decoder_expected      | parsed_array | An array of parsed signals containing the expected decoder output. Also used as the encoder input. |
| encoder_decoder_input | parsed_array | An array of parsed signals containing both the encoder-decoder input and expected output.             |

See [Unit Tests](../UnitTests.md) for more info.
