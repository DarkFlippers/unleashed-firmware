## Main changes
- Current API: 88.2
* SubGHz: **Fix endless TX causing RAW files to be transmitted and crash the system** (via RPC / Mobile App) (Fixes issue #1008)
* SubGHz: **Fix crash when exiting CLI `subghz chat` with an external CC1101** - pressing Ctrl+C dereferenced the just-freed external CC1101 radio plugin during chat worker shutdown; internal CC1101 was unaffected (by @mishamyte | PR #1036 | Fixes #829)
* SubGHz: **Fix Frequency Analyzer leaving the radio in a degraded state** - the worker parked the internal CC1101 with the antenna isolated and its near-field AGC profile still loaded; apps driving the radio HAL directly (without a full re-init) inherited a "sticky" sensitivity loss - the radio is now reset to its boot state on exit (by @mishamyte | PR #1045 | Fixes #1044)
* SubGHz: **Add Telcoma/Cardin EDGE protocol** (32bit, Static) (by @half2me | PR #1001)
* SubGHz: **Fix Pecinin keeloq manufacturer** (by @lifegame1lu111)
* SubGHz: **RAW files corruption fix** (by @Lechnio)
* LFRFID: **Support of Hitag Micro chips** (8265/8210/H5.5) (by @mishamyte | PR #1002)
* LFRFID: **Wipe T5577** (reset to blank, with read-back verification) (by @mishamyte | PR #1003)
* LFRFID: **Read T5577 tags holding multiple EM4100 IDs again** - a T5577 written with several EM4100 IDs (e.g. via Multiwriter) hung on Read since the Electra protocol was added; also resets a stale PAC/Stanley decoder buffer (by @mishamyte | PR #1025 | Fixes #1024)
* NFC: **Native MIFARE Plus support in SL3** - MIFARE Plus is now a first-class protocol instead of detection-only: read (AES auth + encrypted/plaintext blocks, admin keys & config, originality signature), automatic dictionary attack with a per-UID key cache (instant re-reads of saved cards), full SL3 emulation with shadow-writeback (a reader can authenticate, read & write the recovered card), write/update-to-card, GetVersion + ATS-based S/X/SE/EV1/EV2 detection, "Add Manually" for 18 Plus variants, and MIFARE-Classic-style dump & keys screens; SL0/SL1/SL2 stay untouched (by @mishamyte | PR #1032 | Closes #1031)
* NFC: **Fix Type 4 Tag emulation crashing on malformed NDEF write APDUs** - a reader could reboot the Flipper with a 4-to-7 byte `UPDATE BINARY`: an empty data field, a truncated NLEN write that underflowed the length into a ~4 GB allocation, or an unclamped 64 KB NLEN; a partial NLEN write also no longer zeroes the byte it didn't cover (by @mishamyte | PR #1051 | Fixes #1050)
* NFC: **Fix Type 4 Tag emulation corrupting the NDEF message on writes starting mid-NLEN** - a reader updating from file offset 1 had its payload shifted one byte and picked up a stale byte from the previous frame (by @Endika | PR #1049)
* NFC: Show MIFARE Ultralight/NTAG PWD & PACK in full info view / on read screen too (by @mishamyte | PR #1010 #1011)
* NFC: **Add Bambu Lab filament spool parser** (type, color, code, temps, spool specs) (ported from [uzyn/flipper-bambu](https://github.com/uzyn/flipper-bambu), GPL-3.0)
* NFC: **Align MIFARE type detection with NXP AN10833** - Classic/Ultralight/NTAG/Plus sizing & security level; fixes Mifare Mini clone mis-detection and Ultralight AES read hang (by @mishamyte | PR #1014)
* NFC: **Read MIFARE Plus 2K in SL1 as full 32 sectors / 64 keys** - a Plus 2K in SL1 is byte-identical to a Classic 1K in SAK/ATQA and was mis-sized as 1K; it is now told apart by its ISO14443-4 ATS (Plus S/X signature), while Plus SE, SmartMX, magic "Perfect CUID" and plain 1K stay 1K (by @mishamyte | PR #1016)
* NFC: **Fix MIFARE Plus 2K SL1 transit parsers** - Troika, Plantain, SevPPK, SZPPK and Two Cities stopped parsing once Plus 2K SL1 cards began reporting as the new `2K` type (PR #1016): the parsers only knew `1K`/`4K` and required a full-card read. They now treat 2K as the 1K these cards present and accept a partial read (by @mishamyte | PR #1038 | Fixes #1037)
* NFC: **Fix transit parsers skipping key recovery on a partial read** - Troika, Plantain, SevPPK, SZPPK and Two Cities reported a read as successful on any partial read (needed for Plus 2K SL1), so a card whose data sector wasn't actually read skipped the dictionary/nested attack instead of recovering its keys; a partial read is now accepted only when the sector the parser needs is read (by @mishamyte | PR #1043 | Fixes #1042)
* NFC: **Show a loading screen while a large CUID dictionary loads on Read** - animated spinner + "CUID dictionary is loading" label instead of a blank/frozen-looking screen while a per-UID (MFKey-recovered) dictionary is scanned (by @mishamyte | PR #1022)
* NFC: **Fix memory leaks & double-frees in the NFC app** - heap-corrupting double-frees in the Plantain and SZPPK/SEVPPK/SK transit parsers (crash on two-trip tickets), leaks in the Saflok parser, the app API resolver (per launch) and the CUID-dictionary error path, plus a ~15x RAM over-allocation of the MIFARE DESFire file-data array (by @mishamyte | PR #1030 | Fixes #1029)
* OFW: NFC: **Fix iso15693 emulation** dropping Read Multiple Blocks for tags >64 blocks
* OFW PR 4383: NFC FeliCa: Add Standard card emulation support (by @nikachu2012)
* RPC: **Add Network and GPS RPC services** (by @apfxtech (Network based on @noproto code and idea) | PR #1013)
* Apps: **NFC Magic** - Gen2 CUID/static-nonce detection, Gen1 4b/7b UID, length-aware wipe & write guard (by @mishamyte)
* Apps: Build tag (**27jul2026**) - **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
* OFW: bit_lib: don't read past the buffer when the bits fit one byte
* OFW: js_event_loop: reject a non-positive timer interval
* OFW: LFRFID: fix the Pyramid trailing Wiegand parity bit
* OFW: LFRFID: NUL-terminate the PAC/Stanley card id before parsing it
* OFW: NFC FeliCa: check the poller error before reading the system-code response
* OFW: Infrared: initialize timings_cnt on decoder alloc and fix its bounds check
* OFW: Allow for checking if an id is in use before adding to view dispatcher 
* OFW: CCID: move USB layer from firmware HAL into ccid_test app
* OFW: Storage Python script: add retry on file copy to Flipper
* Apps: Update FindMy app
* Fix BLE sync, fix possible delay related issues
* Disabled debug and trace logs in the FW binary (apps .fap's are not affected) to free up some flash space for new features
* NFC: Fix typo in SLIX poller (by @WillyJL)
* NFC: Internal MIFARE Plus cleanup - data-drive the "Add Manually" generator variants and unify the admin-key address mapping into one source of truth; small internal-flash saving, no functional change (by @mishamyte | PR #1035)
* NFC: Fix wrong parent protocol in the Type 4 Tag listener assert - copy-pasted from the SLIX listener, so debug builds crashed as soon as Type 4 Tag emulation received a frame; release builds were unaffected (by @mishamyte | PR #1051)
<br><br>

----

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Unleashed FW Web Installer](https://web.unleashedflip.com)

## Please support development of the project

| Service                                                                                                                                                                                        | Remark                    | QR Code                                                                                                                                                                                                                             | Link/Wallet                                                                                       |
|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------|
| <img src="https://cdn.simpleicons.org/patreon/dark/white" alt="Patreon" width="14"/> **Patreon**                                                                                               |                           | <div align="center"><a href="https://github.com/user-attachments/assets/a88a90a5-28c3-40b4-864a-0c0b79494a42"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [patreon.com/mmxdev](https://patreon.com/mmxdev)                                                  |
| <img src="https://cdn.simpleicons.org/boosty" alt="Boosty" width="14"/> **Boosty**                                                                                                             | patreon alternative       | <div align="center"><a href="https://github.com/user-attachments/assets/893c0760-f738-42c1-acaa-916019a7bdf8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [boosty.to/mmxdev](https://boosty.to/mmxdev)                                                      |
| <img src="https://gist.githubusercontent.com/m-xim/255a3ef36c886dec144a58864608084c/raw/71da807b4abbd1582e511c9ea30fad27f78d642a/cloudtips_icon.svg" alt="Cloudtips" width="14"/> CloudTips    | only RU payments accepted | <div align="center"><a href="https://github.com/user-attachments/assets/5de31d6a-ef24-4d30-bd8e-c06af815332a"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [pay.cloudtips.ru/p/7b3e9d65](https://pay.cloudtips.ru/p/7b3e9d65)                                |
| <img src="https://raw.githubusercontent.com/gist/PonomareVlad/55c8708f11702b4df629ae61129a9895/raw/1657350724dab66f2ad68ea034c480a2df2a1dfd/YooMoney.svg" alt="YooMoney" width="14"/> YooMoney | only RU payments accepted | <div align="center"><a href="https://github.com/user-attachments/assets/33454f79-074b-4349-b453-f94fdadc3c68"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [yoomoney.ru/fundraise/XA49mgQLPA0.221209](https://yoomoney.ru/fundraise/XA49mgQLPA0.221209)      |
| <img src="https://cdn.simpleicons.org/tether" alt="USDT" width="14"/> USDT                                                                                                                     | TRC20                     | <div align="center"><a href="https://github.com/user-attachments/assets/0500498d-18ed-412d-a1a4-8a66d0b6f057"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`                                                              |
| <img src="https://cdn.simpleicons.org/ethereum" alt="ETH" width="14"/> ETH                                                                                                                     | BSC/ERC20-Tokens          | <div align="center"><a href="https://github.com/user-attachments/assets/0f323e98-c524-4f41-abb2-f4f1cec83ab6"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`                                                      |
| <img src="https://cdn.simpleicons.org/bitcoin" alt="BTC" width="14"/> BTC                                                                                                                      |                           | <div align="center"><a href="https://github.com/user-attachments/assets/5a904d45-947e-4b92-9f0f-7fbaaa7b37f8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`                                                      |
| <img src="https://cdn.simpleicons.org/solana" alt="SOL" width="13"/> SOL                                                                                                                       | Solana/Tokens             | <div align="center"><a href="https://github.com/user-attachments/assets/ab33c5e0-dd59-497b-9c91-ceb89c36b34d"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `DSgwouAEgu8iP5yr7EHHDqMNYWZxAqXWsTEeqCAXGLj8`                                                    |
| <img src="https://cdn.simpleicons.org/dogecoin" alt="DOGE" width="14"/> DOGE                                                                                                                   |                           | <div align="center"><a href="https://github.com/user-attachments/assets/2937edd0-5c85-4465-a444-14d4edb481c0"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`                                                              |
| <img src="https://cdn.simpleicons.org/litecoin" alt="LTC" width="14"/> LTC                                                                                                                     |                           | <div align="center"><a href="https://github.com/user-attachments/assets/441985fe-f028-4400-83c1-c215760c1e74"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`                                                     |
| <img src="https://bitcoincash.org/img/green/bitcoin-cash-circle.svg" alt="BCH" width="14"/> BCH                                                                                                |                           | <div align="center"><a href="https://github.com/user-attachments/assets/7f365976-19a3-4777-b17e-4bfba5f69eff"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`                                                      |
| <img src="https://cdn.simpleicons.org/monero" alt="XMR" width="14"/> XMR                                                                                                                       | Monero                    | <div align="center"><a href="https://github.com/user-attachments/assets/96186c06-61e7-4b4d-b716-6eaf1779bfd8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn` |
| <img src="https://cdn.simpleicons.org/ton" alt="TON" width="14"/> TON                                                                                                                          |                           | <div align="center"><a href="https://github.com/user-attachments/assets/92a57e57-7462-42b7-a342-6f22c6e600c1"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `UQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmsxa`                                                |


#### Thanks to our sponsors who supported project in the past and special thanks to sponsors who supports us on regular basis:
@mishamyte, ClaraCrazy, Pathfinder [Count Zero cDc], callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!


## **Recommended update option - Web Updater**

### What `e`, ` `, `c` means? What I need to download if I don't want to use Web updater?
What build I should download and what this name means - `flipper-z-f7-update-(version)(e / c).tgz` ? <br>
`flipper-z` = for Flipper Zero device<br>
`f7` = Hardware version - same for all flipper zero devices<br>
`update` = Update package, contains updater, all assets (plugins, IR libs, etc.), and firmware itself<br>
`(version)` = Firmware version<br>
| Designation | [Base Apps](https://github.com/xMasterX/all-the-plugins#default-pack) | [Extra Apps](https://github.com/xMasterX/all-the-plugins#extra-pack) |
|-----|:---:|:---:|
| ` ` | ✅ |  |
| `c` |  |  |
| `e` | ✅ | ✅ |

**To enable RGB Backlight support go into LCD & Notifications settings**

⚠️RGB backlight [hardware mod](https://github.com/quen0n/flipperzero-firmware-rgb#readme), works only on modded flippers! do not enable on non modded device!


Firmware Self-update package (update from microSD) - `flipper-z-f7-update-(version).tgz` for mobile app / qFlipper / web<br>
Archive of `scripts` folder (contains scripts for FW/plugins development) - `flipper-z-any-scripts-(version).tgz`<br>
SDK files for plugins development and uFBT - `flipper-z-f7-sdk-(version).zip`



