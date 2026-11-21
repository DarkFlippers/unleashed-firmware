# Sub-GHz Signal Settings

## Counter mode, rolling counter and button editor for a saved signal

<br>
<br>

### This is an Unleashed only feature, and it only shows up with Debug turned on

<br>
<br>

## Where it is

- Open a saved signal - `Sub-GHz` -> `Saved`, or a `.sub` file from the Archive
- The menu is `Emulate`, `Rename`, `Delete`, and `Signal Settings`
- With Debug off, `Signal Settings` is not in that menu at all


<br>
<br>

## Turning Debug on

- `Settings` -> `System` -> `Debug` -> `ON` is the normal way
- The quick way - hold <kbd>DOWN</kbd> on the desktop to open the info screen, then long press <kbd>OK</kbd>
- That screen shows firmware version and build date, hardware version, uptime, flipper name, and the Debug state as `[D:ON]` or `[D:OFF]`
- Long <kbd>OK</kbd> toggles it, short <kbd>BACK</kbd> exits
- It is the same setting as `Settings` -> `System` -> `Debug`, and it also reveals the `Internal` tab in the Archive


<br>
<br>

## The three items

| Item | What it does |
|---|---|
| `Counter Mode` | `System`, then `Mode 1` and up, as many as the protocol has |
| `Edit Counter` | Enter the rolling counter of this signal in HEX |
| `Edit Button` | Enter the button code of this signal in HEX |

<br>

#### An item you cannot use is locked and shows

```
Not available
for this
protocol !
```


<br>
<br>

## Counter Mode

- Changing it writes a `CounterMode:` line into the `.sub` file right away and reloads the file, nothing is transmitted
- `System` is mode 0, the normal remote behaviour
- Only these protocols have modes, everything else is locked:

| Protocol | Choices |
|---|---|
| Nice FloR-S | `System`, `Mode 1`, `Mode 2` |
| CAME Atomo | `System`, `Mode 1` - `Mode 3` |
| Alutech AT-4N | `System`, `Mode 1`, `Mode 2` |
| KeeLoq | `System`, `Mode 1` - `Mode 10` |
| Phoenix_V2 | `System`, `Mode 1`, `Mode 2` |

<br>

#### What each mode does to the counter is in [Experimental Counter Mode](/documentation/SubGHzCounterMode.md), along with the warnings - read that before using anything other than `System`


<br>
<br>

## Edit Counter

- Shows the current counter, press <kbd>OK</kbd> to enter a new one
- The header tells you the length, for example `Enter 16-bits counter in HEX`
- Counters longer than 16 bits get a 4 byte field (8 hex digits), everything else 2 bytes (4 hex digits)
- Locked when the protocol has no counter, or when the counter cannot be read - KeeLoq with manufacturer `AN-Motors` or `Unknown` for example
- The value you enter is written as it is, it is not stepped by the rolling counter multiplier on that save
- With a `Counter Mode` other than `System` the mode decides the counter itself, so editing it there has little point


<br>
<br>

## Edit Button

- Shows the current button code, press <kbd>OK</kbd> to enter a new one
- One byte, the header tells you the length, for example `Enter 4-bits button in HEX`
- Which codes your receiver reacts to depends on the system, the original remote's codes are the ones to try
- Locked when the protocol has no button


<br>
<br>

## Be aware - confirming an edit transmits

> [!WARNING]
> `Edit Counter` and `Edit Button` save the new value by running the transmit path. The firmware starts a transmission and stops it immediately, and the file is rewritten when that transmission stops.

- So a short burst goes out on the frequency of the signal, with the new value in it, as if you pressed send once
- If that frequency is receive only for your region, the transmission is refused and the edit is not saved - see [How to extend Sub-GHz frequency range](/documentation/DangerousSettings.md)
- `Edit Button` sets the rolling counter multiplier to 0 for that save and restores it after, so the counter does not move
- Only dynamic (rolling code) protocols are written back this way

<br>
<br>

*Made for Unleashed FW, please mention source when copying*
