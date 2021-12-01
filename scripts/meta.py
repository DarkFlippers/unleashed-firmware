#!/usr/bin/env python3

from flipper.app import App
import json


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # generate
        self.parser_generate = self.subparsers.add_parser(
            "generate", help="Generate JSON meta file"
        )
        self.parser_generate.add_argument("-p", dest="project", required=True)
        self.parser_generate.add_argument(
            "-DBUILD_DATE", dest="build_date", required=True
        )
        self.parser_generate.add_argument("-DGIT_COMMIT", dest="commit", required=True)
        self.parser_generate.add_argument("-DGIT_BRANCH", dest="branch", required=True)
        self.parser_generate.add_argument(
            "-DTARGET", dest="target", type=int, required=True
        )
        self.parser_generate.set_defaults(func=self.generate)

        # merge
        self.parser_merge = self.subparsers.add_parser(
            "merge", help="Merge JSON meta files"
        )
        self.parser_merge.add_argument(
            "-i", dest="input", action="append", nargs="+", required=True
        )
        self.parser_merge.set_defaults(func=self.merge)

    def generate(self):
        meta = {}
        for k, v in vars(self.args).items():
            if k in ["project", "func", "debug"]:
                continue
            if isinstance(v, str):
                v = v.strip('"')
            meta[self.args.project + "_" + k] = v

        print(json.dumps(meta, indent=4))
        return 0

    def merge(self):
        full = {}
        for path in self.args.input[0]:
            with open(path, mode="r") as file:
                dict = json.loads(file.read())
                full.update(dict)

        print(json.dumps(full, indent=4))
        return 0


if __name__ == "__main__":
    Main()()
