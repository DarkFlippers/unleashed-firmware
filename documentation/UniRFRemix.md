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


