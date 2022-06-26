#!/usr/bin/env python3

import os
import re
import shutil
import subprocess
import multiprocessing

from flipper.app import App


SOURCE_CODE_FILE_EXTENSIONS = [".h", ".c", ".cpp", ".cxx", ".hpp"]
SOURCE_CODE_FILE_PATTERN = r"^[0-9A-Za-z_]+\.[a-z]+$"
SOURCE_CODE_DIR_PATTERN = r"^[0-9A-Za-z_]+$"


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # generate
        self.parser_check = self.subparsers.add_parser(
            "check", help="Check source code format and file names"
        )
        self.parser_check.add_argument("input", nargs="+")
        self.parser_check.set_defaults(func=self.check)

        # merge
        self.parser_format = self.subparsers.add_parser(
            "format", help="Format source code and fix file names"
        )
        self.parser_format.add_argument(
            "input",
            nargs="+",
        )
        self.parser_format.set_defaults(func=self.format)

    def _check_folders(self, folders: list):
        show_message = False
        pattern = re.compile(SOURCE_CODE_DIR_PATTERN)
        for folder in folders:
            for dirpath, dirnames, filenames in os.walk(folder):
                for dirname in dirnames:
                    if not pattern.match(dirname):
                        to_fix = os.path.join(dirpath, dirname)
                        self.logger.warning(f"Found incorrectly named folder {to_fix}")
                        show_message = True
        if show_message:
            self.logger.warning(
                f"Folders are not renamed automatically, please fix it by yourself"
            )

    def _find_sources(self, folders: list):
        output = []
        for folder in folders:
            for dirpath, dirnames, filenames in os.walk(folder):
                for filename in filenames:
                    ext = os.path.splitext(filename.lower())[1]
                    if not ext in SOURCE_CODE_FILE_EXTENSIONS:
                        continue
                    output.append(os.path.join(dirpath, filename))
        return output

    @staticmethod
    def _format_source(task):
        try:
            subprocess.check_call(task)
            return True
        except subprocess.CalledProcessError as e:
            return False

    def _format_sources(self, sources: list, dry_run: bool = False):
        args = ["clang-format", "--Werror", "--style=file", "-i"]
        if dry_run:
            args.append("--dry-run")

        files_per_task = 69
        tasks = []
        while len(sources) > 0:
            tasks.append(args + sources[:files_per_task])
            sources = sources[files_per_task:]

        pool = multiprocessing.Pool()
        results = pool.map(self._format_source, tasks)

        return all(results)

    def _fix_filename(self, filename: str):
        return filename.replace("-", "_")

    def _replace_occurance(self, sources: list, old: str, new: str):
        old = old.encode()
        new = new.encode()
        for source in sources:
            content = open(source, "rb").read()
            if content.count(old) > 0:
                self.logger.info(f"Replacing {old} with {new} in {source}")
                content = content.replace(old, new)
                open(source, "wb").write(content)

    def _apply_file_naming_convention(self, sources: list, dry_run: bool = False):
        pattern = re.compile(SOURCE_CODE_FILE_PATTERN)
        good = []
        bad = []
        # Check sources for invalid filesname
        for source in sources:
            basename = os.path.basename(source)
            if not pattern.match(basename):
                new_basename = self._fix_filename(basename)
                if not pattern.match(new_basename):
                    self.logger.error(f"Unable to fix name for {basename}")
                    return False
                bad.append((source, basename, new_basename))
            else:
                good.append(source)
        # Notify about errors or replace all occurances
        if dry_run:
            if len(bad) > 0:
                self.logger.error(f"Found {len(bad)} incorrectly named files")
                self.logger.info(bad)
                return False
        else:
            # Replace occurances in text files
            for source, old, new in bad:
                self._replace_occurance(sources, old, new)
            # Rename files
            for source, old, new in bad:
                shutil.move(source, source.replace(old, new))
        return True

    def check(self):
        result = 0
        sources = self._find_sources(self.args.input)
        if not self._format_sources(sources, dry_run=True):
            result |= 0b01
        if not self._apply_file_naming_convention(sources, dry_run=True):
            result |= 0b10
        self._check_folders(self.args.input)
        return result

    def format(self):
        result = 0
        sources = self._find_sources(self.args.input)
        if not self._format_sources(sources):
            result |= 0b01
        if not self._apply_file_naming_convention(sources):
            result |= 0b10
        self._check_folders(self.args.input)
        return result


if __name__ == "__main__":
    Main()()
