Tama P1 Emulator for Flipper Zero
=======================================

This is a tama P1 Emulator app for Flipper Zero, based on [TamaLIB](https://github.com/jcrona/tamalib/).

How to play
-----------
Create a `tama_p1` folder in your microSD card, and put the ROM as `rom.bin`.
Left button is A, OK is B, and right button is C. Hold the back button to exit.
There is currently no saving, so your progress will be reset when you exit the
app.

Building
--------
Run the following to compile icons:
```
scripts/assets.py icons applications/tama_p1/icons applications/tama_p1/compiled
```

Note: you may also need to add `-Wno-unused-parameter` to `CCFLAGS` in
`site_cons/cc.scons` to suppress unused parameter errors in TamaLIB.

Implemented
-----------
- Basic emulation
- Input
- Sound

To-do
-----
- Saving/loading
  - Multiple slots?
- In-game reset
- Test mode?
- Volume adjustment
