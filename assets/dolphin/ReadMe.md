# Dolphin assets

Dolphin assets are split into 3 parts:

- blocking - Essential animations that are used for blocking system notifications. They are packed to `assets_dolphin_blocking.[h,c]`.
- internal  - Internal animations that are used for idle dolphin animation. Converted to `assets_dolphin_internal.[h,c]`.
- external  - External animations that are used for idle dolphin animation. Packed to resource folder and placed on SD card.

# Files

- `manifest.txt` - contains animations enumeration that is used for random animation selection. Starting point for Dolphin.
- `meta.txt`     - contains data that describes how animation is drawn.
- `frame_X.png`  - animation frame.

## File manifest.txt

Flipper Format File with ordered keys.

Header:

```
Filetype: Flipper Animation Manifest
Version: 1
```

- `Name` - name of animation. Must be exact animation directory name.
- `Min butthurt`, `Max butthurt` - range of dolphin's butthurt for this animation.
- `Min level`, `Max level` - range of dolphin's level for this animation. If 0, this animation doesn't participate in random idle animation selection and can only be selected by exact name.
- `Weight` - chance of this animation to be choosen at random animation selection.

Some animations can be excluded from participation in random animation selection, such as `L1_NoSd_128x49`.

## File meta.txt

Flipper Format File with ordered keys.

Header:

```
Filetype: Flipper Animation
Version: 1
```

- `Width` - animation width in px (<= 128)
- `Height` - animation height in px (<= 64)
- `Passive frames` - number of bitmap frames for passive animation state
- `Active frames` - number of bitmap frames for active animation state (can be 0)
- `Frames order` - order of bitmap frames where first N frames are passive and following M are active. Each X number in order refers to bitmap frame, with name frame\_X.bm. This file must exist. Any X number can be repeated to refer same frame in animation.
- `Active cycles` - cycles to repeat of N active frames for full active period. E.g. if frames for active cycles are 6 and 7, and active cycles is 3, so full active period plays 6 7 6 7 6 7. Full period of passive + active period are called *total period*.
- `Frame rate` - number of frames to play for 1 second.
- `Duration` - total amount of seconds to play 1 animation.
- `Active cooldown` - amount of seconds (after passive mode) to pass before entering next active mode.

- `Bubble slots` - amount of bubble sequences.
- Any bubble sequence plays whole sequence during active mode. There can be many bubble sequences and bubbles inside it. Bubbles in 1 bubble sequence have to reside in 1 slot. Bubbles order in 1 bubble sequence is determined by occurance in file. As soon as frame index goes out of EndFrame index of bubble - next animation bubble is choosen. There can also be free of bubbles frames between 2 bubbles.

- `Slot` - number to unite bubbles for same sequence.
- `X`, `Y` - are coordinates of left top corner of bubble.
- `Text` - text in bubble. New line is `\n`
- `AlignH` - horizontal place of bubble corner (Left, Center, Right)
- `AlignV` - vertical place of bubble corner (Top, Center, Bottom)
- `StartFrame`, `EndFrame` - frame index range inside whole period to show bubble.

### Understanding of frame indexes

For example we have

```
Passive frames: 6
Active frames: 2
Frames order: 0 1 2 3 4 5 6 7
Active cycles: 4
```

Then we have indexes

```
                        passive(6)            active (2 * 4)
Real frames order:   0  1  2  3  4  5     6  7  6  7  6  7  6  7
Frames indexes:      0  1  2  3  4  5     6  7  8  9  10 11 12 13
```
