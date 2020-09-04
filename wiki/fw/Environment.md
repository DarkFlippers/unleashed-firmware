# Building/debugging/emulating

## Preparing for build

1. Install [docker compose](https://docs.docker.com/compose/install/)
2. After startup you should run `docker-compose up -d` to run the container.
3. Then you can run `docker-compose exec dev make -C <target_dir>` to build application.

If Dockerfile is changed you should run `docker-compose down` and `docker-compose build` for rebuild the image.

## Local build

For simple case as unit tests or integration test that no require hardware we mock HW and replace RTOS syscalls to POSIX syscalls. We get simple linux app that easy to debug and get flexible control on hardware stubs.

You can run firmware locally (with HAL stub).

* `docker-compose exec dev make -C target_lo` for build
* `docker-compose exec dev target_lo/build/target_lo` for run
* `docker-compose exec dev make -C target_lo test` for running tests

For UI we do "UI emulator" (not implemented)

1. Web page with display and other UI elements, controls
2. Local (linux) version of firmware. All calls, writing data to UI like display or LED, redirects to unix socket writes, and messages from unix socket redirect to firmware (emulates button press, change batt level, insert/remove USB, etc.)
3. Webserver that run linux version fw, pass events from webpage to unixsocket and vice versa.

## Firmware emulation (not implemented)

For more HW- and RTOS- specific checks we run real FW in [Renode](https://interrupt.memfault.com/blog/intro-to-renode)

## Running on remote Debug/test bench

Eventually we run real FW on remote debug/test bench (#26): flipper board + RPi + some stuff to control and check real hardware.

# Debug/test bench (not implemented)

* 24×7 connected target Flipper device and accessible via Internet. Raspberry PI or some Linux single-board PC can be used as basic high-level control board.
* Tool can push/click each user buttons by hardware by "control board" (low level). Usage of optocouples/reed-switch relays is fine for that.
* Connect other Flipper peripherals to target:
	* 433 door bell/barrier controller, to read it status and it's remote control (to sniff it signal by flipper).
	* Some iButtons and it's reader can be also connected to target.
	* RFID reader under the target, to paste readed keys from it to out UART.
	* RFID cards with different IDs. Can changed by servo or carousel from CD-changer
	* IR-transmitter/receiver.
	* ...all the peripherals, that we'll can realize...
* "Hardware" USB peripherals (SWD programmer tool, etc?) reconnection and control of PWR, RST, BOOTx internal service lines, etc also need. This can be made by some software hacks or by relays.
* Image from target display will be translated by webcam to web page.
* WEB page, accessed to all developers (or maybe for everyone to view is better?) with these things:
	* Target status area, Flipper control buttons, "connected" to real target. Power and service line switches.
	* Ability to upload custom firmware binary to target. Take firmware build from CI/CD. Button to flash.
	* Test tool UART with Flipper peripherals status and target device UART log also should be here.
* OpenOCD connection from target (can be accessed from developers around the world by VPN or just port forwarding with IP access lists). So this feature allows deep remote debug.
* List can be expanded with other good ideas...

## Usage

1. Developers connect to target directly by shedule.
2. Run CI tests:
	* For test automation we can use RobotDemo or simple expect tool/python scripts/etc.
	* Apply test cases and submit its results.
