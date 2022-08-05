# Requirements

- Python3
- Python3 packages: Pillow & heatshrink2 

# Compiling

```bash
./fbt icons proto dolphin_internal dolphin_blocking dolphin_ext resources
```

# Compiling with Docker-Compose

```bash
docker-compose exec dev ./fbt icons proto dolphin_internal dolphin_blocking dolphin_ext resources
```

# Asset naming rules

## Images and Animations

`NAME_VARIANT_SIZE`

- `NAME`    - mandatory - Asset name in CamelCase. [A-Za-z0-9], special symbols not allowed
- `VARIANT` - optional  - icon variant: can relate to state or rendering conditions. Examples: active, inactive, inverted.
- `SIZE`    - mandatory - size in px. Example square 10, 20, 24, etc. Example rectangular: 10x8, 19x5, etc.

Image names will be automatically prefixed with `I_`, animation names with `A_`.
Icons and Animations will be gathered into `icon.h` and `icon.c`.

## Dolphin and Games assets

Rules are same as for Images and Animations plus assets are grouped by level and level prepends `NAME`.
Good starting point: https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/AssetNaming/

# Important notes

Don't include assets that you are not using, compiler is not going to strip unused assets.

# Structure
- `compiled`            - Output folder made for compiled assets, after building project, in `build` directory.
- `dolphin`             - Dolphin game assets sources. Goes to `compiled` and `resources` folders in `build` directory.
- `icons`               - Icons sources. Goes to `compiled` folder in `build` directory.
- `protobuf`            - Protobuf sources. Goes to `compiled` folder in `build` directory.
- `resources`           - Assets that is going to be provisioned to SD card.
- `slideshow`           - One-time slideshows for desktop
- `unit_tests`          - Some pre-defined signals for testing purposes.
