# File Formats for Flipper's SubGhz Subsystem

## `.sub` File Format

Flipper uses `.sub` files to store SubGhz transmissions. They are text files in Flipper File Format. `.sub` files can contain either a SubGhz Key with a certain protocol or SubGhz RAW data.

A `.sub` files consist of 3 parts:

- **header**: contains file type, version, and frequency
- **preset information**: preset type and, in case of a custom preset, transceiver configuration data
- **protocol and its data**: contains protocol name and its specific data, such as key, bit length, etc., or RAW data

Flipper's SubGhz subsystem uses presets to configure radio transceiver. Presets are used to configure modulation, bandwidth, filters, etc. There are several presets available in stock firmware, and there is a way to create custom presets. See [SubGhz Presets](#adding-a-custom-preset) for more details.

## Header Format

Header is a mandatory part of `.sub` file. It contains file type, version, and frequency.

| Field         | Type      | Description                                                       |
| ---           | ---       | ---                                                               |
| `Filetype`    | string    | Filetype of subghz file format, must be `Flipper SubGhz Key File` |
| `Version`     | uint      | Version of subghz file format, current version is 1               |
| `Frequency`   | uint      | Frequency in Hertz                                                |

## Preset Information

Preset information is a mandatory part of `.sub` file. It contains preset type and, in case of custom preset, transceiver configuration data.

When using one of the standard presets, only `Preset` field is required. When using custom preset, `Custom_preset_module` and `Custom_preset_data` fields are required.

| Field                     | Description                                                                                                                           |
| ---                       | ---                                                                                                                                   |
| `Preset`                  | Radio preset name (configures modulation, bandwidth, filters, etc.). When using a custom preset, must be `FuriHalSubGhzPresetCustom`  |
| `Custom_preset_module`    | Transceiver identifier, `CC1101` for Flipper Zero                                                                                     |
| `Custom_preset_data`      | Transceiver configuration data                                                                                                        |

Built-in presets:

- `FuriHalSubGhzPresetOok270Async` - On/Off Keying, 270kHz bandwidth, async(IO throw GP0)
- `FuriHalSubGhzPresetOok650Async` - On/Off Keying, 650kHz bandwidth, async(IO throw GP0)
- `FuriHalSubGhzPreset2FSKDev238Async` - 2 Frequency Shift Keying, deviation 2kHz, 270kHz bandwidth, async(IO throw GP0)
- `FuriHalSubGhzPreset2FSKDev476Async` - 2 Frequency Shift Keying, deviation 47kHz, 270kHz bandwidth, async(IO throw GP0)

### Transceiver Configuration Data

Transceiver configuration data is a string of bytes, encoded in hex format, separated by spaces. For CC1101 data structure is: `XX YY XX YY .. 00 00 ZZ ZZ ZZ ZZ ZZ ZZ ZZ ZZ`, where:

- XX holds register address,
- YY contains register value,
- 00 00: marks register block end,
- `ZZ ZZ ZZ ZZ ZZ ZZ ZZ ZZ`: 8 byte PA table (Power amplifier ramp table).

