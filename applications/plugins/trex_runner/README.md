# t-rex-runner
Flipper Zero port of Chrome's running T-rex game

## Compiling

You need a full source of the [Flipper Zero firmware](https://github.com/flipperdevices/flipperzero-firmware/tree/release),
take the `release` branch to build for the stable release.

Copy or symlink this folder as `flipperzero-firmware/applications_user/t-rex-runner`.

Run the build from the root of the firmware folder:
```
./fbt firmware_t-rex-runner
```

If you have Flipper Zero attached to USB, you can immediately compile and run the app on device:
```
./fbt launch_app APPSRC=applications_user/t-rex-runner
```
