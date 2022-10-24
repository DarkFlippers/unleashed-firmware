### New changes
* API version changed to 6.1 due to OFW updates, Extra apps pack update is already done! Download it from link below
* Halloween theme + animation :) (by @Svaarich) (will be disabled on first days of November)
* Plugins: Add new plugin - HC-SR04 Distance sensor [(original by Sanqui)](https://github.com/Sanqui/flipperzero-firmware/tree/hc_sr04) - Ported and modified by @xMasterX - How to connect -> (5V -> VCC) / (GND -> GND) / (13|TX -> Trig) / (14|RX -> Echo)
* Plugins: Snake -> Allow food to spawn anywhere (by @TQMatvey | PR #130)
* Plugins: Use clear power in temp sensor plugin
* Plugins: WS -> add protocol Acurite 592TXR (OFW PR 1916 by Skorpionm)
* Plugins: WS -> fix oregon2 flags, and protocol type
* Fixed wrong fbt arguments in tasks.json in vscode example config -> plugin_dist -> fap_dist
* OFW PR: TikTok: reset cursor after enter and reconnect (OFW PR 1921 by gornekich)
* OFW: Furi: smaller crash routine (breaking change, API version increased because of that)

#### [🎲 Download extra apps pack](https://download-directory.github.io/?url=https://github.com/UberGuidoZ/Flipper/tree/main/Applications/Unleashed%20(and%20RogueMaster))

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper 1.2.1 (allows .tgz installation) (official link)](https://update.flipperzero.one/builds/qFlipper/1.2.1/)

## Please support development of the project
* Boosty: https://boosty.to/mmxdev
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for iOS mobile app / qFlipper

Update using qFlipper (1.2.0+) is now possible with `.tgz` update package! Also you can use Web Updater or self-update package.

