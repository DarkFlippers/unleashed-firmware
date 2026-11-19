## v1.9

- New *Decode* action: decodes the A..B selection with the firmware's Sub-GHz decoders, groups identical frames and saves the chosen one as a key file.
- Loading a rolling-code signal (KeeLoq, Nice Flor-S, ...) no longer bumps its counter on custom firmware.
- A decoded frame is saved under its protocol and manufacturer name by default (e.g. KeeLoq_Verex, then KeeLoq_Verex_1, KeeLoq_Verex_2, ...).
- Saving from a file like merge_edit1 suggests merge_edit2 instead of merge_edit1_edit1.
- Saving under an existing name asks to overwrite it instead of refusing; the open file is never overwritten.
- *Repeat each* shows its maximum (64).
- The app can be launched with a .sub file path as argument and then opens it straight in the editor (used by Sub-GHz → More → Edit in Lechnio's Momentum fork).

Note: the app now takes about 2 KB more RAM once loaded, so when free RAM (not the 24000-sample cap) limits loading, files can be about 1000 samples shorter.

## v1.8

- Fixed Cut collapsing every gap longer than 32 ms in the whole capture.
- Loading a non-RAW signal now offers a button picker for protocols that support custom buttons.

## v1.7

- Add option to configure merge gap in range 1ms-1s.
- Add option to multiplicate signals when loading single or merging multiple files up to 64 times.
Multiplied signals (other then RAWs) will be seperated by synchronization gap
that comes from internal decoder. The gap is by decoders to synchronize signals without preamble.
- Removed limit of 16 merged files. There is no limit now (beside RAM capacity).
- Rewritten gap clamping mechanizm. Now long gaps are not clamped to 32ms.
The original length has been preserved instead.
- Optimized loading via transmit deserialzier.

## v1.6

- Fixed MPU fault stack overflow error on some flipper firmwares.
The bottle neck is internal deserializer called when loading recognized signals.
The change bumps the application stack from 2KB to 3KB as it seems sufficient value.

Note:
If you ever encounter this issue in the future please let me know on GitHub.
I don't want to raise this value too high if it's not needed because raising
the stack reduces the already very limited heap resources.

## v1.5

- Added merge feature for .sub files
- Added jitter normalizer option
- Added support for loading as RAW any other protocol supported by the firmware.
This was achieved by using the device's internal decoder.

## v1.4

- Added cut feature
- Added loading and saving screen views
- Added marks indicating there is more signal data on that side
- Reorganized overlapped views
- Extended max visible basename length to fit default RAW filename with its full date
- Optimized loading longer .sub files and implement limiter
- View will now automatically switch when swiched to invisible marker

## v1.3

- Apply proper icons for listed .sub files

## v1.2

- Made A/B markers recompute its position correctly in the waveform mode.
When the markers were aligned to a signal edge in the waveform mode,
its possition relative to that edge changed while zooming in/out.

## v1.1

- Adjusted namings
- Highlighted the selected A..B range with a dotted line along the top of the wave area
- Auto-number saved trims to avoid overwriting, show saved filename

## v1.0

- Allow editing RAW Sub-GHz files