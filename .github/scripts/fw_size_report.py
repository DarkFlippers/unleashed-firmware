#!/usr/bin/env python3
"""Report f7 firmware flash/RAM usage and DFU size from a built firmware.elf.

Parses `arm-none-eabi-size -A <firmware.elf>`. On the STM32WB55 the firmware
grows up from the start of flash and the linker fills the gap up to the radio
stack / internal-storage boundary with a `.free_flash` section
(targets/f7/stm32wb55xx_flash.ld) — so its size *is* "how much flash is left".

  flash used = .text + .rodata + .data   (flash-resident)
  flash free = .free_flash
  RAM used   = .data + .bss

Emits Markdown (--out) and prints JSON to stdout:
  {"free_bytes":int|null,"used_bytes":int|null,"total_bytes":int|null,"dfu_bytes":int|null}

Sources (any one):
  * CI:   --size-bin <arm-none-eabi-size> --elf <firmware.elf>
  * Test: --size-output <captured `size -A` text file>
DFU size (optional): --dfu <firmware.dfu>
"""

import argparse
import json
import os
import subprocess
from pathlib import Path

FLASH_SECTIONS = (".text", ".rodata", ".data")  # flash-resident
RAM_SECTIONS = (".data", ".bss")


def parse_size(text):
    """Return {section: bytes} from `arm-none-eabi-size -A` (sysv) output."""
    sizes = {}
    for line in text.splitlines():
        parts = line.split()
        if len(parts) != 3:  # "section size addr"; header/total rows differ
            continue
        section, size, _addr = parts
        try:
            sizes[section] = int(size)
        except ValueError:
            continue  # e.g. the "section size addr" header row
    return sizes


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
    free = sizes.get(".free_flash")
    used = sum(sizes[s] for s in FLASH_SECTIONS if s in sizes) or None
    ram = sum(sizes[s] for s in RAM_SECTIONS if s in sizes) or None
    total = used + free if (used is not None and free is not None) else None
    dfu = os.path.getsize(args.dfu) if (args.dfu and Path(args.dfu).exists()) else None

    lines = ["### 💾 Flash & RAM (f7)", ""]
    if free is None and used is None:
        lines.append("_Size data unavailable (firmware ELF not found)._")
    else:
        lines += ["| Metric | Size |", "|---|---|"]
        if used is not None:
            lines.append(f"| Firmware (flash-resident) | {human(used)} |")
        if free is not None:
            pct = f" — {free / total * 100:.1f}% of region free" if total else ""
            lines.append(f"| **Free flash** | **{human(free)}**{pct} |")
        if total is not None:
            lines.append(f"| Flash region (used + free) | {human(total)} |")
        if dfu is not None:
            lines.append(f"| DFU image | {human(dfu)} |")
        if ram is not None:
            lines.append(f"| RAM (.data + .bss) | {human(ram)} |")
    Path(args.out).write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(json.dumps({
        "free_bytes": free, "used_bytes": used,
        "total_bytes": total, "dfu_bytes": dfu,
    }))


if __name__ == "__main__":
    main()
