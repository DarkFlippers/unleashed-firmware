# Requirements

- Python3
- ImageMagic
- Make

# Compiling

```bash
make all
```

# Asset naming rules

## Images and Animations

`NAME_VARIANT_SIZE`

- `NAME`    - mandatory - Asset name in CamelCase. [A-Za-z0-9], special symbols not allowed
- `VARIANT` - optional  - icon variant: can relate to state or rendering conditions. Examples: active, inactive, inverted.
- `SIZE`    - mandatory - size in px. Example square 10, 20, 24, etc. Example rectangular: 10x8, 19x5, etc.

Image names will be automatically prefixed with `I_`, animation names with `A_`.
Icons and Animations will be gathered into `icon.h` and `icon.c`.

# Important notes

Don't include assets that you are not using, compiler is not going to strip unused assets.

