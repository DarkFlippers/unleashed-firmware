# Docker Build
Build and flash your flipper in docker. Simply use the makefile rules to build the docker image and flash your flipper device. The makefile automatically detects and mounts your fliper into the docker container.

## Quick build: 
1. Connect your flipper over USB
2. Run the command: `make flash`

## Help Menu
```text
make [all|build|dev|clean|help]
    img  : Create docker image
    flash: Build and flash flipper
    bash : Launch shell in unleashed_firmware contianer
    clean: Remove docker image
    help : Print this help menu
```
