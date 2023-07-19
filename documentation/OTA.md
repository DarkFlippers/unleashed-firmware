# Executing code from RAM

In Flipper firmware, we have a special boot mode that loads a specially crafted system image into RAM and transfers control to it. System image executing in RAM has full write access to Flipper's entire flash memory — something that's not possible when running main code from the same flash.

We leverage that boot mode to perform OTA firmware updates, including operations on a radio stack running on the second MCU core.

# How does Flipper OTA work?

Installation of OTA updates goes through 3 stages:

## 1. Backing up internal storage (`/int`)

It is a special partition of Flipper's flash memory, taking up all available space not used by the firmware code. Newer versions of firmware may be of different size, and simply installing them would cause flash repartitioning and data loss.

So, before taking any action on the firmware, we back up the current configuration from `/int` into a plain tar archive on the SD card.

## 2. Performing device update

The main firmware loads an updater image — a customized build of the main Flipper firmware — into RAM and runs it. Updater performs operations on system flash as described by an Update manifest file.

First, if there's a Radio stack image bundled with the update, updater compares its version with the currently installed one. If they don't match, updater performs stack deinstallation followed by writing and installing a new one. The installation itself is performed by proprietary software FUS running on Core2, and leads to a series of system restarts.

Then, updater validates and corrects Option Bytes — a special memory region containing low-level configuration for Flipper's MCU.

After that, updater loads a `.dfu` file with firmware to be flashed, checks its integrity using CRC32, writes it to system flash and validates written data.

## 3. Restoring internal storage and updating resources

After performing operations on flash memory, the system restarts into newly flashed firmware. Then it performs restoration of previously backed up `/int` contents.

If the update package contains an additional resources archive, it is extracted onto the SD card.

# Update manifest

An update package comes with a manifest that contains a description of its contents. The manifest is in Flipper File Format — a simple text file, comprised of key-value pairs.

## Mandatory fields

An update manifest must contain the following keys in the given order:

- **Filetype**: a constant string, "Flipper firmware upgrade configuration".

- **Version**: manifest version. The current value is 2.

- **Info**: arbitrary string, describing package contents.

- **Target**: hardware revision for which the package is built.

- **Loader**: file name of stage 2 loader that is executed from RAM.

- **Loader CRC**: CRC32 of loader file. Note that it is represented in little-endian hex.

## Optional fields

Other fields may have empty values. In this case, updater skips all operations related to these values.

- **Radio**: file name of radio stack image, provided by STM.

- **Radio address**: address to install the radio stack at. It is specified in Release Notes by STM.

- **Radio version**: radio major, minor and sub versions followed by branch, release and stack type packed into 6 hex-encoded bytes.

- **Radio CRC**: CRC32 of radio image.

- **Resources**: file name of TAR archive with resources to be extracted onto the SD card.

- **OB reference**, **OB mask**, **OB write mask**: reference values for validating and correcting option bytes.

# OTA update error codes

We designed the OTA update process to be as fail-safe as possible. We don't start any risky operations before validating all related pieces of data to ensure we don't leave the device in a partially updated, or bricked, state.

Even if something goes wrong, updater allows you to retry failed operations and reports its state with an error code. These error codes have an `[XX-YY]` format, where `XX` encodes the failed operation, and `YY` contains extra details on its progress where the error occurred.

|    Stage description    |   Code | Progress   | Description                                |
| :---------------------: | -----: | ---------- | ------------------------------------------ |
| Loading update manifest |  **1** | **13**     | Updater reported hardware version mismatch |
|                         |        | **20**     | Failed to get saved manifest path          |
|                         |        | **30**     | Failed to load manifest                    |
|                         |        | **40**     | Unsupported update package version         |
|                         |        | **50**     | Package has mismatching HW target          |
|                         |        | **60**     | Missing DFU file                           |
|                         |        | **80**     | Missing radio firmware file                |
|     Backing up LFS      |  **2** | **0-100**  | FS read/write error                        |
|    Checking radio FW    |  **3** | **0-99**   | Error reading radio firmware file          |
|                         |        | **100**    | CRC mismatch                               |
|  Uninstalling radio FW  |  **4** | **0**      | SHCI Delete command error                  |
|                         |        | **80**     | Error awaiting command status              |
|    Writing radio FW     |  **5** | **0-100**  | Block read/write error                     |
|   Installing radio FW   |  **6** | **10**     | SHCI Install command error                 |
|                         |        | **80**     | Error awaiting command status              |
|      Core2 is busy      |  **7** | **10**     | Couldn't start C2                          |
|                         |        | **20**     | Failed to switch C2 to FUS mode            |
|                         |        | **30**     | Error in FUS operation                     |
|                         |        | **50**     | Failed to switch C2 to stack mode          |
|  Validating opt. bytes  |  **8** | **yy**     | Option byte code                           |
|    Checking DFU file    |  **9** | **0**      | Error opening DFU file                     |
|                         |        | **1-98**   | Error reading DFU file                     |
|                         |        | **99-100** | Corrupted DFU file                         |
|      Writing flash      | **10** | **0-100**  | Block read/write error                     |
|    Validating flash     | **11** | **0-100**  | Block read/write error                     |
|      Restoring LFS      | **12** | **0-100**  | FS read/write error                        |
|   Updating resources    | **13** | **0-100**  | SD card read/write error                   |

# Building update packages

## Full package

To build a full update package, including firmware, radio stack and resources for the SD card, run:

`./fbt COMPACT=1 DEBUG=0 updater_package`

## Minimal package

To build a minimal update package, including only firmware, run:

`./fbt COMPACT=1 DEBUG=0 updater_minpackage`

## Customizing update bundles

Default update packages are built with Bluetooth Light stack.
You can pick a different stack if your firmware version supports it, and build a bundle with it by passing the stack type and binary name to `fbt`:

`./fbt updater_package COMPACT=1 DEBUG=0 COPRO_OB_DATA=scripts/ob_custradio.data COPRO_STACK_BIN=stm32wb5x_BLE_Stack_full_fw.bin COPRO_STACK_TYPE=ble_full`

Note that `COPRO_OB_DATA` must point to a valid file in the `scripts` folder containing reference Option Byte data matching your radio stack type.

In certain cases, you might have to confirm your intentions by adding `COPRO_DISCLAIMER=...` to the build command line.

## Building partial update packages

You can customize package contents by calling `scripts/update.py` directly.
For example, to build a package only for installing BLE FULL stack:

```shell
scripts/update.py generate \
  -t f7 -d r13.3_full -v "BLE FULL 13.3" \
  --stage dist/f7/flipper-z-f7-updater-*.bin \
  --radio lib/stm32wb_copro/firmware/stm32wb5x_BLE_Stack_full_fw.bin \
  --radiotype ble_full
```

For the full list of options, check `scripts/update.py generate` help.
