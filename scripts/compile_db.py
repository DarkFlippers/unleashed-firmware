#!/usr/bin/env python3

from flipper.app import App
import json
import pathlib


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # generate
        self.parser_generate = self.subparsers.add_parser(
            "generate", help="Check source code format and file names"
        )
        self.parser_generate.add_argument("-p", dest="path", required=True)
        self.parser_generate.set_defaults(func=self.generate)

    def parse_sources(self, path, source_path, flags_path):
        flags = ""
        with open(path + "/" + flags_path) as f:
            for line in f:
                if line.strip():
                    flags += line.strip() + " "

        local_path = str(pathlib.Path().resolve())

        data = []
        with open(path + "/" + source_path) as f:
            for line in f:
                if line.strip():
                    file = line.strip()
                    data.append(
                        {
                            "directory": local_path,
                            "command": flags + "-c " + file,
                            "file": file,
                        }
                    )
        return data

    def generate(self):
        DB_SOURCE = [
            {
                "name": "ASM",
                "source": "db.asm_source.list",
                "flags": "db.asm_flags.list",
            },
            {"name": "C", "source": "db.c_source.list", "flags": "db.c_flags.list"},
            {
                "name": "CPP",
                "source": "db.cpp_source.list",
                "flags": "db.cpp_flags.list",
            },
        ]

        path = self.args.path
        out_data = []
        out_path = path + "/" + "compile_commands.json"
        out_file = open(out_path, mode="w")

        for record in DB_SOURCE:
            self.logger.info(
                f"Processing {record['name']} ({record['source']}, {record['flags']})"
            )
            data = self.parse_sources(path, record["source"], record["flags"])
            out_data += data

        self.logger.info(f"Saving")
        json.dump(out_data, out_file, indent=2)

        self.logger.info(f"Compilation DB written to " + out_path)
        return 0


if __name__ == "__main__":
    Main()()
