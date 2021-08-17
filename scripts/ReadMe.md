# About

This folder contains differnt scripts that automates routine actions.
Flashing scripts are based on cli version of [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).
You will need to add STM32_Programmer_CLI to your path to use them.

# Flashing empty MCU/Flipper

Always flash your device in the folllowing sequence:

- OTP (Only on empty MCU)
- Core2 firmware
- Core1 firmware
- Option Bytes

## Otp flashing

!!! Flashing incorrect OTP may permanently brick your device !!!

Normally OTP data generated and flashed at factory.
In case if MCU was replaced you'll need correct OTP data to be able to use companion applications.
Use `otp.py` to generate OTP data and `flash_otp_version_*` to flash OTP zone.
You will need exact main board revision to genrate OTP data. It can be found on main PCB.

!!! Flashing incorrect OTP may permanently brick your device !!!

## Core2 flashing

Script blindly updates FUS and Radiostack. This operation is going to corrupt bootloader and firmware.
Reflash Core1 after Core2.

## Core1 flashing

Script compiles and flashes both bootloader and firmware.

## Option Bytes

!!! Setting incorrect Otion Bytes may brick your MCU !!!

Defaults are mostly OK, but there are couple things that we'd like to tune.
Also OB may be damaged, so we've made couple scripts to check and set option bytes.

!!! Setting incorrect Otion Bytes may brick your MCU !!!

Checking option bytes:

```bash
ob.py check
```

Setting option bytes:

```bash
ob.py set
```
