### New changes
* If you have copied any apps manually into `apps` folder - remove `apps` folder or that specific apps you copied on your microSD before installing this release to avoid issues due to OFW API version update! If you using regular builds or extra pack builds (e) without your manually added apps, all included apps will be installed automatically, no extra actions needed!
* Settings->LCD and Notifications will be resetted to default due to new Contrast setting from OFW
-----
* Plugins: **New RFID 125KHz and iButton Fuzzers (remake from scratch + new features)** (by @gid9798 | PR #507)
* Plugins: SubGHz Bruteforcer -> Time delay (between signals) setting (hold Up in main screen(says Up to Save)) + allow more repeats (by @gid9798 & @xMasterX)
* Plugins: Update TOTP (Authenticator) [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
* Plugins: Unitemp SCD30 support (PR in unitemp repo by @divinebird / fixed by @xMasterX)
* Plugins: Fix ProtoView issue #503 -> (Broken saved files with custom modulation)
* SubGHz: Remove broken modulation that was causing buffer overrun (fixes issue #506)
* SubGHz: Notifications fixes (by @wosk | PR #464)
* GUI: `Byte input` new feature: editor without keyboard (press Up until you get into new input, then use up/down to input values) (by @gid9798 | PR #509)
* Infrared: Update universal remote assets - add new ACs and TCL TV
* API: Add furi_hal_version_uid_default (+ Fix TOTP) (by @ClaraCrazy | PR #502)
* OFW PR 2756: fix: make dialog_file_browser_set_basic_options initialize all fields (by JarvisCraft)
* OFW: Fix reading Mifare Classic cards with unusual access conditions and fix emulation of unknown keys
* OFW: fbt: stable build dates
* OFW: weather_station: add oregon3 with THGR221
* OFW: Services: simplify api (DOLPHIN_DEED->dolphin_deed - function instead of macros + remake all apps in extra pack and main fw to use new API)
* OFW: Core2, SRAM2: provide safety gap
* OFW: FuriHal: always clock SMPS from HSI
* OFW: ble: refactored bt gatt characteristics setup (+ remake of BT HID Led descriptor in new way to work with this changes)
* OFW: Scripts: WiFi board updater
* OFW: github: re-enabled f18 build
* OFW: added ISO15693 (NfcV) (was already added before, so we just updated it with latest changes)
* OFW: fbt: added Flipper selection when multiple are connected over USB
* OFW: fbt, ufbt: added checks for appid in app manifests
* OFW: Fix core2 permisions
* OFW: SubGhz: add subghz_protocol_registry external API (was already in our API but in different way)
* OFW: Furi: smaller critical enter and critical exit macro
* OFW: Serial_CLI: Fixing serial cli logger error so it sounds more concise
* OFW: Remove unused resources
* OFW: Dolphin: new animation
* OFW: f7: add PB9 to debug pins
* OFW: Settings: add contrast adjustment -> **Settings->LCD and Notifications will be resetted to default values one time after installing**
* OFW: FuriHal: add system setting to device info, bump device info version

#### [🎲 Download latest extra apps pack](https://github.com/xMasterX/all-the-plugins/archive/refs/heads/main.zip)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

## Please support development of the project
* **Boosty** (patreon alternative): https://boosty.to/mmxdev
* cloudtips (only RU payments accepted): https://pay.cloudtips.ru/p/7b3e9d65
* YooMoney (only RU payments accepted): https://yoomoney.ru/fundraise/XA49mgQLPA0.221209
* USDT(TRC20): `TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`
* BCH: `qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`
* ETH/BSC/ERC20-Tokens: `darkflippers.eth` (or `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`)
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`
* XMR (Monero): `41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn`
* TON: `EQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmpGf`

### Thanks to our sponsors:
callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

**Recommended option - Web Updater**

What means `n` or `e` in - `flipper-z-f7-update-(version)(n / e).tgz` ? - `n` means this build comes without our custom animations, only official flipper animations, 
`e` means build has extra apps pack preinstalled

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for mobile app / qFlipper


