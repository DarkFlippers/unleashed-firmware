# Sub-GHz Remote


# UPDATE!!!!!!
## Now you can create and edit map files directly on flipper, go into Applications->Sub-GHz->Remote Maker

<br>
<br>
<br>


### The SubGHz Remote Tool *requires* the creation of custom user map with `.txt` extension in the `subghz_remote` folder on the sdcard. 

#### If these files are not exist or not configured properly, **you will receive an error each time you try to select wrong file in the UniRF Tool**.

## You can add as many `.txt` map files as you want, file name doesn't matter!


## Incorrect or unconfigured file error

If the `.txt` file has not been properly configured, the following error will be thrown when trying to run the UniRF Remix app:

```
Config is incorrect.

Please configure map

Press Back to Exit
```



## Setting up the `subghz_remote/example.txt` file:

```
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
  - Press Back button to exit app.

* ##### SubGHz Remote Map
  - File path should not have any spaces or special characters (- and _ excluded).
  - Labels are limited to 16 characters.
    - Why? This is to prevent overlapping elements on screen.
    - For example: If you set your label or file to ```WWWWWWWWWWWWWWW``` you'll be over the screen limits.
