# About

This folder contains supplementary scripts that automates routine actions.
Flashing scripts are based on cli version of [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).
You will need to add STM32_Programmer_CLI to your path to use them.

# Flashing empty MCU/Flipper

Always flash your device in the following sequence:

- OTP (Only on empty MCU)
- Core1 and Core2 firmware flashing
- Option Bytes

## Otp flashing

!!! Flashing incorrect OTP may permanently brick your device !!!

Normally OTP data generated and flashed at the factory.
In case if MCU was replaced you'll need correct OTP data to be able to use companion applications.
Use `otp.py` to generate and flash OTP data.
You will need exact main board revision to generate OTP data. It can be found on main PCB.
Also display type, region and etc...

!!! Flashing incorrect OTP may permanently brick your device !!!

## Core1 and Core2 firmware flashing

Core2 goes first, then Core1.
Never flash FUS or you will lose your job, girlfriend and keys in secure enclave.

## Option Bytes

!!! Setting incorrect Option Bytes may brick your MCU !!!

Defaults are mostly OK, but there are couple things that we'd like to tune.
Also, OB may be damaged, so we've made couple scripts to check and set option bytes.

!!! Setting incorrect Option Bytes may brick your MCU !!!

Checking option bytes:

```bash
ob.py check
```

Setting option bytes:

```bash
ob.py set
```

# Assets delivery

Build the firmware and run in the root folder of the repo:

```bash
python scripts/storage.py -p <flipper_cli_port> send build/latest/resources /ext
```


# Slideshow creation

Put fullscreen slideshow frames in .png format into `assets/slideshow/my_show` folder, named frame_xx.png, where xx is zero-padded frame number, starting with #0.

Then run 

```bash
python scripts/slideshow.py -i assets/slideshow/my_show/ -o assets/slideshow/my_show/.slideshow
```

Upload generated .slideshow file to Flipper's internal storage and restart it.
