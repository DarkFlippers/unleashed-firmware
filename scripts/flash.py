#!/usr/bin/env python3


from flipper.app import App
from flipper.assets.coprobin import CoproBinary
from flipper.cube import CubeProgrammer

STATEMENT = "AGREE_TO_LOSE_FLIPPER_FEATURES_THAT_USE_CRYPTO_ENCLAVE"


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        # Wipe
        self.parser_wipe = self.subparsers.add_parser("wipe", help="Wipe MCU Flash")
        self._addArgsSWD(self.parser_wipe)
        self.parser_wipe.set_defaults(func=self.wipe)
        # Core 1 boot
        self.parser_core1bootloader = self.subparsers.add_parser(
            "core1bootloader", help="Flash Core1 Bootloader"
        )
        self._addArgsSWD(self.parser_core1bootloader)
        self.parser_core1bootloader.add_argument(
            "bootloader", type=str, help="Bootloader binary"
        )
        self.parser_core1bootloader.set_defaults(func=self.core1bootloader)
        # Core 1 firmware
        self.parser_core1firmware = self.subparsers.add_parser(
            "core1firmware", help="Flash Core1 Firmware"
        )
        self._addArgsSWD(self.parser_core1firmware)
        self.parser_core1firmware.add_argument(
            "firmware", type=str, help="Firmware binary"
        )
        self.parser_core1firmware.set_defaults(func=self.core1firmware)
        # Core 1 all
        self.parser_core1 = self.subparsers.add_parser(
            "core1", help="Flash Core1 Bootloader and Firmware"
        )
        self._addArgsSWD(self.parser_core1)
        self.parser_core1.add_argument("bootloader", type=str, help="Bootloader binary")
        self.parser_core1.add_argument("firmware", type=str, help="Firmware binary")
        self.parser_core1.set_defaults(func=self.core1)
        # Core 2 fus
        self.parser_core2fus = self.subparsers.add_parser(
            "core2fus", help="Flash Core2 Firmware Update Service"
        )
        self._addArgsSWD(self.parser_core2fus)
        self.parser_core2fus.add_argument(
            "--statement",
            type=str,
            help="NEVER FLASH FUS, IT WILL ERASE CRYPTO ENCLAVE",
            required=True,
        )
        self.parser_core2fus.add_argument(
            "fus_address", type=str, help="Firmware Update Service Address"
        )
        self.parser_core2fus.add_argument(
            "fus", type=str, help="Firmware Update Service Binary"
        )
        self.parser_core2fus.set_defaults(func=self.core2fus)
        # Core 2 radio stack
        self.parser_core2radio = self.subparsers.add_parser(
            "core2radio", help="Flash Core2 Radio stack"
        )
        self._addArgsSWD(self.parser_core2radio)
        self.parser_core2radio.add_argument(
            "radio", type=str, help="Radio Stack Binary"
        )
        self.parser_core2radio.add_argument(
            "--addr",
            dest="radio_address",
            help="Radio Stack Binary Address, as per release_notes",
            type=lambda x: int(x, 16),
            default=0,
            required=False,
        )
        self.parser_core2radio.set_defaults(func=self.core2radio)

    def _addArgsSWD(self, parser):
        parser.add_argument(
            "--port", type=str, help="Port to connect: swd or usb1", default="swd"
        )
        parser.add_argument("--serial", type=str, help="ST-Link Serial Number")

    def _getCubeParams(self):
        return {
            "port": self.args.port,
            "serial": self.args.serial,
        }

    def wipe(self):
        self.logger.info("Wiping flash")
        cp = CubeProgrammer(self._getCubeParams())
        self.logger.info("Setting RDP to 0xBB")
        cp.setOptionBytes({"RDP": ("0xBB", "rw")})
        self.logger.info("Verifying RDP")
        r = cp.checkOptionBytes({"RDP": ("0xBB", "rw")})
        assert r is True
        self.logger.info(f"Result: {r}")
        self.logger.info("Setting RDP to 0xAA")
        cp.setOptionBytes({"RDP": ("0xAA", "rw")})
        self.logger.info("Verifying RDP")
        r = cp.checkOptionBytes({"RDP": ("0xAA", "rw")})
        assert r is True
        self.logger.info(f"Result: {r}")
        self.logger.info("Complete")
        return 0

    def core1bootloader(self):
        self.logger.info("Flashing bootloader")
        cp = CubeProgrammer(self._getCubeParams())
        cp.flashBin("0x08000000", self.args.bootloader)
        self.logger.info("Complete")
        cp.resetTarget()
        return 0

    def core1firmware(self):
        self.logger.info("Flashing firmware")
        cp = CubeProgrammer(self._getCubeParams())
        cp.flashBin("0x08008000", self.args.firmware)
        self.logger.info("Complete")
        cp.resetTarget()
        return 0

    def core1(self):
        self.logger.info("Flashing bootloader")
        cp = CubeProgrammer(self._getCubeParams())
        cp.flashBin("0x08000000", self.args.bootloader)
        self.logger.info("Flashing firmware")
        cp.flashBin("0x08008000", self.args.firmware)
        cp.resetTarget()
        self.logger.info("Complete")
        return 0

    def core2fus(self):
        if self.args.statement != STATEMENT:
            self.logger.error(
                "PLEASE DON'T. THIS FEATURE INTENDED ONLY FOR FACTORY FLASHING"
            )
            return 1
        self.logger.info("Flashing Firmware Update Service")
        cp = CubeProgrammer(self._getCubeParams())
        cp.flashCore2(self.args.fus_address, self.args.fus)
        self.logger.info("Complete")
        return 0

    def core2radio(self):
        stack_info = CoproBinary(self.args.radio)
        if not stack_info.is_stack():
            self.logger.error("Not a Radio Stack")
            return 1
        self.logger.info(f"Will flash {stack_info.img_sig.get_version()}")

        radio_address = self.args.radio_address
        if not radio_address:
            radio_address = stack_info.get_flash_load_addr()
            self.logger.warning(
                f"Radio address not provided, guessed as 0x{radio_address:X}"
            )
        if radio_address > 0x080E0000:
            self.logger.error("I KNOW WHAT YOU DID LAST SUMMER")
            return 1

        cp = CubeProgrammer(self._getCubeParams())
        self.logger.info("Removing Current Radio Stack")
        cp.deleteCore2RadioStack()
        self.logger.info("Flashing Radio Stack")
        cp.flashCore2(radio_address, self.args.radio)
        self.logger.info("Complete")
        return 0


if __name__ == "__main__":
    Main()()
