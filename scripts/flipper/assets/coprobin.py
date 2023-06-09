import struct
import math
import os
import os.path
import sys


#  From lib/stm32wb_copro/wpan/interface/patterns/ble_thread/shci/shci.h
__STACK_TYPE_CODES = {
    "BLE_FULL": 0x01,
    "BLE_HCI": 0x02,
    "BLE_LIGHT": 0x03,
    "BLE_BEACON": 0x04,
    "BLE_BASIC": 0x05,
    "BLE_FULL_EXT_ADV": 0x06,
    "BLE_HCI_EXT_ADV": 0x07,
    "THREAD_FTD": 0x10,
    "THREAD_MTD": 0x11,
    "ZIGBEE_FFD": 0x30,
    "ZIGBEE_RFD": 0x31,
    "MAC": 0x40,
    "BLE_THREAD_FTD_STATIC": 0x50,
    "BLE_THREAD_FTD_DYAMIC": 0x51,
    "802154_LLD_TESTS": 0x60,
    "802154_PHY_VALID": 0x61,
    "BLE_PHY_VALID": 0x62,
    "BLE_LLD_TESTS": 0x63,
    "BLE_RLV": 0x64,
    "802154_RLV": 0x65,
    "BLE_ZIGBEE_FFD_STATIC": 0x70,
    "BLE_ZIGBEE_RFD_STATIC": 0x71,
    "BLE_ZIGBEE_FFD_DYNAMIC": 0x78,
    "BLE_ZIGBEE_RFD_DYNAMIC": 0x79,
    "RLV": 0x80,
    "BLE_MAC_STATIC": 0x90,
}


class CoproException(ValueError):
    pass


#  Formats based on AN5185
class CoproFooterBase:
    SIG_BIN_SIZE = 5 * 4
    _SIG_BIN_COMMON_SIZE = 2 * 4

    def get_version(self):
        return (
            f"Version {self.version_major}.{self.version_minor}.{self.version_sub}, "
            f"branch {self.version_branch}, build {self.version_build} (magic {self.magic:X})"
        )

    def get_details(self):
        raise CoproException("Not implemented")

    def __init__(self, raw: bytes):
        if len(raw) != self.SIG_BIN_SIZE:
            raise CoproException("Invalid footer size")
        sig_common_part = raw[-self._SIG_BIN_COMMON_SIZE :]
        parts = struct.unpack("BBBBI", sig_common_part)
        self.version_major = parts[3]
        self.version_minor = parts[2]
        self.version_sub = parts[1]
        #  AN5185 mismatch: swapping byte halves
        self.version_build = parts[0] & 0x0F
        self.version_branch = (parts[0] & 0xF0) >> 4
        self.magic = parts[4]


class CoproFusFooter(CoproFooterBase):
    FUS_MAGIC_IMG_STACK = 0x23372991
    FUS_MAGIC_IMG_FUS = 0x32279221
    FUS_MAGIC_IMG_OTHER = 0x42769811

    FUS_BASE = 0x80F4000
    FLASH_PAGE_SIZE = 4 * 1024

    def __init__(self, raw: bytes):
        super().__init__(raw)
        if self.magic not in (
            self.FUS_MAGIC_IMG_OTHER,
            self.FUS_MAGIC_IMG_FUS,
            self.FUS_MAGIC_IMG_STACK,
        ):
            raise CoproException(f"Invalid FUS img magic {self.magic:x}")
        own_data = raw[: -self._SIG_BIN_COMMON_SIZE]
        parts = struct.unpack("IIBBBB", own_data)
        self.info1 = parts[0]
        self.info2 = parts[1]
        self.sram2b_1ks = parts[5]
        self.sram2a_1ks = parts[4]
        self.flash_4ks = parts[2]

    def get_details(self):
        return f"SRAM2b={self.sram2b_1ks}k SRAM2a={self.sram2a_1ks}k flash={self.flash_4ks}p"

    def is_stack(self):
        return self.magic == self.FUS_MAGIC_IMG_STACK

    def get_flash_pages(self, fullsize):
        return math.ceil(fullsize / self.FLASH_PAGE_SIZE)

    def get_flash_base(self, fullsize):
        if not self.is_stack():
            raise CoproException("Not a stack image")
        return self.FUS_BASE - self.get_flash_pages(fullsize) * self.FLASH_PAGE_SIZE


class CoproSigFooter(CoproFooterBase):
    SIG_MAGIC_ST = 0xD3A12C5E
    SIG_MAGIC_CUSTOMER = 0xE2B51D4A

    def __init__(self, raw: bytes):
        super().__init__(raw)
        if self.magic not in (self.SIG_MAGIC_ST, self.SIG_MAGIC_CUSTOMER):
            raise CoproException(f"Invalid FUS img magic {self.magic:x}")
        own_data = raw[: -self._SIG_BIN_COMMON_SIZE]
        parts = struct.unpack("IIBBH", own_data)
        self.reserved_1 = parts[0]
        self.reserved_2 = parts[1]
        self.size = parts[2]
        self.source = parts[3]
        self.reserved_34 = parts[4]

    def get_details(self):
        return f"Signature Src {self.source:x} size {self.size:x}"


class CoproBinary:
    def __init__(self, binary_path):
        self.binary_path = binary_path
        self.img_sig_footer = None
        self.img_sig = None
        self.binary_size = -1
        self._load()

    def _load(self):
        with open(self.binary_path, "rb") as fin:
            whole_file = fin.read()
            self.binary_size = len(whole_file)

            img_sig_footer_bin = whole_file[-CoproFooterBase.SIG_BIN_SIZE :]
            self.img_sig_footer = CoproSigFooter(img_sig_footer_bin)
            img_sig_size = self.img_sig_footer.size + CoproSigFooter.SIG_BIN_SIZE
            img_sig_bin = whole_file[
                -(img_sig_size + CoproFusFooter.SIG_BIN_SIZE) : -img_sig_size
            ]
            self.img_sig = CoproFusFooter(img_sig_bin)

    def is_valid(self):
        return self.img_sig_footer is not None and self.img_sig is not None

    def is_stack(self):
        return self.img_sig and self.img_sig.is_stack()

    def get_flash_load_addr(self):
        if not self.is_stack():
            raise CoproException("Not a stack image")
        return self.img_sig.get_flash_base(self.binary_size)


def get_stack_type(typestr: str):
    stack_code = __STACK_TYPE_CODES.get(typestr.upper(), None)
    if stack_code is None:
        raise CoproException(f"Unknown stack type {typestr}. See shci.h")
    return stack_code


def _load_bin(binary_path: str):
    print(binary_path)
    copro_bin = CoproBinary(binary_path)
    print(copro_bin.img_sig.get_version())
    if copro_bin.img_sig.is_stack():
        print(f"\t>> FLASH AT {copro_bin.get_flash_load_addr():X}\n")


def main():
    coprodir = (
        sys.argv[1]
        if len(sys.argv) > 1
        else "../../../lib/STM32CubeWB/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x"
    )
    for fn in os.listdir(coprodir):
        if not fn.endswith(".bin"):
            continue
        _load_bin(os.path.join(coprodir, fn))


if __name__ == "__main__":
    main()
