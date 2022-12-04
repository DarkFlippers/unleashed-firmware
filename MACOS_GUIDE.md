# **macOS compile guide**  by deafbed
### Tested with M1 Macbook Air, macOS Monterey (12.4)

* *(NB: do NOT include the "$ " if copying/pasting any of the below commands)*

## FROM SCRATCH

### Install qFlipper
qFlipper can be found on the App Store or you can download it from [https://flipperzero.one/update](https://flipperzero.one/update)

### Install Brew
Open Terminal.app and run

`$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`

**IMPORTANT:** once installed, run the two commands given to you ("Next steps")

*(first should begin `$ echo 'eval...'`, second should begin `$ eval "$(/opt/...))`*

Quit and re-open Terminal.app, then run

`$ brew help`

to check if the installation was successful

### Install Docker Desktop
Download the appropriate version (Intel/M1) of Docker Desktop from
[https://docs.docker.com/desktop/mac/install/](https://docs.docker.com/desktop/mac/install/)

*(If the downloaded file has no extension/won't open, simply append .dmg to the filename)*

Open Docker.dmg and drag Docker.app to the Applications folder

Eject Docker.dmg from the sidebar, navigate to your Applications folder and open Docker. Install as prompted and wait a few seconds until Docker is running

Quit and re-open Terminal, and run

`$ docker help`

to check if the install was successful


### Make sure PIP is up-to-date
Still in Terminal, run

`$ python3 -m pip install --upgrade pip`

### Download the firmware
Next, run

`$ git clone --recursive https://github.com/RogueMaster/flipperzero-firmware-wPlugins.git`

This will download the latest build files from the github repository (in this case most likely to your Home folder)

### Install the necessary prerequisites
Navigate to the folder you just downloaded

`$ cd flipperzero-firmware-wPlugins`

Next, we run 2 scripts to download the remaining requirements. First:

`$ brew bundle --verbose`

Then:

`$ pip3 install -r scripts/requirements.txt`

### Compile
Make sure Docker is still open, if not open Docker.app

In Terminal, still in the flipperzero-firmware-wPlugins directory, run

`$ docker-compose up -d`

Then:

`$ docker-compose exec dev ./fbt`

It may take a while and you might see some errors but if all goes well the compiled firmware (DFU) file should now be in **flipperzero-firmware-wPlugins/dist/f7-C/flipper-z-f7-full-local.dfu**

### Put your Flipper into Recovery/DFU mode
Put your flipper into DFU mode by:

1. holding BACK + LEFT until it turns off
2. once turned off, release BACK but keep holding LEFT until the LED flashes blue
3. once the LED flashes blue, release LEFT and your Flipper should be in recovery mode

### Install the firmware
Open qFlipper.app

Connect your Flipper via USB

In qFlipper, click "Install from file" (underneath the big INSTALL button)

Navigate to your DFU file (**flipperzero-firmware-wPlugins/dist/f7-C/flipper-z-f7-full-local.dfu**)

Wait for the process to finish. If your Flipper device is no longer in DFU mode (i.e. shows the normal Desktop), unplug your Flipper and close the qFlipper app

### Post-Install
Open qFlipper.app and connect your Flipper via USB

If everything worked, your Flipper should be recognized and the firmware version should match the github commit shown here:
[https://github.com/RogueMaster/flipperzero-firmware-wPlugins](https://github.com/RogueMaster/flipperzero-firmware-wPlugins)
