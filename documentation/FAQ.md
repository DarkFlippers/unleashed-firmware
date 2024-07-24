# FAQ

## I bought Flipper Zero and I don't know what I can do with it, pls help

- Start with reading [official main page](https://flipperzero.one/)
- Then check out official docs where you can find answers to [most questions](https://docs.flipper.net/)

## How do I install Unleashed firmware?

See [this](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

## What version I should install? What do letters `e`, `r`, `c`... mean?

Follow this link for [details](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/CHANGELOG.md#recommended-update-option---web-updater).

## INSTALLED UNLEASHED AND NOW BACKLIGHT DOESNT WORK?

You’ve installed a version made for custom RGB modded flippers. The version ending in `r` is specifically for `RGB` modded flippers. <br />

Please, do not use that version if your flipper isn’t modded!
  
## What apps (plugins) are included with Unleashed FW?

See default pack and extra pack (for `e` build) list [here](https://github.com/xMasterX/all-the-plugins/tree/dev).

## Where I can find differences between original (official) firmware and Unleashed firmware?

[Right here](https://github.com/DarkFlippers/unleashed-firmware#whats-changed)

## How to use SubGHz Remote app?

1. Open app, press `Back` button, select `New map file`
2. Configure signal files and their names for every button (also you can add only one signal and make other buttons empty - just don't select any files for them in config)
3. Save new map file
4. Open map file and select your previously created file
5. Use buttons to send subghz signal files that you selected in map config at step 2

## How to build (compile) firmware?

Follow this [link](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToBuild.md#how-to-build-by-yourself).

## I installed Unleashed firmware and now my mobile app doesn't connect to flipper ( OR I changed flipper device name and my mobile app now doesn't connect to flipper )

  1. Click Forget flipper in mobile app
  2. Open your `phone settings - bluetooth`, find flipper - if it present here - open its options and click forget device
  3. On flipper itself open `Settings -> Bluetooth -> Forget all devices` and confirm
  4. Make sure your flipper has bluetooth `ON` and open Mobile app and pair it to flipper
  5. Done

## My desktop (pin, favourites, etc..) (or other) settings was reset to default after update, what to do?

Just configure that settings again, all is fine, and make sure you seen changelogs for the releases that came out after your previous version, when settings struct is changed, settings file are reset after update, this happens only when struct changes is required, so don't assume that settings will be reset in every release, this will happen only in specific ones

## Why is flipper not connecting to Chrome?

The most common cause of the flipper not connecting to google chrome is having qFlipper open while trying to connect your flipper. <br />

Or having second flipper lab page open at same time.<br />

You must close qFlipper (or other flipper lab web pages) before attempting to connect your flipper to Chrome.
    
## Flipper doesn't work! How to restore firmware???

Follow this [guide](https://docs.flipper.net/basics/firmware-update/firmware-recovery)

## Useful links and files

Flipper Awesome - place where you can find almost all links that you might need:<br />
* [Awesome-FlipperZero](https://github.com/djsime1/awesome-flipperzero)
* Dict files for iButton Fuzzer and RFID Fuzzer:<br />
  * https://t.me/flipperzero_unofficial_ru/37058 <br />
  * https://t.me/flipperzero_unofficial_ru/37072
* UL Releases in [Telegram](https://t.me/unleashed_fw)
* UL Dev Builds in [Telegram](https://t.me/kotnehleb)
* Our [Discord](https://discord.unleashedflip.com)

## How to change flipper name?

All is simple:
1. Open `Settings -> Desktop -> Change Flipper Name`
2. Enter new name and click `Save`
3. Exit from settings - Flipper will automatically reboot
4. Done, you have custom name which will stay until you reset it to default or replace with new one

## How to reset name to default?

1. Open `Settings -> Desktop -> Change Flipper Name`
2. Do not enter anything, just click Save
3. Exit from settings - Flipper will automatically reboot
4. Done, name is reset to original one.  

## How do I copy files from GitHub to my Flipper Zero?

Follow this detailed [guide](https://github.com/wrenchathome/flipperfiles/blob/main/_Guides/How2Flipper.pdf).

## Where can I find “This file” or “That file” for my flipper?
    
These 2 repos will cover most (99.9%) of your needs:<br />
* https://github.com/UberGuidoZ/Flipper/tree/main
* https://github.com/UberGuidoZ/Flipper-IRDB/tree/main
 
## How can I support Unleashed firmware project?

Please follow this [link](https://github.com/DarkFlippers/unleashed-firmware#please-support-development-of-the-project).

## What are the dev builds? Where I can get latest build for dev branch?

This is an automatic assembly of the latest commits from the repository that have not yet been released, the previous build is deleted when a new one is uploaded and old remains only as file in the telegram channel

> [!CAUTION]
>
> Be aware that this is not release ready builds!
>
> They may have bugs and issues,
> if you are using dev build and found issue,
> report it! In [GitHub issues](https://github.com/DarkFlippers/unleashed-firmware/issues)

Dev builds is available in Discord, Win channel - `unleashed-development` <br />
Builds also can be found [here](https://t.me/kotnehleb).<br />
And [here](https://dev.unleashedflip.com/)<br />

## What is the update server?

We have our own update server https://up.unleashedflip.com/directory.json  <br /><br />
It is identical to the official one, it is impossible to change it in applications without rebuilding the application, it is hardcoded there <br /><br />
If you want to use it, you need to patch or build your own build of the application you are interested in <br />

Also you can use it with uFBT to build apps for UL SDK, uFBT will accept that link as one of args<br />

The server will remain active and will be automatically updated

## External Radio: How to connect CC1101 module

[Guide](https://github.com/quen0n/flipperzero-ext-cc1101)

## How to add extra Sub-GHz frequencies

[Guide](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzSettings.md)

## How to use Flipper as new remote (Nice FlorS, BFT Mitto, Somfy Telis, Aprimatic, AN-Motors, etc..)

[Guide](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)

## How Can I Unlock / Remove SubGHz restriction?

> [!TIP]
>
> If you are using Unleashed firmware - **all region locks are removed by default**!

Also, there is a way to go outside of frequencies stated in `CC1101 datasheet`, but transmission on those frequencies may cause chip damage, make sure you know what you are doing! 

Do not edit this settings to bypass region lock since there is no region locks in unleashed, all chip supported frequencies will work without any extra steps.<br /><br />

But, if you know that you need to bypass subghz chip safety restriction you can unlock the safety restriction which will allow you to go outside the chips supported frequency. <br /><br />
This covers how to do it and information regarding the risks of damage to the flipper by doing so. 

Please read [this](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/DangerousSettings.md) before.

## Can I clone a car key fob for my own car to use flipper as a key?

No, and trying to do so with Read RAW will lead to key desync or unpair with blacklist which means re-pair is very hard and requires service tools

## Will Unleashed FW support car keyfobs decoding, cloning, emulating?

No, never

## Where can I find jamming files?

Nowhere, this is illegal in almost every country in the world

## I saw something on TikTok and want to ask how to do it, I just wanna be like real hacker

And you might be banned for that in our communities, since 99% of that content is fake, or showing illegal actions, and we don't like TikTok related questions.

## I was banned in Unleashed Discord / Telegram / etc.. How to remove ban? I created GitHub issue and it was removed too!

Not possible, rules is rules, read them before sending messages in our communities

## How to clean .DS_Store and other dot files left from macOS

`sudo dot_clean -mn /Volumes/Flipper\ SD` -> `Flipper\ SD` may be named differently for you, replace with your microSD card name

## How to sort files on flipper microSD on macOS / Linux?

Will make sorting faster, and will work for OFW 
1. `brew install fatsort` -> Install fatsort using `brew.sh` (only on macOS)
2. `diskutil list` -> Find your disk name for flipper microSD
3. `diskutil unmount /Volumes/Flipper\ SD`
4. `sudo fatsort -n /dev/disk4s1` -> Replace `disk4s1` with your microSD id found on step 2

## Your Flipper feels slow and unresponsive?

1. Make sure you using good microSD card from known brand, flipper works with microSD via SPI that means not any microSD will work good even if it works ok with other devices.
2. Go into `Settings -> System` and make sure that you have
    ```text
    Log Level = None
    Debug = OFF
    Heap Trace = None
    ```
3. If some of that settings is set to something different - change it to `None` / `OFF` 
4. Make sure your battery is charged, that can affect performance too

## Flipper crashed, stuck, frozen?

Reboot it by holding `Left` + `Back` buttons

![how to reboot flipper gif, shows how to hold left and back button](https://media.tenor.com/eUbBDDEzmwMAAAAC/flipper-zero-flipper-zero-reboot.gif)

## How to reset forgotten Flipper pin code?

**Disconnect USB Cable if it was connected**
1. Turn off the device - hold back button -> `Turn Off` 
**If you can't turn it off, try next step but hold buttons for 30-40 seconds)**
2. Hold <kbd>Up</kbd> + <kbd>Back</kbd> for `~5 sec` -> You will see reset screen -> Hold <kbd>Right</kbd> to reset (and <kbd>Down</kbd> arrow to exit if you don't want to reset pin code)
3. Done, internal memory (dolphin level, settings, pin code, is erased to default settings)

## What are the differences between x, y and z firmware?

If you just got your flipper and not sure what will work better for you, start with original official firmware, if you think you need more features or want to remove subghz region locks then:<br />
* Try installing **Unleashed firmware**, which is fork of official firmware with many new features and preinstalled plugins (check out `e` build).<br />
* In other case, If you want to experiment more with UI and other things look for existing forks of Unleashed firmware.<br />
* Or, create your own fork with your own customisations<br />
* Also, before reporting any found issue make sure you are in correct repo, if you are using not **Unleashed**, but different fork or original firmware, do not report issue in **Unleashed firmware** repo or UL communities (Telegram, Discord, etc..)

## Is there a correct way to capturing Infrared signals?

There is indeed especially with AC units, a new documentation has been released with some notes and steps on capturing infrared signals correctly along with some example data so you are able to understand the difference visually between the two.

[More info](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/InfraredCaptures.md)

## NFC / RFID FAQ

From our good friend `@Equip` and  `@np0` <br />

### MIFARE Ultralight

Scan the card, hold the Flipper Zero up to the reader to get the password to unlock the rest of the sectors, then scan the card again.

### MIFARE DESFire / MIFARE Ultralight C

The Flipper Zero has no available attacks for this card currently.
 
### Bank cards

- You cannot clone bank cards
- The Flipper Zero cannot emulate bank cards
- The Flipper Zero cannot pretend to be a point of sale machine
 
### Amiibos

- `NTAG215`. That's it. It's not going on a MIFARE Classic. 
- Currently, you cannot write Amiibos to new physical tags. yet.
 
### HID / iClass

- `Picopass` iClass can be read using the `Picopass` reader plugin
- 26bit Picopass can be downgraded to H10301 RFID credentials (note, it is not guaranteed to work if the reader is not configured to read low frequency)
- Readers will need to be configured and have an LF RFID antenna in order to be read. Certain iClass readers are HF only, and do not have the ability to have LF configured
- **Emulation for Picopass** was added on July 26th, and the updated version can be found in latest releases of **Unleashed** firmware with apps preinstalled, or in official Apps Hub via Flipper Mobile app
- Write support for personalization mode cards is doable with app
- The Seader app and a [SAM expansion board](https://www.redteamtools.com/nard-sam-expansion-board-for-flipper-zero-with-hid-seos-iclass-sam/) will allow reading more secure HID cards, which may be helpful in downgrade attacks
 
### LF-RFID

If you're wanting to make clones of low frequency RFID chips you need to write to T5577's. `Blanks` do not exist. All of the chips the Flipper Zero can interact with are read-only and cannot be overwritten or purchased blank. 

T5577s are multi-emulator chips that the Flipper Zero can program to be other tags
 
### Unknown Card / Fob

If you have exhausted all options of scanning via NFC / RFID / PICOPASS then take a photo of:

- The front and back of your credential 
- The reader you use with the credential 
- If your credential is a card, hold it up to a very bright light source e.g. a lightbulb and take a photo of the exposed antenna. This is useful for identification, post it for us to identify!

## How do I access the CLI / Logs?

To access the Serial CLI, click one of the following based on your platform.

<blockquote>
  <details>
    <summary>Desktop web browser*</summary>
    <em>*Chromium browsers only, such as: Google Chrome, Microsoft Edge, Opera / Opera GX, Brave, and Vivaldi.</em>
    <ul>
      <li>Connect your Flipper via USB.</li>
      <li>Ensure qFlipper and any other serial terminals are closed.</li>
      <li>Open <a href="https://my.flipp.dev/">my.flipp.dev</a> in one of the aforementioned browsers.</li>
      <li>Click <kbd>CONNECT</kbd> and select <kbd>USB Serial Device</kbd> from the list.</li>
      <li>Wait until you can see your device details on screen.</li>
      <li>Select the 💻 CLI item from the left sidebar.</li>
      <li><strong>Done!</strong></li>
    </ul>
  </details>
</blockquote>
<blockquote>
  <details>
    <summary>Windows</summary>
    <ul>
      <li>Install <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html">PuTTY</a> if it isn't already.</li>
      <li>Connect your Flipper via USB.</li>
      <li>Open qFlipper and look for the COM port next to the Flipper's name. <em> (Should say COM followed by a number, like <kbd>COM1</kbd>)</em></li>
      <li>Take note of the COM port number.</li>
      <li><strong>CLOSE qFlipper</strong>, otherwise the next steps won't work.</li>
      <li>Open PuTTY and ensure you're on the Session screen.</li>
      <li>Select <kbd>Serial</kbd> under connection type.</li>
      <li>Set serial line to the COM port. <em> (Just COM followed by the number, like <kbd>COM1</kbd>)</em></li>
      <li>Set speed to <code>115200</code></li>
      <li><em>Optional: Save the session settings for easy connection later.</em></li>
      <li>Finally, click <kbd>Open</kbd> to enter the CLI.</li>
      <li><strong>Done!</strong></li>
      <li>If you get an "Access Denied" error, make sure qFlipper isn't running!</li>
    </ul>
  </details>
</blockquote>
<blockquote>
  <details>
    <summary>MacOS/Linux</summary>
    <em>Note: I'm a filthy Windows user without any way to verify this procedure. Let me know if it's wrong!</em>
    <ul>
      <li>Install <a href="https://www.gnu.org/software/screen/">GNU Screen</a> if it isn't already.</li>
      <li>Connect your Flipper via USB.</li>
      <li>Open qFlipper and look for the device path next to the Flipper's name. <em>(Starts with /dev/tty)</em></li>
      <li><em>Alternatively: Run <code>ls /dev/tty.*</code> in a terminal.</em></li>
      <li>Take note of the full device path.</li>
      <li><strong>CLOSE qFlipper</strong>, otherwise the next steps won't work.</li>
      <li>Open a terminal.</li>
      <li>Run <code>screen PATH 115200</code>, replacing PATH with the device path from earlier.</li>
      <li><strong>Done!</strong></li>
    </ul>
  </details>
</blockquote>
<blockquote>
  <details>
    <summary>Android</summary>
    <ul>
      <li>Install <a href="https://play.google.com/store/apps/details?id=de.kai_morich.serial_usb_terminal">Serial USB Terminal</a> if it isn't already.</li>
      <li>Open the app and go to the Connections screen in the hamburger menu <em>(3 bars icon)</em></li>
      <li>Connect your Flipper via USB.</li>
      <li>Click the refresh icon if it doesn't automatically show up.</li>
      <li>Allow Serial USB Terminal to access Flipper if prompted.</li>
      <li>If it doesn't automatically connect, click the connect icon in the upper right. <em>(2 plugs icon)</em></li>
      <li><strong>Done!</strong></li>
      <li><em>Note: To exit log mode, you'll have to disconnect and reconnect using the icon.</em></li>
    </ul>
  </details>
</blockquote>
<blockquote>
  <details>
    <summary>iPhone</summary>
    Unfortunately, iOS is incapable of accessing a serial terminal over USB; try one of the other methods<br />
  <ul>
  <li>On the Flipper, open the settings, go to System, and set Log Level to Debug. <em>(You can keep Debug set to off unless someone asks you to turn it on)</em></li>
  <li>Once you have the CLI open, type <code>log</code> and press enter to start watching logs. Press <kbd>Ctrl-C</kbd> or <kbd>Cmd-C</kbd> to exit log mode.</li>
</ul>
    </details>
</blockquote>

<br />
<br />

**CLI FAQ Source + Check out this FAQ for more info:**<br /><br />
https://github.com/djsime1/awesome-flipperzero/blob/main/FAQ.md
