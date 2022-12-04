# flipperzero-sentry-safe-plugin

Flipper zero exploiting vulnerability to open any Sentry Safe and Master Lock electronic safe without any pin code.

[Vulnerability described here](https://github.com/H4ckd4ddy/bypass-sentry-safe)

### Installation

- Download [last release fap file](https://github.com/H4ckd4ddy/flipperzero-sentry-safe-plugin/releases/latest)
- Copy fap file to the apps folder of your flipper SD card

### Usage

- Start "Sentry Safe" plugin
- Place wires as described on the plugin screen
- Press enter
- Open safe

### Build

- Recursively clone your base firmware (official or not)
- Clone this repository in `applications_user`
- Build with `./fbt fap_dist APPSRC=applications_user/flipperzero-sentry-safe-plugin`
- Retreive builed fap in dist subfolders

(More info about build tool [here](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/documentation/fbt.md))
