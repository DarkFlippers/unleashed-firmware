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

    def _replace_occurrence(self, sources: list, old: str, new: str):
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
        # Check sources for invalid filenames
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
        # Notify about errors or replace all occurrences
        if dry_run:
            if len(bad) > 0:
                self.logger.error(f"Found {len(bad)} incorrectly named files")
                self.logger.info(bad)
                return False
        else:
            # Replace occurrences in text files
            for source, old, new in bad:
                self._replace_occurrence(sources, old, new)
            # Rename files
            for source, old, new in bad:
                shutil.move(source, source.replace(old, new))
        return True

    def _apply_file_permissions(self, sources: list, dry_run: bool = False):
        execute_permissions = 0o111
        pattern = re.compile(SOURCE_CODE_FILE_PATTERN)
        good = []
        bad = []
        # Check sources for unexpected execute permissions
        for source in sources:
            st = os.stat(source)
            perms_too_many = st.st_mode & execute_permissions
            if perms_too_many:
                good_perms = st.st_mode & ~perms_too_many
                bad.append((source, oct(perms_too_many), good_perms))
            else:
                good.append(source)
        # Notify or fix
        if dry_run:
            if len(bad) > 0:
                self.logger.error(f"Found {len(bad)} incorrect permissions")
                self.logger.info([record[0:2] for record in bad])
                return False
        else:
            for source, perms_too_many, new_perms in bad:
                os.chmod(source, new_perms)
        return True

    def _perform(self, dry_run: bool):
        result = 0
        sources = self._find_sources(self.args.input)
        if not self._format_sources(sources, dry_run=dry_run):
            result |= 0b001
        if not self._apply_file_naming_convention(sources, dry_run=dry_run):
            result |= 0b010
        if not self._apply_file_permissions(sources, dry_run=dry_run):
            result |= 0b100
        self._check_folders(self.args.input)
        return result

    def check(self):
        return self._perform(dry_run=True)

    def format(self):
        return self._perform(dry_run=False)


if __name__ == "__main__":
    Main()()
