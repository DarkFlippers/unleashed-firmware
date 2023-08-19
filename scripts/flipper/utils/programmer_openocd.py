import logging
import os
import typing
from enum import Enum

from flipper.utils.programmer import Programmer
from flipper.utils.openocd import OpenOCD
from flipper.utils.stm32wb55 import STM32WB55
from flipper.assets.obdata import OptionBytesData


class OpenOCDProgrammerResult(Enum):
    Success = 0
    ErrorGeneric = 1
    ErrorAlignment = 2
    ErrorAlreadyWritten = 3
    ErrorValidation = 4


class OpenOCDProgrammer(Programmer):
    def __init__(
        self,
        interface: str = "interface/cmsis-dap.cfg",
        port_base: typing.Union[int, None] = None,
        serial: typing.Union[str, None] = None,
    ):
        super().__init__()

        config = {}

        config["interface"] = interface
        config["target"] = "target/stm32wbx.cfg"

        if serial is not None:
            if interface == "interface/cmsis-dap.cfg":
                config["serial"] = f"cmsis_dap_serial {serial}"
            elif "stlink" in interface:
                config["serial"] = f"stlink_serial {serial}"

        if port_base is not None:
            config["port_base"] = port_base

        self.openocd = OpenOCD(config)
        self.logger = logging.getLogger()

    def reset(self, mode: Programmer.RunMode = Programmer.RunMode.Run) -> bool:
        stm32 = STM32WB55(self.openocd)
        if mode == Programmer.RunMode.Run:
            stm32.reset(stm32.RunMode.Run)
        elif mode == Programmer.RunMode.Stop:
            stm32.reset(stm32.RunMode.Init)
        else:
            raise Exception("Unknown mode")

        return True

    def flash(self, address: int, file_path: str, verify: bool = True) -> bool:
        if not os.path.exists(file_path):
            raise Exception(f"File {file_path} not found")

        self.openocd.start()
        self.openocd.send_tcl("init")
        self.openocd.send_tcl(
            f"program {file_path} 0x{address:08x}{' verify' if verify else ''} reset exit"
        )
        self.openocd.stop()

        return True

    def _ob_print_diff_table(self, ob_reference: bytes, ob_read: bytes, print_fn):
        print_fn(
            f'{"Reference": <20} {"Device": <20} {"Diff Reference": <20} {"Diff Device": <20}'
        )

        # Split into 8 byte, word + word
        for i in range(0, len(ob_reference), 8):
            ref = ob_reference[i : i + 8]
            read = ob_read[i : i + 8]

            diff_str1 = ""
            diff_str2 = ""
            for j in range(0, len(ref.hex()), 2):
                byte_str_1 = ref.hex()[j : j + 2]
                byte_str_2 = read.hex()[j : j + 2]

                if byte_str_1 == byte_str_2:
                    diff_str1 += "__"
                    diff_str2 += "__"
                else:
                    diff_str1 += byte_str_1
                    diff_str2 += byte_str_2

            print_fn(
                f"{ref.hex(): <20} {read.hex(): <20} {diff_str1: <20} {diff_str2: <20}"
            )

    def option_bytes_validate(self, file_path: str) -> bool:
        # Registers
        stm32 = STM32WB55(self.openocd)

        # OpenOCD
        self.openocd.start()
        stm32.reset(stm32.RunMode.Init)

        # Generate Option Bytes data
        ob_data = OptionBytesData(file_path)
        ob_values = ob_data.gen_values().export()
        ob_reference = ob_values.reference
        ob_compare_mask = ob_values.compare_mask
        ob_length = len(ob_reference)
        ob_words = int(ob_length / 4)

        # Read Option Bytes
        ob_read = bytes()
        for i in range(ob_words):
            addr = stm32.OPTION_BYTE_BASE + i * 4
            value = self.openocd.read_32(addr)
            ob_read += value.to_bytes(4, "little")

        # Compare Option Bytes with reference by mask
        ob_compare = bytes()
        for i in range(ob_length):
            ob_compare += bytes([ob_read[i] & ob_compare_mask[i]])

        # Compare Option Bytes
        return_code = False

        if ob_reference == ob_compare:
            self.logger.info("Option Bytes are valid")
            return_code = True
        else:
            self.logger.error("Option Bytes are invalid")
            self._ob_print_diff_table(ob_reference, ob_compare, self.logger.error)

        # Stop OpenOCD
        stm32.reset(stm32.RunMode.Run)
        self.openocd.stop()

        return return_code

    def _unpack_u32(self, data: bytes, offset: int):
        return int.from_bytes(data[offset : offset + 4], "little")

    def option_bytes_set(self, file_path: str) -> bool:
        # Registers
        stm32 = STM32WB55(self.openocd)

        # OpenOCD
        self.openocd.start()
        stm32.reset(stm32.RunMode.Init)

        # Generate Option Bytes data
        ob_data = OptionBytesData(file_path)
        ob_values = ob_data.gen_values().export()
        ob_reference_bytes = ob_values.reference
        ob_compare_mask_bytes = ob_values.compare_mask
        ob_write_mask_bytes = ob_values.write_mask
        ob_length = len(ob_reference_bytes)
        ob_dwords = int(ob_length / 8)

        # Clear flash errors
        stm32.clear_flash_errors()

        # Unlock Flash and Option Bytes
        stm32.flash_unlock()
        stm32.option_bytes_unlock()

        ob_need_to_apply = False

        for i in range(ob_dwords):
            device_addr = stm32.OPTION_BYTE_BASE + i * 8
            device_value = self.openocd.read_32(device_addr)
            ob_write_mask = self._unpack_u32(ob_write_mask_bytes, i * 8)
            ob_compare_mask = self._unpack_u32(ob_compare_mask_bytes, i * 8)
            ob_value_ref = self._unpack_u32(ob_reference_bytes, i * 8)
            ob_value_masked = device_value & ob_compare_mask

            need_patch = ((ob_value_masked ^ ob_value_ref) & ob_write_mask) != 0
            if need_patch:
                ob_need_to_apply = True

                self.logger.info(
                    f"Need to patch: {device_addr:08X}: {ob_value_masked:08X} != {ob_value_ref:08X}, REG[{i}]"
                )

                # Check if this option byte (dword) is mapped to a register
                device_reg_addr = stm32.option_bytes_id_to_address(i)

                # Construct new value for the OB register
                ob_value = device_value & (~ob_write_mask)
                ob_value |= ob_value_ref & ob_write_mask

                self.logger.info(f"Writing {ob_value:08X} to {device_reg_addr:08X}")
                self.openocd.write_32(device_reg_addr, ob_value)

        if ob_need_to_apply:
            stm32.option_bytes_apply()
        else:
            self.logger.info("Option Bytes are already correct")

        # Load Option Bytes
        # That will reset and also lock the Option Bytes and the Flash
        stm32.option_bytes_load()

        # Stop OpenOCD
        stm32.reset(stm32.RunMode.Run)
        self.openocd.stop()

        return True

    def otp_write(self, address: int, file_path: str) -> OpenOCDProgrammerResult:
        # Open file, check that it aligned to 8 bytes
        with open(file_path, "rb") as f:
            data = f.read()
            if len(data) % 8 != 0:
                self.logger.error(f"File {file_path} is not aligned to 8 bytes")
                return OpenOCDProgrammerResult.ErrorAlignment

        # Check that address is aligned to 8 bytes
        if address % 8 != 0:
            self.logger.error(f"Address {address} is not aligned to 8 bytes")
            return OpenOCDProgrammerResult.ErrorAlignment

        # Get size of data
        data_size = len(data)

        # Check that data size is aligned to 8 bytes
        if data_size % 8 != 0:
            self.logger.error(f"Data size {data_size} is not aligned to 8 bytes")
            return OpenOCDProgrammerResult.ErrorAlignment

        self.logger.debug(f"Writing {data_size} bytes to OTP at {address:08X}")
        self.logger.debug(f"Data: {data.hex().upper()}")

        # Start OpenOCD
        self.openocd.start()

        # Registers
        stm32 = STM32WB55(self.openocd)

        try:
            # Check that OTP is empty for the given address
            # Also check that data is already written
            already_written = True
            for i in range(0, data_size, 4):
                file_word = int.from_bytes(data[i : i + 4], "little")
                device_word = self.openocd.read_32(address + i)
                if device_word != 0xFFFFFFFF and device_word != file_word:
                    self.logger.error(
                        f"OTP memory at {address + i:08X} is not empty: {device_word:08X}"
                    )
                    return OpenOCDProgrammerResult.ErrorAlreadyWritten

                if device_word != file_word:
                    already_written = False

            if already_written:
                self.logger.info("OTP memory is already written with the given data")
                return OpenOCDProgrammerResult.Success

            self.reset(self.RunMode.Stop)
            stm32.clear_flash_errors()

            # Write OTP memory by 8 bytes
            for i in range(0, data_size, 8):
                word_1 = int.from_bytes(data[i : i + 4], "little")
                word_2 = int.from_bytes(data[i + 4 : i + 8], "little")
                self.logger.debug(
                    f"Writing {word_1:08X} {word_2:08X} to {address + i:08X}"
                )
                stm32.write_flash_64(address + i, word_1, word_2)

            # Validate OTP memory
            validation_result = True

            for i in range(0, data_size, 4):
                file_word = int.from_bytes(data[i : i + 4], "little")
                device_word = self.openocd.read_32(address + i)
                if file_word != device_word:
                    self.logger.error(
                        f"Validation failed: {file_word:08X} != {device_word:08X} at {address + i:08X}"
                    )
                    validation_result = False
        finally:
            # Stop OpenOCD
            stm32.reset(stm32.RunMode.Run)
            self.openocd.stop()

        return (
            OpenOCDProgrammerResult.Success
            if validation_result
            else OpenOCDProgrammerResult.ErrorValidation
        )

    def option_bytes_recover(self) -> bool:
        try:
            self.openocd.start()
            stm32 = STM32WB55(self.openocd)
            stm32.reset(stm32.RunMode.Halt)
            stm32.option_bytes_recover()
            return True
        finally:
            self.openocd.stop()
