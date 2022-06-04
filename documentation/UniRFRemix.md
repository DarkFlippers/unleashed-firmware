# UniRF Remix

### The UniRF Tool *requires* the following manual changes be made to the `universal_rf_map` file in the `subghz/assets` folder located on the sdcard. 

#### If these custom changes are not made, **you will receive an error each time you try to run the UniRF Tool**.

## Incorrect or unconfigured file error

If the `universal_rf_map` file has not been properly configured, the following error will be thrown when trying to run the UniRF Remix app:

```
Config is incorrect.

Please configure
universal_rf_map

Hold Back to Exit
```
## Setting up the `subghz/assets/universal_rf_map` file:

```
Filetype: Flipper SubGhz RAW File
Version: 1
UP: /ext/subghz/Up.sub
DOWN: /ext/subghz/Down.sub
LEFT: /ext/subghz/Left.sub
RIGHT: /ext/subghz/Right.sub
OK: /ext/subghz/Ok.sub
ULABEL: Up Label
DLABEL: Down Label
LLABEL: Left Label
RLABEL: Right Label
OKLABEL: Ok Label
```

The UP/DOWN/LEFT/RIGHT/OK file locations must be set to the specific file you want mapped to that directional pad direction.

The ULABEL/DLABEL/LLABEL/RLABEL/OKLABEL variables should be set to the text to be displayed for each of the files set earlier.

## Example:

```
Filetype: Flipper SubGhz RAW File
Version: 1
UP: /ext/subghz/Fan1.sub
DOWN: /ext/subghz/Fan2.sub
LEFT: /ext/subghz/Door.sub
RIGHT: /ext/subghz/Garage3.sub
OK: /ext/subghz/Garage3l.sub
ULABEL: Fan ON
DLABEL: Fan OFF
LLABEL: Doorbell
RLABEL: Garage OPEN
OKLABEL: Garage CLOSE
```

## Notes
* ##### App Usage
  - Press a button to send the assigned capture file.
  - Press Back button to set how many repeats the app should send. Capped at 5 repeats.
  - Hold Back button to exit app.
  - Only RAW SubGhz captures are supported currently.
  - No skip function.

* ##### Universal RF Map
  - Backwards compatible with [jimilinuxguy Universal RF Remote](https://github.com/jimilinuxguy/flipperzero-universal-rf-remote) map file. You should be able to use the map file as is with both versions.
  - Recommend that you update the map file (if you using "jimilinuxguy's Universal RF" file) to the version included in this repo.
  - File path should not have any spaces or special characters (- and _ excluded).
  - Labels are limited to 12 characters.
    - Why? This is to prevent overlapping elements on screen.
    - For example: If you set your label or file to ```WWWWWWWWWWWWWWW``` you'll be over the screen limits.
