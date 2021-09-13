#!/usr/bin/env python3

import logging
import argparse
import subprocess
import io
import os
import sys

ICONS_SUPPORTED_FORMATS = ["png"]

ICONS_TEMPLATE_H_HEADER = """#pragma once
#include <gui/icon.h>

"""
ICONS_TEMPLATE_H_ICON_NAME = "extern const Icon {name};\n"

ICONS_TEMPLATE_C_HEADER = """#include \"assets_icons.h\"

#include <gui/icon_i.h>

"""
ICONS_TEMPLATE_C_FRAME = "const uint8_t {name}[] = {data};\n"
ICONS_TEMPLATE_C_DATA = "const uint8_t *{name}[] = {data};\n"
ICONS_TEMPLATE_C_ICONS = "const Icon {name} = {{.width={width},.height={height},.frame_count={frame_count},.frame_rate={frame_rate},.frames=_{name}}};\n"


class Main:
    def __init__(self):
        # command args
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("-d", "--debug", action="store_true", help="Debug")
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        self.parser_icons = self.subparsers.add_parser(
            "icons", help="Process icons and build icon registry"
        )
        self.parser_icons.add_argument("source_directory", help="Source directory")
        self.parser_icons.add_argument("output_directory", help="Output directory")
        self.parser_icons.set_defaults(func=self.icons)

        self.parser_manifest = self.subparsers.add_parser(
            "manifest", help="Create directory Manifest"
        )
        self.parser_manifest.add_argument("local_path", help="local_path")
        self.parser_manifest.set_defaults(func=self.manifest)

        self.parser_copro = self.subparsers.add_parser(
            "copro", help="Gather copro binaries for packaging"
        )
        self.parser_copro.add_argument("cube_dir", help="Path to Cube folder")
        self.parser_copro.add_argument("output_dir", help="Path to output folder")
        self.parser_copro.add_argument("mcu", help="MCU series as in copro folder")
        self.parser_copro.set_defaults(func=self.copro)

        # logging
        self.logger = logging.getLogger()

    def __call__(self):
        self.args = self.parser.parse_args()
        if "func" not in self.args:
            self.parser.error("Choose something to do")
        # configure log output
        self.log_level = logging.DEBUG if self.args.debug else logging.INFO
        self.logger.setLevel(self.log_level)
        self.handler = logging.StreamHandler(sys.stdout)
        self.handler.setLevel(self.log_level)
        self.formatter = logging.Formatter("%(asctime)s [%(levelname)s] %(message)s")
        self.handler.setFormatter(self.formatter)
        self.logger.addHandler(self.handler)
        # execute requested function
        self.args.func()

    def icons(self):
        self.logger.debug(f"Converting icons")
        icons_c = open(os.path.join(self.args.output_directory, "assets_icons.c"), "w")
        icons_c.write(ICONS_TEMPLATE_C_HEADER)
        icons = []
        # Traverse icons tree, append image data to source file
        for dirpath, dirnames, filenames in os.walk(self.args.source_directory):
            self.logger.debug(f"Processing directory {dirpath}")
            dirnames.sort()
            if not filenames:
                continue
            if "frame_rate" in filenames:
                self.logger.debug(f"Folder contatins animation")
                icon_name = "A_" + os.path.split(dirpath)[1].replace("-", "_")
                width = height = None
                frame_count = 0
                frame_rate = 0
                frame_names = []
                for filename in sorted(filenames):
                    fullfilename = os.path.join(dirpath, filename)
                    if filename == "frame_rate":
                        frame_rate = int(open(fullfilename, "r").read().strip())
                        continue
                    elif not self.iconIsSupported(filename):
                        continue
                    self.logger.debug(f"Processing animation frame {filename}")
                    temp_width, temp_height, data = self.icon2header(fullfilename)
                    if width is None:
                        width = temp_width
                    if height is None:
                        height = temp_height
                    assert width == temp_width
                    assert height == temp_height
                    frame_name = f"_{icon_name}_{frame_count}"
                    frame_names.append(frame_name)
                    icons_c.write(
                        ICONS_TEMPLATE_C_FRAME.format(name=frame_name, data=data)
                    )
                    frame_count += 1
                assert frame_rate > 0
                assert frame_count > 0
                icons_c.write(
                    ICONS_TEMPLATE_C_DATA.format(
                        name=f"_{icon_name}", data=f'{{{",".join(frame_names)}}}'
                    )
                )
                icons_c.write("\n")
                icons.append((icon_name, width, height, frame_rate, frame_count))
            else:
                # process icons
                for filename in filenames:
                    if not self.iconIsSupported(filename):
                        continue
                    self.logger.debug(f"Processing icon {filename}")
                    icon_name = "I_" + "_".join(filename.split(".")[:-1]).replace(
                        "-", "_"
                    )
                    fullfilename = os.path.join(dirpath, filename)
                    width, height, data = self.icon2header(fullfilename)
                    frame_name = f"_{icon_name}_0"
                    icons_c.write(
                        ICONS_TEMPLATE_C_FRAME.format(name=frame_name, data=data)
                    )
                    icons_c.write(
                        ICONS_TEMPLATE_C_DATA.format(
                            name=f"_{icon_name}", data=f"{{{frame_name}}}"
                        )
                    )
                    icons_c.write("\n")
                    icons.append((icon_name, width, height, 0, 1))
        # Create array of images:
        self.logger.debug(f"Finalizing source file")
        for name, width, height, frame_rate, frame_count in icons:
            icons_c.write(
                ICONS_TEMPLATE_C_ICONS.format(
                    name=name,
                    width=width,
                    height=height,
                    frame_rate=frame_rate,
                    frame_count=frame_count,
                )
            )
        icons_c.write("\n")
        # Create Public Header
        self.logger.debug(f"Creating header")
        icons_h = open(os.path.join(self.args.output_directory, "assets_icons.h"), "w")
        icons_h.write(ICONS_TEMPLATE_H_HEADER)
        for name, width, height, frame_rate, frame_count in icons:
            icons_h.write(ICONS_TEMPLATE_H_ICON_NAME.format(name=name))
        self.logger.debug(f"Done")

    def icon2header(self, file):
        output = subprocess.check_output(["convert", file, "xbm:-"])
        assert output
        f = io.StringIO(output.decode().strip())
        width = int(f.readline().strip().split(" ")[2])
        height = int(f.readline().strip().split(" ")[2])
        data = f.read().strip().replace("\n", "").replace(" ", "").split("=")[1][:-1]
        return width, height, data

    def iconIsSupported(self, filename):
        extension = filename.lower().split(".")[-1]
        return extension in ICONS_SUPPORTED_FORMATS

    def manifest(self):
        from flipper.manifest import Manifest

        directory_path = os.path.normpath(self.args.local_path)
        if not os.path.isdir(directory_path):
            self.logger.error(f'"{directory_path}" is not a directory')
            exit(255)
        manifest_file = os.path.join(directory_path, "Manifest")
        old_manifest = Manifest()
        if os.path.exists(manifest_file):
            self.logger.info(
                f"old manifest is present, loading for compare and removing file"
            )
            old_manifest.load(manifest_file)
            os.unlink(manifest_file)
        self.logger.info(f'Creating new Manifest for directory "{directory_path}"')
        new_manifest = Manifest()
        new_manifest.create(directory_path)
        new_manifest.save(manifest_file)
        self.logger.info(f"Comparing new manifest with old")
        only_in_old, changed, only_in_new = Manifest.compare(old_manifest, new_manifest)
        for record in only_in_old:
            self.logger.info(f"Only in old: {record}")
        for record in changed:
            self.logger.info(f"Changed: {record}")
        for record in only_in_new:
            self.logger.info(f"Only in new: {record}")
        self.logger.info(f"Complete")

    def copro(self):
        from flipper.copro import Copro

        copro = Copro(self.args.mcu)
        copro.loadCubeInfo(self.args.cube_dir)
        copro.bundle(self.args.output_dir)


if __name__ == "__main__":
    Main()()