More details can be found in [CC1101 datasheet](https://www.ti.com/lit/ds/symlink/cc1101.pdf) and `furi_hal_subghz` code.

## File Data

`.sub` file data section contains either key data — protocol name and its specific data, bit length, etc., or RAW data — an array of signal timings, recorded without any protocol-specific processing.

### Key Files

`.sub` files with key data files contain protocol name and its specific data, such as key value, bit length, etc.
Check out protocol registry for full list of supported protocol names.

Example of key data block in Princeton format:

```
...
Protocol: Princeton
Bit: 24
Key: 00 00 00 00 00 95 D5 D4
TE: 400
```

Protocol-specific fields in this example:

| Field | Description                       |
| ---   | ---                               |
| `Bit` | Princeton payload length, in bits |
| `Key` | Princeton payload data            |
| `TE`  | Princeton quantization interval   |

This file may contain additional fields, more details on available fields can be found in subghz protocols library.

### RAW Files

RAW `.sub` files contain raw signal data that is not processed through protocol-specific decoding. These files are useful for testing purposes, or for sending data that is not supported by any known protocol.

For RAW files, 2 fields are required:

 * `Protocol`, must be `RAW`
 * `RAW_Data`, contains an array of timings, specified in micro seconds. Values must be non-zero, start with a positive number, and interleaved (change sign with each value). Up to 512 values per line. Can be specified multiple times to store multiple lines of data.

Example of RAW data:

    Protocol: RAW
    RAW_Data: 29262 361 -68 2635 -66 24113 -66 11 ...


Long payload not fitting into internal memory buffer and consisting of short duration timings (<10us) may not be read fast enough from SD Card. That might cause signal transmission to stop before reaching the end of the payload. Ensure that your SD Card has good performance before transmitting long or complex RAW payloads.


## File Examples

### Key File, Standard Preset

    Filetype: Flipper SubGhz Key File
    Version: 1
    Frequency: 433920000
    Preset: FuriHalSubGhzPresetOok650Async
    Protocol: Princeton
    Bit: 24
    Key: 00 00 00 00 00 95 D5 D4
    TE: 400


### Key File, Custom Preset

    Filetype: Flipper SubGhz Key File
    Version: 1
    Frequency: 433920000
    Preset: FuriHalSubGhzPresetCustom
    Custom_preset_module: CC1101
    Custom_preset_data: 02 0D 03 07 08 32 0B 06 14 00 13 00 12 30 11 32 10 17 18 18 19 18 1D 91 1C 00 1B 07 20 FB 22 11 21 B6 00 00 00 C0 00 00 00 00 00 00
    Protocol: Princeton
    Bit: 24
    Key: 00 00 00 00 00 95 D5 D4
    TE: 400

### RAW File, Standard Preset

    Filetype: Flipper SubGhz RAW File
    Version: 1
    Frequency: 433920000
    Preset: FuriHalSubGhzPresetOok650Async
    Protocol: RAW
    RAW_Data: 29262 361 -68 2635 -66 24113 -66 11 ...
    RAW_Data: -424 205 -412 159 -412 381 -240 181 ...
    RAW_Data: -1448 361 -17056 131 -134 233 -1462 131 -166 953 -100 ...

### RAW File, Custom Preset

    Filetype: Flipper SubGhz RAW File
    Version: 1
    Frequency: 433920000
    Preset: FuriHalSubGhzPresetCustom
    Custom_preset_module: CC1101
    Сustom_preset_data: 02 0D 03 07 08 32 0B 06 14 00 13 00 12 30 11 32 10 17 18 18 19 18 1D 91 1C 00 1B 07 20 FB 22 11 21 B6 00 00 00 C0 00 00 00 00 00 00
    Protocol: RAW
    RAW_Data: 29262 361 -68 2635 -66 24113 -66 11 ...
    RAW_Data: -424 205 -412 159 -412 381 -240 181 ...
    RAW_Data: -1448 361 -17056 131 -134 233 -1462 131 -166 953 -100 ...

# SubGhz Configuration Files

SubGhz application provides support for adding extra radio presets and additional keys for decoding transmissions in certain protocols.

## SubGhz `keeloq_mfcodes_user` File

This file contains additional manufacturer keys for Keeloq protocol. It is used to decode Keeloq transmissions.
This file is loaded at subghz application start and is located at path `/ext/subghz/assets/keeloq_mfcodes_user`.

### File Format

File contains a header and a list of manufacturer keys.

File header format:

| Field         | Type      | Description                                                           |
| ---           |           | ---                                                                   |
| `Filetype`    | string    | SubGhz Keystore file format, always `Flipper SubGhz Keystore File`    |
| `Version`     | uint      | File format version, 0                                                |
| `Encryption`  | uint      | File encryption: for user-provided file, set to 0 (disabled)          |

Following the header, file contains a list of user-provided manufacture keys, one key per line.
For each key, a name and encryption method must be specified, according to comment in file header. More information can be found in keeloq decoder source code.

### Example

    # to use manual settings and prevent them from being deleted on upgrade, rename *_user.example files to *_user
    # for adding manufacture keys
    # AABBCCDDEEFFAABB:X:NAME
    # AABBCCDDEEFFAABB - man 64 bit
    # X - encryption method:
    # - 0 - iterates over both previous and man in direct and reverse byte sequence
    # - 1 - Simple Learning
    # - 2 - Normal_Learning
    # - 3 - Secure_Learning
    # - 4 - Magic_xor_type1 Learning
    # 
    # NAME - name (string without spaces) max 64 characters long
    Filetype: Flipper SubGhz Keystore File
    Version: 0
    Encryption: 0
    AABBCCDDEEFFAABB:1:Test1
    AABBCCDDEEFFAABB:1:Test2


## SubGhz `setting_user` File

This file contains additional radio presets and frequencies for SubGhz application. It is used to add new presets and frequencies for existing presets. This file is be loaded on subghz application start and is located at path `/ext/subghz/assets/setting_user`.

### File Format

File contains a header, basic options, and optional lists of presets and frequencies.

Header must contain following fields:

- `Filetype`: SubGhz setting file format, must be `Flipper SubGhz Setting File`.
- `Version`: file format version, current is `1`.

#### Basic Settings

- `Add_standard_frequencies`: bool, flag indicating whether to load standard frequencies shipped with firmware. If set to `false`, only frequencies specified in this file will be used.
- `Default_frequency`: uint, default frequency used in SubGhz application.

#### Adding More Frequencies

- `Frequency`: uint — additional frequency for the subghz application frequency list. Used in Read and Read RAW. You can specify multiple frequencies, one per line.

#### Adding More Hopper Frequencies

- `Hopper_frequency`: uint — additional frequency for subghz application frequency hopping. Used in Frequency Analyzer. You can specify multiple frequencies, one per line.

Repeating same frequency will cause Flipper to listen on this frequency more often.

#### Adding a Custom Preset

You can have as many presets as you want. Presets are embedded into `.sub` files, so another Flipper can load them directly from that file.
Each preset is defined by following fields:

| Field                     | Description                                                                                                           |
| ---                       | ---                                                                                                                   |
| `Custom_preset_name`      | string, preset name that will be shown in SubGHz application                                                          |
| `Custom_preset_module`    | string, transceiver identifier. Set to `CC1101` for Flipper Zero                                                      |
| `Custom_preset_data`      | transceiver configuration data. See [Transceiver Configuration Data](#transceiver-configuration-data) for details.    |

### Example

```
# to use manual settings and prevent them from being deleted on upgrade, rename *_user.example files to *_user
Filetype: Flipper SubGhz Setting File
Version: 1

# Add Standard frequencies for your region
Add_standard_frequencies: true

# Default Frequency: used as default for "Read" and "Read Raw"
Default_frequency: 433920000

# Frequencies used for "Read", "Read Raw" and "Frequency Analyzer"
Frequency: 300000000
Frequency: 310000000
Frequency: 320000000

# Frequencies used for hopping mode (keep this list small or flipper will miss signal)
Hopper_frequency: 300000000
Hopper_frequency: 310000000
Hopper_frequency: 310000000

# Custom preset
# format for CC1101 "Custom_preset_data:" XX YY XX YY .. 00 00 ZZ ZZ ZZ ZZ ZZ ZZ ZZ ZZ, where: XX-register, YY - register data, 00 00 - end load register, ZZ - 8 byte Pa table register

#Custom_preset_name: AM_1
Custom_preset_module: CC1101
Custom_preset_data: 02 0D 03 07 08 32 0B 06 14 00 13 00 12 30 11 32 10 17 18 18 19 18 1D 91 1C 00 1B 07 20 FB 22 11 21 B6 00 00 00 C0 00 00 00 00 00 00

#Custom_preset_name: AM_2
#Custom_preset_module: CC1101
#Custom_preset_data: 02 0D 03 07 08 32 0B 06 14 00 13 00 12 30 11 32 10 17 18 18 19 18 1D 91 1C 00 1B 07 20 FB 22 11 21 B6 00 00 00 C0 00 00 00 00 00 00
```
