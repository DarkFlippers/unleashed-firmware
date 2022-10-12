#!/usr/bin/env python3

import asyncio
import os

from flipper.app import App


class Main(App):
    def init(self):
        self.parser.add_argument("watch_list", nargs="+", help="Directories to watch")
        self.is_building = False

    def clearConsole(self):
        os.system("cls" if os.name in ("nt", "dos") else "clear")

    async def rebuild(self, line):
        self.clearConsole()
        self.logger.info(f"Triggered by: {line}")
        proc = await asyncio.create_subprocess_exec("./fbt")
        await proc.wait()
        await asyncio.sleep(1)
        self.is_building = False

    async def run(self):
        proc = await asyncio.create_subprocess_exec(
            "fswatch", *self.args.watch_list, stdout=asyncio.subprocess.PIPE
        )

        while True:
            data = await proc.stdout.readline()
            line = data.decode().strip()
            if not self.is_building:
                self.is_building = True
                asyncio.create_task(self.rebuild(line))

    def call(self):
        try:
            asyncio.run(self.run())
        except KeyboardInterrupt:
            pass
        return 0


if __name__ == "__main__":
    Main()()
