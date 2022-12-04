# How to change Flipper name:
## [Feature & Documentation By Unleashed/xMasterX](https://github.com/DarkFlippers/unleashed-firmware/documentation/CustomFlipperName.md)

## Instruction
1. Read [How to build](https://github.com/RogueMaster/flipperzero-firmware/blob/dev/documentation/HowToBuild.md) and [How to install](https://github.com/RogueMaster/flipperzero-firmware/blob/dev/documentation/HowToInstall.md) to know how to build and install firmware
2. Follow how to build instructions to prepare all things before continuing
3. Run release build to verify all is ok - `./fbt COMPACT=1 DEBUG=0 updater_package`
4. Clear build files - `./fbt COMPACT=1 DEBUG=0 updater_package -c`
5. Run command with extra environment var before `./fbt` that variable should contain your custom name in alphanumeric characters - max length 8 chars
 `CUSTOM_FLIPPER_NAME=Name ./fbt COMPACT=1 DEBUG=0 updater_package` - where `Name` write your custom name
6. Copy `dist/f7-C/f7-update-local` folder to microSD `update/myfw/` and run `update` file on flipper from file manager app (Archive)
7. Flash from microSD card only!!!! .dfu update from qFlipper will not work properly since name and serial number will be changed
8. Done, you will have custom name, serial number and bluetooth mac address
9. Also you can skip 5-7 and flash with `CUSTOM_FLIPPER_NAME=Name ./fbt COMPACT=1 DEBUG=0 FORCE=1 flash_usb_full`


## Troubleshooting
### I'm using Windows and name changing / building firmware doesn't work
- Use PowerShell or VSCode terminal(powershell by default)
- Clear build files - `.\fbt.cmd COMPACT=1 DEBUG=0 updater_package -c`
- Enter this in same terminal `$Env:CUSTOM_FLIPPER_NAME="Name"`
- Run release build - `.\fbt.cmd COMPACT=1 DEBUG=0 updater_package`
- Flash as described before (see 6.)
- If something still not work - Run powershell or VSCode as Admin
### Name stays same for every new build
- Clear build files - `./fbt COMPACT=1 DEBUG=0 updater_package -c`
- Try again
### I want to return my original name and serial number
- Flash stock FW or any CFW using microSD card offline update method

Or
- Clear build files - `./fbt COMPACT=1 DEBUG=0 updater_package -c`
- Run release build - `./fbt COMPACT=1 DEBUG=0 updater_package`
- Copy `dist/f7-C/f7-update-local` folder to microSD `update/myfw/` and run `update` file on flipper from file manager app (Archive)
- Flash from microSD card only, .dfu update from qFlipper will not work properly since name and serial number will be changed
