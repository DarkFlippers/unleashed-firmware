#!/usr/bin/env python3

from flipper.app import App
from os.path import join, exists
from os import makedirs
from update import Main as UpdateMain
import shutil


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_copy = self.subparsers.add_parser(
            "copy", help="Copy firmware binaries & metadata"
        )

        self.parser_copy.add_argument("-t", dest="target", required=True)
        self.parser_copy.add_argument("-p", dest="projects", nargs="+", required=True)
        self.parser_copy.add_argument("-s", dest="suffix", required=True)
        self.parser_copy.add_argument(
            "--bundlever",
            dest="version",
            help="If set, bundle update package for self-update",
            required=False,
        )
        self.parser_copy.add_argument(
            "--noclean",
            dest="noclean",
            action="store_true",
            help="Don't clean output directory",
            required=False,
        )
        self.parser_copy.set_defaults(func=self.copy)

    def get_project_filename(self, project, filetype):
        #  Temporary fix
        if project == "firmware" and filetype != "elf":
            project = "full"
        return f"flipper-z-{self.args.target}-{project}-{self.args.suffix}.{filetype}"

    def get_dist_filepath(self, filename):
        return join(self.output_dir_path, filename)

    def copy_single_project(self, project):
        target_project = f"{self.args.target}-{project}"
        obj_directory = join("firmware", ".obj", target_project)

        for filetype in ("elf", "bin", "dfu", "json"):
            shutil.copyfile(
                join(obj_directory, f"{project}.{filetype}"),
                self.get_dist_filepath(self.get_project_filename(project, filetype)),
            )

    def copy(self):
        self.output_dir_path = join("dist", self.args.target)
        if exists(self.output_dir_path) and not self.args.noclean:
            shutil.rmtree(self.output_dir_path)

        if not exists(self.output_dir_path):
            makedirs(self.output_dir_path)

        for project in self.args.projects:
            self.copy_single_project(project)

        self.logger.info(
            f"Firmware binaries can be found at:\n\t{self.output_dir_path}"
        )
        if self.args.version:
            bundle_dir = join(
                self.output_dir_path, f"{self.args.target}-update-{self.args.suffix}"
            )
            bundle_args = [
                "generate",
                "-d",
                bundle_dir,
                "-v",
                self.args.version,
                "-t",
                self.args.target,
                "-dfu",
                self.get_dist_filepath(self.get_project_filename("firmware", "dfu")),
                "-stage",
                self.get_dist_filepath(self.get_project_filename("updater", "bin")),
            ]
            self.logger.info(
                f"Use this directory to self-update your Flipper:\n\t{bundle_dir}"
            )
            return UpdateMain(no_exit=True)(bundle_args)

        return 0


if __name__ == "__main__":
    Main()()
