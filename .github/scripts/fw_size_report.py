#!/usr/bin/env python3
"""Report f7 firmware flash/RAM usage, DFU size, and *usable* free flash.

Parses ``arm-none-eabi-size -A <firmware.elf>``. On the STM32WB55 the firmware
grows up from the start of flash and the linker fills the gap up to the top of
the 1 MB flash with a ``.free_flash`` section (targets/f7/stm32wb55xx_flash.ld).

IMPORTANT: ``.free_flash`` overcounts usable space. The linker script models
flash as the full ``0x08000000..0x08100000`` 1 MB, but the BLE coprocessor
("core2") radio stack + FUS physically occupy the TOP of flash and are NOT
usable by our firmware/storage. So the real free flash is::

    radio_addr = FUS_BASE - ceil(copro_bin_size / 4096) * 4096   # stack load addr
    reserved   = flash_top - radio_addr                          # radio stack + FUS/secure
    usable_free = .free_flash - reserved                         # == radio_addr - free_flash_start

This mirrors Momentum's report (flash_free = radio_addr - flash_base - fw_size)
and the repo's own scripts/flipper/assets/coprobin.py (CoproFusFooter.get_flash_base),
where the radio stack load address depends only on the .bin size. For the default
``ble_light`` stack the reserved region is 164.00 KiB, leaving ~21.79 KiB usable.

  flash used = flash_top - .free_flash size - radio reserved ... reported as
               firmware footprint = free_flash_start - FLASH_BASE  (all sections)
  RAM used   = .data + .bss

Emits Markdown (--out) and prints JSON to stdout.

Sources (any one):
  * CI:   --size-bin <arm-none-eabi-size> --elf <firmware.elf>
  * Test: --size-output <captured `size -A` text file>
Optional: --dfu <firmware.dfu>   --copro-bin <radio stack .bin>  --copro-label ble_light
"""

import argparse
import json
import math
import os
import subprocess
from pathlib import Path

RAM_SECTIONS = (".data", ".bss")

# STM32WB55 flash geometry (see targets/f7/stm32wb55xx_flash.ld + coprobin.py).
FLASH_BASE = 0x08000000
FLASH_TOP = 0x08100000  # ORIGIN(FLASH) + LENGTH(FLASH) = 1 MiB
FUS_BASE = 0x080F4000   # CoproFusFooter.FUS_BASE
FLASH_PAGE = 4096


def parse_size(text):
    """Return {section: (bytes, addr)} from `arm-none-eabi-size -A` (sysv) output."""
    sizes = {}
    for line in text.splitlines():
        parts = line.split()
        if len(parts) != 3:  # "section size addr"; header/total rows differ
            continue
        section, size, addr = parts
        try:
            sizes[section] = (int(size), int(addr))
        except ValueError:
            continue  # e.g. the "section size addr" header row
    return sizes


def radio_load_addr(copro_size):
    """Flash load address of the radio stack, from its .bin size alone.

    Same math as scripts/flipper/assets/coprobin.py CoproFusFooter.get_flash_base:
    the stack is page-aligned and butts up against FUS_BASE.
    """
    pages = math.ceil(copro_size / FLASH_PAGE)
    return FUS_BASE - pages * FLASH_PAGE


