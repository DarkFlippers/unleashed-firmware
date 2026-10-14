#!/usr/bin/env python3

import math
import os
import subprocess
import sys
from pathlib import Path

from ansi.color import fg
from flipper.app import App

FBT_ROOT_DIR = Path(__file__).absolute().parent.parent


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_elfsize = self.subparsers.add_parser("elf", help="Dump elf stats")
        self.parser_elfsize.add_argument("elfname", action="store")
        self.parser_elfsize.set_defaults(func=self.process_elf)

        self.parser_binsize = self.subparsers.add_parser("bin", help="Dump bin stats")
        self.parser_binsize.add_argument("binname", action="store")
        self.parser_binsize.set_defaults(func=self.process_bin)

        self.parser_free = self.subparsers.add_parser(
            "free", help="Dump free flash space, reserving core2 radio stack region"
        )
        self.parser_free.add_argument("elfname", action="store")
        self.parser_free.add_argument(
            "--label", default="", help="Build flavor name, e.g. debug or release"
        )
        self.parser_free.add_argument(
            "--copro-bin",
            default=None,
            help="Core2 radio stack .bin (default: COPRO_STACK_BIN from fbt_options)",
        )
        self.parser_free.set_defaults(func=self.process_free)

    def _get_elf_sections(self, elfname):
        """Return {section: (size, address)} from `arm-none-eabi-size -A` output."""
        sections = {}
        all_sizes = subprocess.check_output(
            ["arm-none-eabi-size", "-A", elfname], shell=False
        )
        for line in all_sizes.splitlines():
            parts = line.decode("utf-8").split()
            if len(parts) != 3:
                continue
            section, size, addr = parts
            try:
                sections[section] = (int(size), int(addr))
            except ValueError:
                continue  # "section size addr" header row
        return sections

    def process_elf(self):
        sections_to_keep = (".text", ".rodata", ".data", ".bss", ".free_flash")
        for section, (size, _) in self._get_elf_sections(self.args.elfname).items():
            if section not in sections_to_keep:
                continue
            print(f"{section:<11} {size:>8} ({(size/1024):6.2f} K)")

        return 0

    def process_bin(self):
        PAGE_SIZE = 4096
        binsize = os.path.getsize(self.args.binname)
        pages = math.ceil(binsize / PAGE_SIZE)
        last_page_state = (binsize % PAGE_SIZE) * 100 / PAGE_SIZE
        print(
            fg.yellow(
                f"{os.path.basename(self.args.binname):<11}: {pages:>4} flash pages (last page {last_page_state:.02f}% full)"
            )
        )
        return 0

    def _get_default_copro_bin(self):
        sys.path.insert(0, str(FBT_ROOT_DIR))
        import fbt_options

        return (
            FBT_ROOT_DIR / fbt_options.COPRO_STACK_BIN_DIR / fbt_options.COPRO_STACK_BIN
        )

    def process_free(self):
        from flipper.assets.coprobin import CoproBinary

        sections = self._get_elf_sections(self.args.elfname)
        free_flash, free_flash_start = sections[".free_flash"]
        # .free_flash is a DSECT spanning from the end of the firmware up to
        # ORIGIN(FLASH) + LENGTH(FLASH), see targets/f7/stm32wb55xx_flash.ld
        flash_base = sections[".isr_vector"][1]
        flash_top = free_flash_start + free_flash

        # Core2 radio stack & FUS live at the top of flash: the linker counts that
        # region as free, but the app processor cannot use it
        copro_bin = self.args.copro_bin or self._get_default_copro_bin()
        radio_addr = CoproBinary(str(copro_bin)).get_flash_load_addr()
        reserved = flash_top - radio_addr
        free = radio_addr - free_flash_start
        used = free_flash_start - flash_base

        label = f" ({self.args.label})" if self.args.label else ""
        colorize = fg.red if free <= 0 else fg.yellow
        print(
            colorize(
                f"free flash{label}: {free/1024:.2f} K "
                f"(firmware {used/1024:.2f} K, "
                f"core2 {os.path.basename(str(copro_bin))} reserves {reserved/1024:.2f} K "
                f"at 0x{radio_addr:08X})"
            )
        )

        warning = " :warning: **flash overflow!**" if free <= 0 else ""
        report = (
            f"💾 Free flash{label}: **{free/1024:.2f} K**{warning} "
            f"(firmware {used/1024:.2f} K, core2 reserves {reserved/1024:.2f} K)"
        )
        if github_output := os.environ.get("GITHUB_OUTPUT"):
            with open(github_output, "a") as file:
                file.write(f"free_flash={free}\n")
                file.write(f"size_report={report}\n")
        # Fork PRs get a read-only token, so the comment steps are skipped there:
        # the run summary is the only place contributors can see this
        if step_summary := os.environ.get("GITHUB_STEP_SUMMARY"):
            with open(step_summary, "a") as file:
                file.write(f"- {report}\n")

        return 0


if __name__ == "__main__":
    Main()()
