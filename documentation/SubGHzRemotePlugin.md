# Sub-GHz Remote

## Combines up to 5 `.sub` files into one remote - Up, Down, Left, Right and OK on the D-pad

<br>
<br>

### Map files are built and edited on the flipper itself, you do not need a PC
### Open `Sub-GHz Remote`, press Back to leave the file browser, and you are in the main menu

<br>
<br>

## Map files

- A map file is a FlipperFormat `.txt` file in the `subghz_remote` folder on the microSD card
- The folder is created on first launch, and an old `unirf` folder is moved into it
- You can add as many map files as you want, file name doesn't matter
- A map file stores the paths of up to 5 `.sub` files, their labels, and their button codes


<br>
<br>

## Main menu

- `Open Map File` - select a map file and go to the remote screen
- `Edit Map File` - select a map file and open the editor
- `New Map File` - enter a name, then the editor opens with 5 empty slots
- `Set Default` - select a map file to open automatically next time
- `Clear Default` - forget it again, shown only when a default is set

<br>

#### The app starts in the file browser, press Back to get to this menu

<br>
<br>

## Default map file

- `Set Default` writes the path into `subghz_remote/.config`
- On the next launch the app loads that map file and goes straight to the remote screen, Back returns to the main menu
- If the default file is missing or broken, the app falls back to the file browser
- `Clear Default` deletes `.config`

```
Filetype: SubGhzRemote Config
Version: 1
Default: /ext/subghz_remote/example.txt
```


<br>
<br>

## Setting up the `subghz_remote/example.txt` file

```
Filetype: Flipper SubRem Map file
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
UBUTTON: 00
DBUTTON: 00
LBUTTON: 00
RBUTTON: 00
OKBUTTON: 00
```

#### The two header lines are required, a file without them is not loaded

- `UP` / `DOWN` / `LEFT` / `RIGHT` / `OK` - full path of the `.sub` file for that D-pad direction
- `ULABEL` / `DLABEL` / `LLABEL` / `RLABEL` / `OKLABEL` - text shown on the remote screen for that slot
- `UBUTTON` / `DBUTTON` / `LBUTTON` / `RBUTTON` / `OKBUTTON` - custom button code sent with that signal, in HEX

<br>

#### Button codes

| Value | Button |
|---|---|
| `00` | Default - the button stored in the `.sub` file |
| `01` | Up |
| `02` | Down |
| `03` | Left |
| `04` | Right |

- Only used by protocols that support custom buttons, ignored by the others
- The editor writes a `*BUTTON` line only when the value is not `00`
- Lines may be in any order, and a slot you don't use can be left out completely

<br>

## Example

```
Filetype: Flipper SubRem Map file
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


<br>
<br>

## Remote screen

- Hold the flipper vertically, the remote screen is drawn in vertical orientation
- Press a D-pad button to send the signal of that slot, hold it to keep sending
- On release the protocol finishes its own repeats, RAW is sent to the end of the file and starts again if you keep holding
- Back stops a transmission that is running, Back again leaves the screen
- The top bar shows internal or external radio, and an arrow while sending
- The slot that is sending is highlighted
- Under the list you get details of the last pressed button - protocol, frequency and modulation, key, serial, counter, manufacturer. Only 5 of those lines fit, the rest are dropped
- A slot with no label shows the `.sub` file name instead, and `Empty Label` if there is no label and no path

<br>

#### Slot errors on the remote screen

- `[X] Can't open file` - the path in the map file does not point to an existing file
- `[X] Error in .sub file` - the file was opened but cannot be used, see the editor for the reason
- An empty slot shows nothing and does nothing


<br>
<br>

## Editor

- `Up` / `Down` - move between the 5 slots
- `OK` - open the edit menu for the chosen slot
- `Left` or `Back` - leave the editor, you are asked `Changes are not saved` if something was changed
- `Right` - preview and save

<br>

#### Edit menu

- `Edit Label` - text input for the label, up to 24 characters, an empty label is allowed
- `Edit File` - file browser over `subghz` to pick the `.sub` file, the file is checked right away
- `Button` - `Default`, `Up`, `Down`, `Left`, `Right`, the chosen one is shown next to the path as `(Btn. Up)`
- `Clear Slot` - empty the slot

<br>

#### Preview and save

- `Right` opens the remote screen in `Preview` mode, nothing is transmitted there
- `Right` or `OK` saves the map file and shows `Saved!`
- `Left` goes back to the editor

<br>

#### Errors in the editor

The chosen slot shows `ERR:` and one of:

- `File not found`
- `Bad frequency` - the frequency is missing from the file, or not supported by the selected radio module
- `Bad modulation`
- `Unsupported protocol` - the protocol cannot be transmitted, or is not one of static, dynamic, BinRAW and RAW

A slot with nothing in it shows `Button not set`.

Picking a file that fails one of those checks in `Edit File` shows an `ERROR` popup and keeps the slot as it was.


<br>
<br>

## Notes

* ##### File paths in a map file
  - Spaces are fine, so are `-` and `_`
  - The name of the `.sub` file is checked for non-ASCII characters and for `\ < > * | " : ?`, a slot with any of those in the file name is skipped
  - `.sub` files must be `Flipper SubGhz Key File` or `Flipper SubGhz RAW File`, version 1

* ##### Labels
  - The old 16 character rule was only about what fits on the screen, it is not a limit in the file
  - The text input accepts up to 24 characters, and a long label is cut off on the remote screen

* ##### Opening a map file from elsewhere
  - A map file opened from the Archive or the file browser is loaded straight into the remote screen
