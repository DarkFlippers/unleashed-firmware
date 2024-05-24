#!/usr/bin/env python3

import os
import requests
import argparse
import subprocess

# usage:
# COMMIT_HASH, COMMIT_MSG, BRANCH_NAME,
# PULL_ID(optional), PULL_NAME(optional) must be set as envs
# maybe from sctipts/get_env.py
# other args must be set via command line args


class AnalyseRequest:
    def __init__(self):
        self.commit_hash = os.environ["COMMIT_HASH"]
        self.commit_msg = os.environ["COMMIT_MSG"]
        self.branch_name = os.environ["BRANCH_NAME"]
        self.pull_id = os.getenv("PULL_ID", default=None)
        self.pull_name = os.getenv("PULL_NAME", default=None)

    def get_payload(self):
        return vars(self)


class AnalyseUploader:
    def __init__(self):
        self.args = self.parse_args()

    @staticmethod
    def get_sections_size(elf_file) -> dict:
        ret = dict()
        all_sizes = subprocess.check_output(
            ["arm-none-eabi-size", "-A", elf_file], shell=False
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
            section_size_payload_name = (
                section[1:] if section.startswith(".") else section
            )
            section_size_payload_name += "_size"
            ret[section_size_payload_name] = size
        return ret

    @staticmethod
    def parse_args():
        parser = argparse.ArgumentParser()
        parser.add_argument("--elf_file", help="Firmware ELF file", required=True)
        parser.add_argument("--map_file", help="Firmware MAP file", required=True)
        parser.add_argument(
            "--analyser_token", help="Analyser auth token", required=True
        )
        parser.add_argument(
            "--analyser_url", help="Analyser analyse url", required=True
        )
        args = parser.parse_args()
        return args

    def upload_analyse_request(self):
        payload = AnalyseRequest().get_payload() | self.get_sections_size(
            self.args.elf_file
        )
        headers = {"Authorization": f"Bearer {self.args.analyser_token}"}
        file = {"map_file": open(self.args.map_file, "rb")}
        response = requests.post(
            self.args.analyser_url, data=payload, files=file, headers=headers
        )
        if not response.ok:
            raise Exception(
                f"Failed to upload map file, code: {response.status_code}, reason: {response.text}"
            )


if __name__ == "__main__":
    analyzer = AnalyseUploader()
    analyzer.upload_analyse_request()
