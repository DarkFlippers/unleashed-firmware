### New changes
* NRF24 sniffer - tweak sniff parameters for more speed and reliability (by @mothball187) (PR #51)
* Fixed text in LF RFID -> Extra Actions
* Updated universal remote assets (by @Amec0e)
* OFW PR: SubGHz decode raw gui (by @qistoph) (PR 1667) / xMasterX: Replaced custom image with default one & Fixed Led don't stop blink after pressing Send from decoder scene
* WAV Player plugin excluded from releases to save space, you can enable it in `applications\meta` for your builds
* OFW PR: SubGhz: add protocol Intertechno_V3 - OFW PR 1622
* OFW PR: SubGhz: add protocol Prastel - OFW PR 1674
* OFW PR: Fix displaying LFRFID protocol names - OFW PR 1682 / xMasterX: Fixed display for N/A manufacturer
* OFW: LF RFID - PAC/Stanley, Paradox, Jablotron, Viking, Pyramid protocols support
* OFW: Picopass write (PR 1658)
* OFW: SubGhz: fix CLI "subghz tx"
* OFW: IR: Fix crash after cancelling Learn New Remote
* OFW: SubGhz: output debug data to external pin
* OFW: Speedup SD card & enlarge your RAM
* OFW: Other small changes

**Note: To avoid issues prefer installing using web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or `.tgz` for iOS mobile app

DFU for update using qFlipper - `flipper-z-f7-full-(version).dfu`

If using DFU update method, download this archive and unpack it to your microSD, replacing all files except files you have edited manually -
`sd-card-(version).zip`

