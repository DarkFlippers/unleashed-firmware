### New Update
* An-Motors and HCS101 keeloq emulation
* FAAC SLH/Spa counter fix, now it uses 20-bits
* Keeloq increased hold duration
* OFW: Update bad_usb_script.c to fix incorrect ALT key 
* OFW: SubGhz: add new keeloq protocols (IronLogic, Comunello, Sommer(fsk476), Normstahl, KEY, EcoStar, Gibidi, Mutancode)
* OFW: Better crash handling
* OFW: ibutton, Infrared, LFRFID GUI fixes
* OFW: Log MFC nonces for use with mfkey32v2 
* OFW: IR: increase raw timings amount
* OFW: Move files from /int to /ext on SD mount
* OFW: Embedded arm-none-eabi toolchain (+ fixed - fbt.cmd runs submodule update anyways ignoring FBT_NO_SYNC)
#### Previous changes
* Update screen after sending signal in SubGhz received signals scene
* Recompiled Universal Remote for ALL buttons (may cause freeze while bruteforcing for 5-10sec, its ok) (see PR #29)
* OFW: Add a FORCE=1(env var) to flash every build
* OFW: nfc: fix exit after emulation
* OFW: Bluetooth Remote Additions
* OFW: Save picopass as picopass or, for 26bit, as lfrfid 
* OFW: added gui-shift command to ducky script 
* OFW: CLI, threads, notifications, archive fixes
* OFW: other fixes
* Added 17 new mf classic keys (Hotels) (PR)
* OFW: Picopass/iClass plugin new UI
* OFW: NFC: On-device tag generator
* OFW: Add GPIO control through RPC
* OFW: Added Javacard Emulated mifare classic 1K compatibility
* Fixed picopass/iclass reader plugin build & included keys into plugin(from OFW PR)
* OFW: SubGhz keypad lock
* OFW: picopass/iclass reader plugin
* OFW: NFC emulation software tunning
* OFW: ToolChain versioning, better Windows support
* OFW: NFC: add Mifare Infineon
* OFW: some FBT fixes
* Merged latest ofw changes - scons build system
* Removed WAV Player - it's not bad as a concept but has a lot of problems
* Some small fixes
* Spectrum Analyzer - show current mode on screen when changing modes
* Spectrum Analyzer - Ultra Narrow mode
* Desktop autolock more time options
