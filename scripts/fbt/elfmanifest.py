import os
import struct
from dataclasses import dataclass, field

from flipper.assets.icon import file2image

from .appmanifest import FlipperApplication

_MANIFEST_MAGIC = 0x52474448


@dataclass
class ElfManifestBaseHeader:
    manifest_version: int
    api_version: int
    hardware_target_id: int

    manifest_magic: int = 0x52474448

    def as_bytes(self):
        return struct.pack(
            "<IIIh",
            self.manifest_magic,
            self.manifest_version,
            self.api_version,
            self.hardware_target_id,
        )


@dataclass
class ElfManifestV1:
    stack_size: int
    app_version: int
    name: str = ""
    icon: bytes = field(default=b"")

    def as_bytes(self):
        return struct.pack(
            "<hI32s?32s",
            self.stack_size,
            self.app_version,
            bytes(self.name.encode("ascii")),
            bool(self.icon),
            self.icon,
        )


def assemble_manifest_data(
    app_manifest: FlipperApplication,
    hardware_target: int,
    sdk_version,
):
    image_data = b""
    if app_manifest.fap_icon:
        image = file2image(os.path.join(app_manifest._apppath, app_manifest.fap_icon))
        if (image.width, image.height) != (10, 10):
            raise ValueError(
                f"Flipper app icon must be 10x10 pixels, but {image.width}x{image.height} was given"
            )
        if len(image.data) > 32:
            raise ValueError(
                f"Flipper app icon must be 32 bytes or less, but {len(image.data)} bytes were given"
            )
        image_data = image.data

    app_version_as_int = ((app_manifest.fap_version[0] & 0xFFFF) << 16) | (
        app_manifest.fap_version[1] & 0xFFFF
    )

    data = ElfManifestBaseHeader(
        manifest_version=1,
        api_version=sdk_version,
        hardware_target_id=hardware_target,
    ).as_bytes()
    data += ElfManifestV1(
        stack_size=app_manifest.stack_size,
        app_version=app_version_as_int,
        name=app_manifest.name,
        icon=image_data,
    ).as_bytes()

    return data
