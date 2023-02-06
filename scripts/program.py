#!/usr/bin/env python3
import typing
import subprocess
import logging
import time
import os
import socket

from abc import ABC, abstractmethod
from dataclasses import dataclass
from flipper.app import App


class Programmer(ABC):
    @abstractmethod
    def flash(self, bin: str) -> bool:
        pass

    @abstractmethod
    def probe(self) -> bool:
        pass

    @abstractmethod
    def get_name(self) -> str:
        pass

    @abstractmethod
    def set_serial(self, serial: str):
        pass


@dataclass
class OpenOCDInterface:
    name: str
    file: str
    serial_cmd: str
    additional_args: typing.Optional[list[str]] = None


class OpenOCDProgrammer(Programmer):
    def __init__(self, interface: OpenOCDInterface):
        self.interface = interface
        self.logger = logging.getLogger("OpenOCD")
        self.serial: typing.Optional[str] = None

    def _add_file(self, params: list[str], file: str):
        params.append("-f")
        params.append(file)

    def _add_command(self, params: list[str], command: str):
        params.append("-c")
        params.append(command)

    def _add_serial(self, params: list[str], serial: str):
        self._add_command(params, f"{self.interface.serial_cmd} {serial}")

    def set_serial(self, serial: str):
        self.serial = serial

    def flash(self, bin: str) -> bool:
        i = self.interface

        if os.altsep:
            bin = bin.replace(os.sep, os.altsep)

        openocd_launch_params = ["openocd"]
        self._add_file(openocd_launch_params, i.file)
        if self.serial:
            self._add_serial(openocd_launch_params, self.serial)
        if i.additional_args:
            for a in i.additional_args:
                self._add_command(openocd_launch_params, a)
        self._add_file(openocd_launch_params, "target/stm32wbx.cfg")
        self._add_command(openocd_launch_params, "init")
        self._add_command(openocd_launch_params, f"program {bin} reset exit 0x8000000")

        # join the list of parameters into a string, but add quote if there are spaces
        openocd_launch_params_string = " ".join(
            [f'"{p}"' if " " in p else p for p in openocd_launch_params]
        )

        self.logger.debug(f"Launching: {openocd_launch_params_string}")

        process = subprocess.Popen(
            openocd_launch_params,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )

        while process.poll() is None:
            time.sleep(0.25)
            print(".", end="", flush=True)
        print()

        success = process.returncode == 0

        if not success:
            self.logger.error("OpenOCD failed to flash")
            if process.stdout:
                self.logger.error(process.stdout.read().decode("utf-8").strip())

        return success

    def probe(self) -> bool:
        i = self.interface

        openocd_launch_params = ["openocd"]
        self._add_file(openocd_launch_params, i.file)
        if self.serial:
            self._add_serial(openocd_launch_params, self.serial)
        if i.additional_args:
            for a in i.additional_args:
                self._add_command(openocd_launch_params, a)
        self._add_file(openocd_launch_params, "target/stm32wbx.cfg")
        self._add_command(openocd_launch_params, "init")
        self._add_command(openocd_launch_params, "exit")

        self.logger.debug(f"Launching: {' '.join(openocd_launch_params)}")

        process = subprocess.Popen(
            openocd_launch_params,
            stderr=subprocess.STDOUT,
            stdout=subprocess.PIPE,
        )

        # Wait for OpenOCD to end and get the return code
        process.wait()
        found = process.returncode == 0

        if process.stdout:
            self.logger.debug(process.stdout.read().decode("utf-8").strip())

        return found

    def get_name(self) -> str:
        return self.interface.name


def blackmagic_find_serial(serial: str):
    import serial.tools.list_ports as list_ports

    if serial and os.name == "nt":
        if not serial.startswith("\\\\.\\"):
            serial = f"\\\\.\\{serial}"

    ports = list(list_ports.grep("blackmagic"))
    if len(ports) == 0:
        return None
    elif len(ports) > 2:
        if serial:
            ports = list(
                filter(
                    lambda p: p.serial_number == serial
                    or p.name == serial
                    or p.device == serial,
                    ports,
                )
            )
            if len(ports) == 0:
                return None

        if len(ports) > 2:
            raise Exception("More than one Blackmagic probe found")

    # If you're getting any issues with auto lookup, uncomment this
    # print("\n".join([f"{p.device} {vars(p)}" for p in ports]))
    port = sorted(ports, key=lambda p: f"{p.location}_{p.name}")[0]

    if serial:
        if (
            serial != port.serial_number
            and serial != port.name
            and serial != port.device
        ):
            return None

    if os.name == "nt":
        port.device = f"\\\\.\\{port.device}"
    return port.device


