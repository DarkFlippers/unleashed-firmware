import multiprocessing
import logging
import os
import sys
import shutil
from collections import Counter

from flipper.utils.fff import *
from flipper.utils.templite import *
from .icon import *


def _convert_image_to_bm(pair: set):
    source_filename, destination_filename = pair
    image = file2image(source_filename)
    image.write(destination_filename)


def _convert_image(source_filename: str):
    image = file2image(source_filename)
    return image.data


class DolphinBubbleAnimation:
    FILE_TYPE = "Flipper Animation"
    FILE_VERSION = 1

    def __init__(
        self,
        name: str,
        min_butthurt: int,
        max_butthurt: int,
        min_level: int,
        max_level: int,
        weight: int,
    ):
        # Manifest
        self.name = name
        self.min_butthurt = min_butthurt
        self.max_butthurt = max_butthurt
        self.min_level = min_level
        self.max_level = max_level
        self.weight = weight
        # Meta and data
        self.meta = {}
        self.frames = []
        self.bubbles = []
        self.bubble_slots = None
        # Logging
        self.logger = logging.getLogger("DolphinBubbleAnimation")

    def load(self, animation_directory: str):
        if not os.path.isdir(animation_directory):
            raise Exception(f"Animation folder doesn't exists: { animation_directory }")

        meta_filename = os.path.join(animation_directory, "meta.txt")
        if not os.path.isfile(meta_filename):
            raise Exception(f"Animation meta file doesn't exists: { meta_filename }")

        self.logger.info(f"Loading meta from {meta_filename}")
        file = FlipperFormatFile()
        file.load(meta_filename)

        # Check file header
        filetype, version = file.getHeader()
        assert filetype == self.FILE_TYPE
        assert version == self.FILE_VERSION

        max_frame_number = None
        unique_frames = None
        total_frames_count = None

        try:
            # Main meta
            self.meta["Width"] = file.readKeyInt("Width")
            self.meta["Height"] = file.readKeyInt("Height")
            self.meta["Passive frames"] = file.readKeyInt("Passive frames")
            self.meta["Active frames"] = file.readKeyInt("Active frames")
            self.meta["Frames order"] = file.readKeyIntArray("Frames order")
            self.meta["Active cycles"] = file.readKeyInt("Active cycles")
            self.meta["Frame rate"] = file.readKeyInt("Frame rate")
            self.meta["Duration"] = file.readKeyInt("Duration")
            self.meta["Active cooldown"] = file.readKeyInt("Active cooldown")
            self.bubble_slots = file.readKeyInt("Bubble slots")

            # Sanity Check
            assert self.meta["Width"] > 0 and self.meta["Width"] <= 128
            assert self.meta["Height"] > 0 and self.meta["Height"] <= 128
            assert self.meta["Passive frames"] > 0
            assert self.meta["Active frames"] >= 0
            assert self.meta["Frames order"]
            if self.meta["Active frames"] > 0:
                assert self.meta["Active cooldown"] > 0
                assert self.meta["Active cycles"] > 0
            else:
                assert self.meta["Active cooldown"] == 0
                assert self.meta["Active cycles"] == 0
            assert self.meta["Frame rate"] > 0
            assert self.meta["Duration"] >= 0

            # Frames sanity check
            max_frame_number = max(self.meta["Frames order"])
            ordered_frames_count = len(self.meta["Frames order"])
            for i in range(max_frame_number + 1):
                frame_filename = os.path.join(animation_directory, f"frame_{i}.png")
                assert os.path.isfile(frame_filename)
                self.frames.append(frame_filename)
            # Sanity check
            unique_frames = set(self.meta["Frames order"])
            unique_frames_count = len(unique_frames)
            if unique_frames_count != max_frame_number + 1:
                self.logger.warning(f"Not all frames were used in {self.name}")
            total_frames_count = self.meta["Passive frames"] + (
                self.meta["Active frames"] * self.meta["Active cycles"]
            )

            # Extra checks
            assert self.meta["Passive frames"] <= total_frames_count
            assert self.meta["Active frames"] <= total_frames_count
            assert (
                self.meta["Passive frames"] + self.meta["Active frames"]
                == ordered_frames_count
            )
        except EOFError as e:
            raise Exception("Invalid meta file: too short")
        except AssertionError as e:
            self.logger.exception(e)
            self.logger.error(f"Animation {self.name} got incorrect meta")
            raise Exception("Meta file is invalid: incorrect data")

        # Bubbles
        while True:
            try:
                # Bubble data
                bubble = {}
                bubble["Slot"] = file.readKeyInt("Slot")
                bubble["X"] = file.readKeyInt("X")
                bubble["Y"] = file.readKeyInt("Y")
                bubble["Text"] = file.readKey("Text")
                bubble["AlignH"] = file.readKey("AlignH")
                bubble["AlignV"] = file.readKey("AlignV")
                bubble["StartFrame"] = file.readKeyInt("StartFrame")
                bubble["EndFrame"] = file.readKeyInt("EndFrame")

                # Sanity check
                assert bubble["Slot"] <= self.bubble_slots
                assert bubble["X"] >= 0 and bubble["X"] < 128
                assert bubble["Y"] >= 0 and bubble["Y"] < 128
                assert len(bubble["Text"]) > 0
                assert bubble["AlignH"] in ["Left", "Center", "Right"]
                assert bubble["AlignV"] in ["Bottom", "Center", "Top"]
                assert bubble["StartFrame"] < total_frames_count
                assert bubble["EndFrame"] < total_frames_count
                assert bubble["EndFrame"] >= bubble["StartFrame"]

                # Store bubble
                self.bubbles.append(bubble)
            except AssertionError as e:
                self.logger.exception(e)
                self.logger.error(
                    f"Animation {self.name} bubble slot {bubble_slot} got incorrect data: {bubble}"
                )
                raise Exception("Meta file is invalid: incorrect bubble data")
            except EOFError:
                break

    def prepare(self):
        bubbles_in_slots = Counter([bubble["Slot"] for bubble in self.bubbles])

        last_slot = -1
        bubble_index = 0
        for bubble in self.bubbles:
            slot = bubble["Slot"]
            if slot == last_slot:
                bubble_index += 1
            else:
                last_slot = slot
                bubble_index = 0
            bubble["_BubbleIndex"] = bubble_index

            bubbles_in_slots[slot] -= 1
            if bubbles_in_slots[slot] != 0:
                bubble["_NextBubbleIndex"] = bubble_index + 1

    def save(self, output_directory: str):
        animation_directory = os.path.join(output_directory, self.name)
        os.makedirs(animation_directory, exist_ok=True)
        meta_filename = os.path.join(animation_directory, "meta.txt")

        file = FlipperFormatFile()
        file.setHeader(self.FILE_TYPE, self.FILE_VERSION)
        file.writeEmptyLine()

        # Write meta data
        file.writeKey("Width", self.meta["Width"])
        file.writeKey("Height", self.meta["Height"])
        file.writeKey("Passive frames", self.meta["Passive frames"])
        file.writeKey("Active frames", self.meta["Active frames"])
        file.writeKey("Frames order", self.meta["Frames order"])
        file.writeKey("Active cycles", self.meta["Active cycles"])
        file.writeKey("Frame rate", self.meta["Frame rate"])
        file.writeKey("Duration", self.meta["Duration"])
        file.writeKey("Active cooldown", self.meta["Active cooldown"])
        file.writeEmptyLine()

        file.writeKey("Bubble slots", self.bubble_slots)
        file.writeEmptyLine()

        # Write bubble data
        for bubble in self.bubbles:
            file.writeKey("Slot", bubble["Slot"])
            file.writeKey("X", bubble["X"])
            file.writeKey("Y", bubble["Y"])
            file.writeKey("Text", bubble["Text"])
            file.writeKey("AlignH", bubble["AlignH"])
            file.writeKey("AlignV", bubble["AlignV"])
            file.writeKey("StartFrame", bubble["StartFrame"])
            file.writeKey("EndFrame", bubble["EndFrame"])
            file.writeEmptyLine()

        file.save(meta_filename)

        to_pack = []
        for index, frame in enumerate(self.frames):
            to_pack.append(
                (frame, os.path.join(animation_directory, f"frame_{index}.bm"))
            )

        if ImageTools.is_processing_slow():
            pool = multiprocessing.Pool()
            pool.map(_convert_image_to_bm, to_pack)
        else:
            for image in to_pack:
                _convert_image_to_bm(image)

    def process(self):
        if ImageTools.is_processing_slow():
            pool = multiprocessing.Pool()
            self.frames = pool.map(_convert_image, self.frames)
        else:
            self.frames = list(_convert_image(frame) for frame in self.frames)


