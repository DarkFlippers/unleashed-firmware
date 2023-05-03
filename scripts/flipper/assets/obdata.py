#!/usr/bin/env python3


from enum import Enum
from dataclasses import dataclass
from typing import Tuple
from array import array


class OBException(ValueError):
    pass


@dataclass
class OBParams:
    word_idx: int
    bits: Tuple[int, int]
    name: str


_OBS_descr = (
    OBParams(0, (0, 8), "RDP"),
    OBParams(0, (8, 9), "ESE"),
    OBParams(0, (9, 12), "BOR_LEV"),
    OBParams(0, (12, 13), "nRST_STOP"),
    OBParams(0, (13, 14), "nRST_STDBY"),
    OBParams(0, (14, 15), "nRSTSHDW"),
    OBParams(0, (15, 16), "UNUSED1"),
    OBParams(0, (16, 17), "IWDGSW"),
    OBParams(0, (17, 18), "IWDGSTOP"),
    OBParams(0, (18, 19), "IWGDSTDBY"),  #  ST's typo: IWDGSTDBY
    OBParams(0, (18, 19), "IWDGSTDBY"),  #  ST's typo: IWDGSTDBY
    OBParams(0, (19, 20), "WWDGSW"),
    OBParams(0, (20, 23), "UNUSED2"),
    OBParams(0, (23, 24), "nBOOT1"),
    OBParams(0, (24, 25), "SRAM2PE"),
    OBParams(0, (25, 26), "SRAM2RST"),
    OBParams(0, (26, 27), "nSWBOOT0"),
    OBParams(0, (27, 28), "nBOOT0"),
    OBParams(0, (28, 29), "UNUSED3"),
    OBParams(0, (29, 32), "AGC_TRIM"),
    OBParams(1, (0, 9), "PCROP1A_STRT"),
    OBParams(1, (9, 32), "UNUSED"),
    OBParams(2, (0, 9), "PCROP1A_END"),
    OBParams(2, (9, 31), "UNUSED"),
    OBParams(2, (31, 32), "PCROP_RDP"),
    OBParams(3, (0, 8), "WRP1A_STRT"),
    OBParams(3, (8, 16), "UNUSED1"),
    OBParams(3, (16, 24), "WRP1A_END"),
    OBParams(3, (24, 32), "UNUSED2"),
    OBParams(4, (0, 8), "WRP1B_STRT"),
    OBParams(4, (8, 16), "UNUSED1"),
    OBParams(4, (16, 24), "WRP1B_END"),
    OBParams(4, (24, 32), "UNUSED2"),
    OBParams(5, (0, 9), "PCROP1B_STRT"),
    OBParams(5, (9, 32), "UNUSED"),
    OBParams(6, (0, 9), "PCROP1B_END"),
    OBParams(6, (9, 32), "UNUSED"),
    OBParams(13, (0, 14), "IPCCDBA"),
    OBParams(13, (14, 32), "UNUSED"),
    OBParams(14, (0, 8), "SFSA"),
    OBParams(14, (8, 9), "FSD"),
    OBParams(14, (9, 12), "UNUSED1"),
    OBParams(14, (12, 13), "DDS"),
    OBParams(14, (13, 32), "UNUSED2"),
    OBParams(15, (0, 18), "SBRV"),
    OBParams(15, (18, 23), "SBRSA"),
    OBParams(15, (23, 24), "BRSD"),
    OBParams(15, (24, 25), "UNUSED1"),
    OBParams(15, (25, 30), "SNBRSA"),
    OBParams(15, (30, 31), "NBRSD"),
    OBParams(15, (31, 32), "C2OPT"),
)


_OBS = dict((param.name, param) for param in _OBS_descr)


@dataclass
class EncodedOBValue:
    value: int
    mask: int
    params: OBParams