def _resolve_hostname(hostname):
    try:
        return socket.gethostbyname(hostname)
    except socket.gaierror:
        return None


def blackmagic_find_networked(serial: str):
    if not serial:
        serial = "blackmagic.local"

    # remove the tcp: prefix if it's there
    if serial.startswith("tcp:"):
        serial = serial[4:]

    # remove the port if it's there
    if ":" in serial:
        serial = serial.split(":")[0]

    if not (probe := _resolve_hostname(serial)):
        return None

    return f"tcp:{probe}:2345"


class BlackmagicProgrammer(Programmer):
    def __init__(
        self,
        port_resolver,  # typing.Callable[typing.Union[str, None], typing.Optional[str]]
        name: str,
    ):
        self.port_resolver = port_resolver
        self.name = name
        self.logger = logging.getLogger("BlackmagicUSB")
        self.port: typing.Optional[str] = None

    def _add_command(self, params: list[str], command: str):
        params.append("-ex")
        params.append(command)

    def _valid_ip(self, address):
        try:
            socket.inet_aton(address)
            return True
        except:
            return False

    def set_serial(self, serial: str):
        if self._valid_ip(serial):
            self.port = f"{serial}:2345"
        elif ip := _resolve_hostname(serial):
            self.port = f"{ip}:2345"
        else:
            self.port = serial

    def flash(self, bin: str) -> bool:
        if not self.port:
            if not self.probe():
                return False

        # We can convert .bin to .elf with objcopy:
        # arm-none-eabi-objcopy -I binary -O elf32-littlearm --change-section-address=.data=0x8000000 -B arm -S app.bin app.elf
        # But I choose to use the .elf file directly because we are flashing our own firmware and it always has an elf predecessor.
        elf = bin.replace(".bin", ".elf")
        if not os.path.exists(elf):
            self.logger.error(
                f"Sorry, but Blackmagic can't flash .bin file, and {elf} doesn't exist"
            )
            return False

        # arm-none-eabi-gdb build/f7-firmware-D/firmware.bin
        # -ex 'set pagination off'
        # -ex 'target extended-remote /dev/cu.usbmodem21201'
        # -ex 'set confirm off'
        # -ex 'monitor swdp_scan'
        # -ex 'attach 1'
        # -ex 'set mem inaccessible-by-default off'
        # -ex 'load'
        # -ex 'compare-sections'
        # -ex 'quit'

        gdb_launch_params = ["arm-none-eabi-gdb", elf]
        self._add_command(gdb_launch_params, f"target extended-remote {self.port}")
        self._add_command(gdb_launch_params, "set pagination off")
        self._add_command(gdb_launch_params, "set confirm off")
        self._add_command(gdb_launch_params, "monitor swdp_scan")
        self._add_command(gdb_launch_params, "attach 1")
        self._add_command(gdb_launch_params, "set mem inaccessible-by-default off")
        self._add_command(gdb_launch_params, "load")
        self._add_command(gdb_launch_params, "compare-sections")
        self._add_command(gdb_launch_params, "quit")

        self.logger.debug(f"Launching: {' '.join(gdb_launch_params)}")

        process = subprocess.Popen(
            gdb_launch_params,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )

        while process.poll() is None:
            time.sleep(0.5)
            print(".", end="", flush=True)
        print()

        if not process.stdout:
            return False

        output = process.stdout.read().decode("utf-8").strip()
        flashed = "Loading section .text," in output

        # Check flash verification
        if "MIS-MATCHED!" in output:
            flashed = False

        if "target image does not match the loaded file" in output:
            flashed = False

        if not flashed:
            self.logger.error("Blackmagic failed to flash")
            self.logger.error(output)

        return flashed

    def probe(self) -> bool:
        if not (port := self.port_resolver(self.port)):
            return False

        self.port = port
        return True

    def get_name(self) -> str:
        return self.name


