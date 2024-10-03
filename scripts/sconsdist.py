#!/usr/bin/env python3

import json
import shutil
import tarfile
import zipfile
from os import makedirs, walk
from os.path import basename, exists, join, relpath

from ansi.color import fg
from flipper.app import App
from flipper.assets.tarball import FLIPPER_TAR_FORMAT, tar_sanitizer_filter
from update import Main as UpdateMain


class ProjectDir:
    def __init__(self, project_dir):
        self.dir = project_dir
        parts = project_dir.split("-")
        self.target = parts[0]
        self.project = parts[1]
        self.flavor = parts[2] if len(parts) > 2 else ""


class Main(App):
    DIST_FILE_PREFIX = "flipper-z-"
    DIST_FOLDER_MAX_NAME_LENGTH = 80

    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_copy = self.subparsers.add_parser(
            "copy", help="Copy firmware binaries & metadata"
        )

        self.parser_copy.add_argument("-p", dest="project", nargs="+", required=True)
        self.parser_copy.add_argument("-s", dest="suffix", required=True)
        self.parser_copy.add_argument("-r", dest="resources", required=False)
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

    def get_project_file_name(self, project: ProjectDir, filetype: str) -> str:
        #  Temporary fix
        project_name = project.project
        if project_name == "firmware" and filetype != "elf":
            project_name = "full"

        dist_target_path = self.get_dist_file_name(project_name, filetype)
        self.note_dist_component(
            project_name, filetype, self.get_dist_path(dist_target_path)
        )
        return dist_target_path

    def note_dist_component(self, component: str, extension: str, srcpath: str) -> None:
        component_key = f"{component}.{extension}"
        if component_key in self._dist_components:
            self.logger.debug(
                f"Skipping duplicate component {component_key} in {srcpath}"
            )
            return
        self._dist_components[component_key] = srcpath

    def get_dist_file_name(self, dist_artifact_type: str, filetype: str) -> str:
        return f"{self.DIST_FILE_PREFIX}{self.target}-{dist_artifact_type}-{self.args.suffix}.{filetype}"

    def get_dist_path(self, filename: str) -> str:
        return join(self.output_dir_path, filename)

    def copy_single_project(self, project: ProjectDir) -> None:
        obj_directory = join("build", project.dir)

        for filetype in ("elf", "bin", "dfu", "json"):
            if exists(src_file := join(obj_directory, f"{project.project}.{filetype}")):
                shutil.copyfile(
                    src_file,
                    self.get_dist_path(self.get_project_file_name(project, filetype)),
                )
        for foldertype in ("sdk_headers", "lib"):
            if exists(sdk_folder := join(obj_directory, foldertype)):
                self.note_dist_component(foldertype, "dir", sdk_folder)

    def copy(self) -> int:
        self._dist_components: dict[str, str] = dict()
        self.projects: dict[str, ProjectDir] = dict(
            map(
                lambda pd: (pd.project, pd),
                map(ProjectDir, self.args.project),
            )
        )

        project_targets = set(map(lambda p: p.target, self.projects.values()))
        if len(project_targets) != 1:
            self.logger.error(f"Cannot mix targets {project_targets}!")
            return 1
        self.target = project_targets.pop()

        project_flavors = set(map(lambda p: p.flavor, self.projects.values()))
        if len(project_flavors) != 1:
            self.logger.error(f"Cannot mix flavors {project_flavors}!")
            return 2
        self.flavor = project_flavors.pop()

        dist_dir_components = [self.target]
        if self.flavor:
            dist_dir_components.append(self.flavor)

        self.output_dir_path = join("dist", "-".join(dist_dir_components))
        if exists(self.output_dir_path) and not self.args.noclean:
            try:
                shutil.rmtree(self.output_dir_path)
            except Exception as ex:
                self.logger.warning(f"Failed to clean output directory: {ex}")

        if not exists(self.output_dir_path):
            self.logger.debug(f"Creating output directory {self.output_dir_path}")
            makedirs(self.output_dir_path)

        for folder in ("debug", "scripts"):
            if exists(folder):
                self.note_dist_component(folder, "dir", folder)

        for project in self.projects.values():
            self.logger.debug(f"Copying {project.project} for {project.target}")
            self.copy_single_project(project)

        self.logger.info(
            fg.boldgreen(
                f"Firmware binaries can be found at:\n\t{self.output_dir_path}"
            )
        )

        if self.args.version:
            if bundle_result := self.bundle_update_package():
                return bundle_result

        required_components = ("firmware.elf", "full.bin", "update.dir")
        if all(
            map(
                lambda c: c in self._dist_components,
                required_components,
            )
        ):
            self.bundle_sdk()

        return 0

    def bundle_sdk(self):
        self.logger.info("Bundling SDK")
        components_paths = dict()

        sdk_components_keys = (
            "full.bin",
            "firmware.elf",
            "update.dir",
            "sdk_headers.dir",
            "lib.dir",
            "scripts.dir",
        )

        sdk_bundle_path = self.get_dist_path(self.get_dist_file_name("sdk", "zip"))
        with zipfile.ZipFile(
            sdk_bundle_path,
            "w",
            zipfile.ZIP_DEFLATED,
        ) as zf:
            for component_key in sdk_components_keys:
                component_path = self._dist_components.get(component_key)

                if component_key.endswith(".dir"):
                    components_paths[component_key] = basename(component_path)
                    for root, dirnames, files in walk(component_path):
                        if "__pycache__" in dirnames:
                            dirnames.remove("__pycache__")
                        for file in files:
                            zf.write(
                                join(root, file),
                                join(
                                    components_paths[component_key],
                                    relpath(
                                        join(root, file),
                                        component_path,
                                    ),
                                ),
                            )
                else:
                    # We use fixed names for files to avoid having to regenerate VSCode project
                    components_paths[component_key] = component_key
                    zf.write(component_path, component_key)

            zf.writestr(
                "components.json",
                json.dumps(
                    {
                        "meta": {
                            "hw_target": self.target,
                            "flavor": self.flavor,
                            "version": self.args.version,
                        },
                        "components": components_paths,
                    }
                ),
            )

        self.logger.info(
            fg.boldgreen(f"SDK bundle can be found at:\n\t{sdk_bundle_path}")
        )

    def bundle_update_package(self):
        self.logger.debug(
            f"Generating update bundle with version {self.args.version} for {self.target}"
        )
        bundle_dir_name = f"{self.target}-update-{self.args.suffix}"[
            : self.DIST_FOLDER_MAX_NAME_LENGTH
        ]
        bundle_dir = self.get_dist_path(bundle_dir_name)
        bundle_args = [
            "generate",
            "-d",
            bundle_dir,
            "-v",
            self.args.version,
            "-t",
            self.target,
            "--dfu",
            self.get_dist_path(
                self.get_project_file_name(self.projects["firmware"], "dfu")
            ),
            "--stage",
            self.get_dist_path(
                self.get_project_file_name(self.projects["updater"], "bin")
            ),
        ]
        if self.args.resources:
            bundle_args.extend(
                (
                    "-r",
                    self.args.resources,
                )
            )
        bundle_args.extend(self.other_args)

        if (bundle_result := UpdateMain(no_exit=True)(bundle_args)) == 0:
            self.note_dist_component("update", "dir", bundle_dir)
            self.logger.info(
                fg.boldgreen(
                    f"Use this directory to self-update your Flipper:\n\t{bundle_dir}"
                )
            )

            # Create tgz archive
            with tarfile.open(
                join(
                    self.output_dir_path,
                    bundle_tgz := f"{self.DIST_FILE_PREFIX}{bundle_dir_name}.tgz",
                ),
                "w:gz",
                compresslevel=9,
                format=FLIPPER_TAR_FORMAT,
            ) as tar:
                self.note_dist_component(
                    "update", "tgz", self.get_dist_path(bundle_tgz)
                )

                tar.add(
                    bundle_dir, arcname=bundle_dir_name, filter=tar_sanitizer_filter
                )
        return bundle_result


if __name__ == "__main__":
    Main()()