def human(n):
    if n is None:
        return "n/a"
    if abs(n) < 1024:
        return f"{n} B"
    if abs(n) < 1024 * 1024:
        return f"{n / 1024:.2f} KiB"
    return f"{n / (1024 * 1024):.2f} MiB"


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--size-bin", help="path to arm-none-eabi-size")
    ap.add_argument("--elf", help="firmware.elf")
    ap.add_argument("--size-output", help="test mode: captured `size -A` output")
    ap.add_argument("--dfu", help="firmware.dfu (optional, for file size)")
    ap.add_argument("--copro-bin", help="radio stack .bin (to reserve its flash region)")
    ap.add_argument("--copro-label", default="", help="radio stack label, e.g. ble_light")
    ap.add_argument("--out", default="size_report.md")
    args = ap.parse_args()

    if args.size_output:
        size_text = Path(args.size_output).read_text(encoding="utf-8", errors="replace")
    elif args.size_bin and args.elf and Path(args.elf).exists():
        try:
            size_text = subprocess.check_output(
                [args.size_bin, "-A", args.elf]
            ).decode("utf-8", "replace")
        except (OSError, subprocess.CalledProcessError):
            size_text = ""  # bad/non-executable size binary -> degrade gracefully
    else:
        size_text = ""

    sizes = parse_size(size_text)

    free_flash, free_flash_start = sizes.get(".free_flash", (None, None))
    # Exact firmware flash footprint = everything below .free_flash (all sections,
    # alignment, init arrays, the .data LMA copy). Falls back to None if no ELF.
    firmware = (free_flash_start - FLASH_BASE) if free_flash_start is not None else None
    ram = sum(sizes[s][0] for s in RAM_SECTIONS if s in sizes) or None
    dfu = os.path.getsize(args.dfu) if (args.dfu and Path(args.dfu).exists()) else None

    # Reserve the radio stack + FUS region that the linker counts as ".free_flash".
    radio_addr = reserved = usable_free = None
    copro_size = None
    if args.copro_bin and Path(args.copro_bin).exists():
        copro_size = os.path.getsize(args.copro_bin)
        radio_addr = radio_load_addr(copro_size)
        reserved = FLASH_TOP - radio_addr
        if free_flash is not None:
            usable_free = free_flash - reserved

    label = f" ({args.copro_label})" if args.copro_label else ""
    lines = [f"### 💾 Flash & RAM — f7{label}", ""]
    if free_flash is None and firmware is None:
        lines.append("_Size data unavailable (firmware ELF not found)._")
    else:
        lines += ["| Metric | Size |", "|---|---|"]
        if firmware is not None:
            lines.append(f"| Firmware (flash) | {human(firmware)} |")
        if dfu is not None:
            lines.append(f"| DFU image | {human(dfu)} |")
        if reserved is not None:
            lines.append(f"| Reserved — radio stack + FUS | {human(reserved)} |")
        if usable_free is not None:
            warn = " ⚠️" if usable_free < 16 * 1024 else ""
            lines.append(f"| **Free flash (usable)** | **{human(usable_free)}{warn}** |")
        elif free_flash is not None:
            # No copro bin -> can only show the raw linker section (overcounts).
            lines.append(f"| Free flash (gross, excl. radio stack) | {human(free_flash)} |")
        if ram is not None:
            lines.append(f"| RAM (.data + .bss) | {human(ram)} |")
        lines.append("")
        if usable_free is not None:
            lines.append(
                f"<sub>1 MiB flash = firmware + usable free + radio/FUS. The BLE "
                f"coprocessor stack sits at the top of flash (load addr "
                f"`0x{radio_addr:08X}`); the linker's `.free_flash` "
                f"({human(free_flash)}) counts that region as free, so usable free = "
                f"`.free_flash` − reserved ({human(reserved)}).</sub>"
            )
        elif free_flash is not None:
            lines.append(
                "<sub>⚠️ Radio stack size unknown (copro .bin not found), so this is the "
                "raw linker `.free_flash` and **overcounts** — the BLE coprocessor stack "
                "(~164 KiB for ble_light) at the top of flash is not subtracted.</sub>"
            )

    Path(args.out).write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(json.dumps({
        "usable_free_bytes": usable_free,
        "free_flash_section_bytes": free_flash,
        "radio_reserved_bytes": reserved,
        "radio_addr": (f"0x{radio_addr:08X}" if radio_addr is not None else None),
        "copro_bin_bytes": copro_size,
        "copro_label": args.copro_label or None,
        "firmware_flash_bytes": firmware,
        "dfu_bytes": dfu,
        "ram_bytes": ram,
    }))


if __name__ == "__main__":
    main()
