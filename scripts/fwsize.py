#!/usr/bin/env python3

from flipper.app import App
import subprocess
import os
import math


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_elfsize = self.subparsers.add_parser("elf", help="Dump elf stats")
        self.parser_elfsize.add_argument("elfname", action="store")
        self.parser_elfsize.set_defaults(func=self.process_elf)

        self.parser_binsize = self.subparsers.add_parser("bin", help="Dump bin stats")
        self.parser_binsize.add_argument("binname", action="store")
        self.parser_binsize.set_defaults(func=self.process_bin)

    def process_elf(self):
        all_sizes = subprocess.check_output(
            ["arm-none-eabi-size", "-A", self.args.elfname], shell=False
        )
        all_sizes = all_sizes.splitlines()

        sections_to_keep = (".text", ".rodata", ".data", ".bss", ".free_flash")
        for line in all_sizes:
            line = line.decode("utf-8")
            parts = line.split()
            if len(parts) != 3:
                continue
            section, size, _ = parts
            if section not in sections_to_keep:
                continue
            print(f"{section:<11} {size:>8} ({(int(size)/1024):6.2f} K)")

        return 0

    def process_bin(self):
        PAGE_SIZE = 4096
        binsize = os.path.getsize(self.args.binname)
        pages = math.ceil(binsize / PAGE_SIZE)
        last_page_state = (binsize % PAGE_SIZE) * 100 / PAGE_SIZE
        print(
            f"{os.path.basename(self.args.binname):<11}: {pages:>4} flash pages (last page {last_page_state:.02f}% full)"
        )
        return 0


if __name__ == "__main__":
    Main()()
