#!/usr/bin/env python3

from os import path

from flipper.app import App
from flipper.utils.fff import *


class Main(App):
    def init(self):
        # Subparsers
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_cleanup = self.subparsers.add_parser(
            "cleanup", help="Cleanup duplicate remotes"
        )
        self.parser_cleanup.add_argument("filename", type=str)
        self.parser_cleanup.set_defaults(func=self.cleanup)

    def cleanup(self):
        f = FlipperFormatFile()
        f.load(self.args.filename)

        filetype, version = f.getHeader()
        if filetype != "IR library file" or version != 1:
            self.logger.error(f"Incorrect file type({filetype}) or version({version})")
            return 1

        data = []
        unique_combo = {}
        unique_payload = {}
        while True:
            try:
                d = {}
                d["comments"] = []
                while (comment := f.readComment()) is not None:
                    d["comments"].append(comment)
                d["name"] = f.readKey("name")
                d["type"] = f.readKey("type")
                key_combo = f'{d["name"]}'
                key_payload = None
                if d["type"] == "parsed":
                    d["protocol"] = f.readKey("protocol")
                    d["address"] = f.readKey("address")
                    d["command"] = f.readKey("command")
                    key_payload = f'{d["protocol"]}{d["address"]}{d["command"]}'
                    key_combo += key_payload
                elif d["type"] == "raw":
                    d["frequency"] = f.readKey("frequency")
                    d["duty_cycle"] = f.readKey("duty_cycle")
                    d["data"] = f.readKey("data")
                    key_payload = f'{d["frequency"]}{d["duty_cycle"]}{d["data"]}'
                    key_combo += key_payload
                else:
                    raise Exception(f'Unknown type: {d["type"]}')

                if not key_combo in unique_combo:
                    unique_combo[key_combo] = d
                    data.append(d)
                    # Check payload only
                    if not key_payload in unique_payload:
                        unique_payload[key_payload] = d
                    else:
                        self.logger.warning(f"Duplicate payload, check manually: {d}")
                else:
                    self.logger.info(f"Duplicate data removed: {d}")
            except EOFError:
                break
        # Form new file
        f = FlipperFormatFile()
        f.setHeader(filetype, version)
        for i in data:
            for comment in i["comments"]:
                f.writeComment(comment)
            f.writeKey("name", i["name"])
            f.writeKey("type", i["type"])
            if i["type"] == "parsed":
                f.writeKey("protocol", i["protocol"])
                f.writeKey("address", i["address"])
                f.writeKey("command", i["command"])
            elif i["type"] == "raw":
                f.writeKey("frequency", i["frequency"])
                f.writeKey("duty_cycle", i["duty_cycle"])
                f.writeKey("data", i["data"])
            else:
                raise Exception(f'Unknown type: {i["type"]}')
        f.save(self.args.filename)

        return 0


if __name__ == "__main__":
    Main()()
