import logging
import multiprocessing
import os
from pathlib import Path

from flipper.app import App
from PIL import Image

_logger = logging.getLogger(__name__)


def _check_image(image, do_fixup=False):
    failed_checks = []
    with Image.open(image) as img:
        # check that is's pure 1-bit B&W
        if img.mode != "1":
            failed_checks.append(f"not 1-bit B&W, but {img.mode}")
            if do_fixup:
                img = img.convert("1")

        # ...and does not have any metadata or ICC profile
        if img.info:
            failed_checks.append(f"has metadata")
            if do_fixup:
                img.info = {}

        if do_fixup:
            img.save(image)
            _logger.info(f"Fixed image {image}")

    if failed_checks:
        _logger.warning(f"Image {image} issues: {'; '.join(failed_checks)}")
    return len(failed_checks) == 0


class ImageLint(App):
    ICONS_SUPPORTED_FORMATS = [".png"]

    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_check = self.subparsers.add_parser(
            "check", help="Check image format and file names"
        )
        self.parser_check.add_argument("input", nargs="+")
        self.parser_check.set_defaults(func=self.check)

        self.parser_format = self.subparsers.add_parser(
            "format", help="Format image and fix file names"
        )
        self.parser_format.add_argument(
            "input",
            nargs="+",
        )
        self.parser_format.set_defaults(func=self.format)

    def _gather_images(self, folders):
        images = []
        for folder in folders:
            for dirpath, _, filenames in os.walk(folder):
                for filename in filenames:
                    if self.is_file_an_icon(filename):
                        images.append(os.path.join(dirpath, filename))
        return images

    def is_file_an_icon(self, filename):
        extension = Path(filename).suffix.lower()
        return extension in self.ICONS_SUPPORTED_FORMATS

    def _process_images(self, images, do_fixup):
        with multiprocessing.Pool() as pool:
            image_checks = pool.starmap(
                _check_image, [(image, do_fixup) for image in images]
            )
        return all(image_checks)

    def check(self):
        images = self._gather_images(self.args.input)
        self.logger.info(f"Found {len(images)} images")
        if not self._process_images(images, False):
            self.logger.error("Some images are not in the correct format")
            return 1
        self.logger.info("All images are in the correct format")
        return 0

    def format(self):
        images = self._gather_images(self.args.input)
        self.logger.info(f"Found {len(images)} images")
        if not self._process_images(images, True):
            self.logger.warning("Applied fixes to some images")
        else:
            self.logger.info("All images were in the correct format")
        return 0


if __name__ == "__main__":
    ImageLint()()