class OptionByte:
    class OBMode(Enum):
        IGNORE = 0
        READ = 1
        READ_WRITE = 2

        @classmethod
        def from_str(cls, value):
            if value == "r":
                return cls.READ
            elif value == "rw":
                return cls.READ_WRITE
            else:
                raise OBException(f"Unknown OB check mode '{value}'")

    def __init__(self, obstr):
        parts = obstr.split(":")
        if len(parts) != 3:
            raise OBException(f"Invalid OB value definition {obstr}")
        self.name = parts[0]
        self.value = int(parts[1], 16)
        self.mode = OptionByte.OBMode.from_str(parts[2].strip())
        self.descr = _OBS.get(self.name, None)
        if self.descr is None:
            raise OBException(f"Missing OB descriptor for {self.name}")

    def encode(self):
        startbit, endbit = self.descr.bits
        value_mask = 2 ** (endbit - startbit) - 1
        value_corrected = self.value & value_mask

        value_shifted = value_corrected << startbit
        value_mask_shifted = value_mask << startbit
        return EncodedOBValue(value_shifted, value_mask_shifted, self)

    def __repr__(self):
        return f"<OB {self.name}, 0x{self.value:x}, {self.mode} at 0x{id(self):X}>"


@dataclass
class ObReferenceValues:
    reference: bytes
    compare_mask: bytes
    write_mask: bytes


class ObReferenceValuesGenerator:
    def __init__(self):
        self.compare_mask = array("I", [0] * 16)
        self.write_mask = array("I", [0] * 16)
        self.ref_values = array("I", [0] * 16)

    def __repr__(self):
        return (
            f"<OBRefs REFS=[{' '.join(hex(v) for v in self.ref_values)}] "
            f"CMPMASK=[{' '.join(hex(v) for v in self.compare_mask)}] "
            f"WRMASK=[{' '.join(hex(v) for v in self.write_mask)}] "
        )

    def export_values(self):
        export_cmpmask = array("I")
        for value in self.compare_mask:
            export_cmpmask.append(value)
            export_cmpmask.append(value)
        export_wrmask = array("I")
        for value in self.write_mask:
            export_wrmask.append(value)
            export_wrmask.append(value)
        export_refvals = array("I")
        for cmpmask, refval in zip(self.compare_mask, self.ref_values):
            export_refvals.append(refval)
            export_refvals.append((refval ^ 0xFFFFFFFF) & cmpmask)
        return export_refvals, export_cmpmask, export_wrmask

    def export(self):
        return ObReferenceValues(*map(lambda a: a.tobytes(), self.export_values()))

    def apply(self, ob):
        ob_params = ob.descr
        encoded_ob = ob.encode()
        self.compare_mask[ob_params.word_idx] |= encoded_ob.mask
        self.ref_values[ob_params.word_idx] |= encoded_ob.value
        if ob.mode == OptionByte.OBMode.READ_WRITE:
            self.write_mask[ob_params.word_idx] |= encoded_ob.mask


class OptionBytesData:
    def __init__(self, obfname):
        self.obs = list()
        with open(obfname, "rt") as obfin:
            self.obs = list(
                OptionByte(line) for line in obfin if not line.startswith("#")
            )

    def gen_values(self):
        obref = ObReferenceValuesGenerator()
        list(obref.apply(ob) for ob in self.obs)
        return obref


def main():
    with open("../../../../logs/obs.bin", "rb") as obsbin:
        ob_sample = obsbin.read(128)
        ob_sample_arr = array("I", ob_sample)
    print(ob_sample_arr)

    obd = OptionBytesData("../../ob.data")
    print(obd.obs)
    # print(obd.gen_values().export())
    ref, mask, wrmask = obd.gen_values().export_values()
    for idx in range(len(ob_sample_arr)):
        real_masked = ob_sample_arr[idx] & mask[idx]
        print(
            f"#{idx}: ref {ref[idx]:08x} real {real_masked:08x} ({ob_sample_arr[idx]:08x} & {mask[idx]:08x}) match {ref[idx]==real_masked}"
        )

    # print(ob_sample)


if __name__ == "__main__":
    main()
