### New changes
* FAAC SLH correct seed view
* Add new keys: FAAC SLH/Spa and BFT Mitto/Secure
* Some updates of custom apps, using furi now instead of cmsis(os)
* Some build changes
* OFW: Furi: core refactoring and CMSIS removal

**Note: We changed version names, because our releases not based on official releases, now versions is called
cg1-(commit-sha) - where "cg" = "Code Grabber", 1 = number of build(if ofw commit doesnt change), and (commit sha) - means ofw dev commit on which our version is based**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip`

DFU for update using qFlipper - `flipper-z-f7-full-(version).dfu`

If using DFU update method, download this archive and unpack it to your microSD, replacing all files except files you have edited manually -
`sd-card-(version).zip`