programmers: list[Programmer] = [
    OpenOCDProgrammer(
        OpenOCDInterface(
            "cmsis-dap",
            "interface/cmsis-dap.cfg",
            "cmsis_dap_serial",
            ["transport select swd"],
        ),
    ),
    OpenOCDProgrammer(
        OpenOCDInterface(
            "stlink", "interface/stlink.cfg", "hla_serial", ["transport select hla_swd"]
        ),
    ),
    BlackmagicProgrammer(blackmagic_find_serial, "blackmagic_usb"),
]

network_programmers = [
    BlackmagicProgrammer(blackmagic_find_networked, "blackmagic_wifi")
]


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        self.parser_flash = self.subparsers.add_parser("flash", help="Flash a binary")
        self.parser_flash.add_argument(
            "bin",
            type=str,
            help="Binary to flash",
        )
        interfaces = [i.get_name() for i in programmers]
        interfaces.extend([i.get_name() for i in network_programmers])
        self.parser_flash.add_argument(
            "--interface",
            choices=interfaces,
            type=str,
            help="Interface to use",
        )
        self.parser_flash.add_argument(
            "--serial",
            type=str,
            help="Serial number or port of the programmer",
        )
        self.parser_flash.set_defaults(func=self.flash)

    def _search_interface(self, serial: typing.Optional[str]) -> list[Programmer]:
        found_programmers = []

        for p in programmers:
            name = p.get_name()
            if serial:
                p.set_serial(serial)
                self.logger.debug(f"Trying {name} with {serial}")
            else:
                self.logger.debug(f"Trying {name}")

            if p.probe():
                self.logger.debug(f"Found {name}")
                found_programmers += [p]
            else:
                self.logger.debug(f"Failed to probe {name}")

        return found_programmers

    def _search_network_interface(
        self, serial: typing.Optional[str]
    ) -> list[Programmer]:
        found_programmers = []

        for p in network_programmers:
            name = p.get_name()

            if serial:
                p.set_serial(serial)
                self.logger.debug(f"Trying {name} with {serial}")
            else:
                self.logger.debug(f"Trying {name}")

            if p.probe():
                self.logger.debug(f"Found {name}")
                found_programmers += [p]
            else:
                self.logger.debug(f"Failed to probe {name}")

        return found_programmers

    def flash(self):
        start_time = time.time()
        bin_path = os.path.abspath(self.args.bin)

        if not os.path.exists(bin_path):
            self.logger.error(f"Binary file not found: {bin_path}")
            return 1

        if self.args.interface:
            i_name = self.args.interface
            interfaces = [p for p in programmers if p.get_name() == i_name]
            if len(interfaces) == 0:
                interfaces = [p for p in network_programmers if p.get_name() == i_name]
        else:
            self.logger.info(f"Probing for interfaces...")
            interfaces = self._search_interface(self.args.serial)

            if len(interfaces) == 0:
                # Probe network blackmagic
                self.logger.info(f"Probing for network interfaces...")
                interfaces = self._search_network_interface(self.args.serial)

            if len(interfaces) == 0:
                self.logger.error("No interface found")
                return 1

            if len(interfaces) > 1:
                self.logger.error("Multiple interfaces found: ")
                self.logger.error(
                    f"Please specify '--interface={[i.get_name() for i in interfaces]}'"
                )
                return 1

        interface = interfaces[0]

        if self.args.serial:
            interface.set_serial(self.args.serial)
            self.logger.info(
                f"Flashing {bin_path} via {interface.get_name()} with {self.args.serial}"
            )
        else:
            self.logger.info(f"Flashing {bin_path} via {interface.get_name()}")

        if not interface.flash(bin_path):
            self.logger.error(f"Failed to flash via {interface.get_name()}")
            return 1

        flash_time = time.time() - start_time
        bin_size = os.path.getsize(bin_path)
        self.logger.info(f"Flashed successfully in {flash_time:.2f}s")
        self.logger.info(f"Effective speed: {bin_size / flash_time / 1024:.2f} KiB/s")
        return 0


if __name__ == "__main__":
    Main()()
