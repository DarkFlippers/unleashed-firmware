# How to Build by yourself:

## Install required software

- Git - [Download](https://git-scm.com/downloads) for Windows, on Linux/Mac install via package manager (`brew`, `apt`, ...)

For development:
- Git
- VSCode

## Clone the Repository

You should clone with 
```shell
$ git clone --recursive https://github.com/DarkFlippers/unleashed-firmware.git
```

## VSCode integration

`fbt` includes basic development environment configuration for VSCode. Run `./fbt vscode_dist` to deploy it. That will copy the initial environment configuration to the `.vscode` folder. After that, you can use that configuration by starting VSCode and choosing the firmware root folder in the `File > Open Folder` menu.

# Build on Linux/macOS

Check out `documentation/fbt.md` for details on building and flashing firmware. 

### Compile plugin and run it on connected flipper

```sh
./fbt COMPACT=1 DEBUG=0 launch_app APPSRC=applications_user/yourplugin
```

### Compile everything for development

```sh
./fbt updater_package
```

### Compile everything for release + get updater package to update from microSD card

```sh
./fbt COMPACT=1 DEBUG=0 updater_package
```

Check `dist/` for build outputs.

Use `flipper-z-{target}-update-{suffix}.tgz` to flash your device.

# Build on Windows

Check out `documentation/fbt.md` for details on building and flashing firmware. 

### Compile everything for development

```powershell
./fbt.cmd updater_package
```

### Compile everything for release + get updater package to update from microSD card

```powershell
./fbt.cmd COMPACT=1 DEBUG=0 updater_package
```

**You may need to change `/` to `\` in front of fbt command (Only for Windows)!**

Check `dist/` for build outputs.

Use `flipper-z-{target}-update-{suffix}.tgz` to flash your device.

If compilation fails, make sure all submodules are all initialized. Either clone with `--recursive` or use `git submodule update --init --recursive`.