class DolphinManifest:
    FILE_TYPE = "Flipper Animation Manifest"
    FILE_VERSION = 1

    TEMPLATE_DIRECTORY = os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "templates"
    )
    TEMPLATE_H = os.path.join(TEMPLATE_DIRECTORY, "dolphin.h.tmpl")
    TEMPLATE_C = os.path.join(TEMPLATE_DIRECTORY, "dolphin.c.tmpl")

    def __init__(self):
        self.animations = []
        self.logger = logging.getLogger("DolphinManifest")

    def load(self, source_directory: str):
        manifest_filename = os.path.join(source_directory, "manifest.txt")

        file = FlipperFormatFile()
        file.load(manifest_filename)

        # Check file header
        filetype, version = file.getHeader()
        assert filetype == self.FILE_TYPE
        assert version == self.FILE_VERSION

        # Load animation data
        while True:
            try:
                # Read animation spcification
                name = file.readKey("Name")
                min_butthurt = file.readKeyInt("Min butthurt")
                max_butthurt = file.readKeyInt("Max butthurt")
                min_level = file.readKeyInt("Min level")
                max_level = file.readKeyInt("Max level")
                weight = file.readKeyInt("Weight")

                assert len(name) > 0
                assert min_butthurt >= 0
                assert max_butthurt >= 0 and max_butthurt >= min_butthurt
                assert min_level >= 0
                assert max_level >= 0 and max_level >= min_level
                assert weight >= 0

                # Initialize animation
                animation = DolphinBubbleAnimation(
                    name, min_butthurt, max_butthurt, min_level, max_level, weight
                )

                # Load Animation meta and frames
                animation.load(os.path.join(source_directory, name))

                # Add to array
                self.animations.append(animation)
            except EOFError:
                break

    def _renderTemplate(self, template_filename: str, output_filename: str, **kwargs):
        template = Templite(filename=template_filename)
        output = template.render(**kwargs)
        with open(output_filename, "w", newline="\n") as file:
            file.write(output)

    def save2code(self, output_directory: str, symbol_name: str):
        # Process frames
        for animation in self.animations:
            animation.process()

        # Prepare substitution data
        for animation in self.animations:
            animation.prepare()

        # Render Header
        self._renderTemplate(
            self.TEMPLATE_H,
            os.path.join(output_directory, f"assets_{symbol_name}.h"),
            animations=self.animations,
            symbol_name=symbol_name,
        )
        # Render Source
        self._renderTemplate(
            self.TEMPLATE_C,
            os.path.join(output_directory, f"assets_{symbol_name}.c"),
            animations=self.animations,
            symbol_name=symbol_name,
        )

    def save2folder(self, output_directory: str):
        manifest_filename = os.path.join(output_directory, "manifest.txt")
        file = FlipperFormatFile()
        file.setHeader(self.FILE_TYPE, self.FILE_VERSION)
        file.writeEmptyLine()

        for animation in self.animations:
            file.writeKey("Name", animation.name)
            file.writeKey("Min butthurt", animation.min_butthurt)
            file.writeKey("Max butthurt", animation.max_butthurt)
            file.writeKey("Min level", animation.min_level)
            file.writeKey("Max level", animation.max_level)
            file.writeKey("Weight", animation.weight)
            file.writeEmptyLine()

            animation.save(output_directory)

        file.save(manifest_filename)

    def save(self, output_directory: str, symbol_name: str):
        os.makedirs(output_directory, exist_ok=True)
        if symbol_name:
            self.save2code(output_directory, symbol_name)
        else:
            self.save2folder(output_directory)


class Dolphin:
    def __init__(self):
        self.manifest = DolphinManifest()
        self.logger = logging.getLogger("Dolphin")

    def load(self, source_directory: str):
        assert os.path.isdir(source_directory)
        # Load Manifest
        self.logger.info(f"Loading directory {source_directory}")
        self.manifest.load(source_directory)

    def pack(self, output_directory: str, symbol_name: str = None):
        self.manifest.save(output_directory, symbol_name)
