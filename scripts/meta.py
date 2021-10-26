#!/usr/bin/env python3

import argparse
import json


class Main:
    def __init__(self):
        # parse CFLAGS
        self.parser = argparse.ArgumentParser(allow_abbrev=False)
        self.parser.add_argument("-p", dest="project", required=True)
        self.parser.add_argument("-DBUILD_DATE", dest="build_date", required=True)
        self.parser.add_argument("-DGIT_COMMIT", dest="commit", required=True)
        self.parser.add_argument("-DGIT_BRANCH", dest="branch", required=True)
        self.parser.add_argument("-DTARGET", dest="target", type=int, required=True)

    def __call__(self):
        self.args, _ = self.parser.parse_known_args()

        meta = {}
        for k, v in vars(self.args).items():
            if k == "project":
                continue
            if isinstance(v, str):
                v = v.strip('"')
            meta[self.args.project + "_" + k] = v

        print(json.dumps(meta, indent=4))


if __name__ == "__main__":
    Main()()
