<h1 align="center">XFW - <code>Xtreme Firmware</code> for the Flipper Zero</h1>

<p align="center">
  <img src="https://user-images.githubusercontent.com/55334727/209031152-efbca66c-3fe0-41b5-a860-e0d7522a9279.png">
</p>

[Intro](https://github.com/ClaraCrazy/Flipper-Xtreme#What-makes-it-special) | [Animations](https://github.com/ClaraCrazy/Flipper-Xtreme#Animations) | [Changelog](https://github.com/ClaraCrazy/Flipper-Xtreme#list-of-changes) | [Known bugs](https://github.com/ClaraCrazy/Flipper-Xtreme#Known-bugs) | [Install](https://github.com/ClaraCrazy/Flipper-Xtreme#Install) | [Build](https://github.com/ClaraCrazy/Flipper-Xtreme#build-it-yourself) | [Discord](https://discord.gg/mC2FxbYSMr)
-----

This firmware is a complete overhaul of "RM" (RogueMaster), with changes on all graphics, `disgusting` code and more. Xtreme brings you the most for your little flipper companion.

-----
<br>
<h2 align="center">What makes it special?</h2>

This firmware is based on RogueMaster (I know, should have forked unleashed, too late now). It combines the bleeding-edge PRs of Rogues firmware with actual understanding of whats going on, fixing bugs that are regularly talked about, removing unstable / broken applications (.FAP) and actually using the level system that just sits abandoned in RM.
<br><br>
- The focus of this firmware is functionality & stability: If an App doesnt work, its gone

- Giving the level system a purpose: Right now, each level unlocks a new wallpaper. More on that below

- Clean upgraded code: RM wrote some updates to certain files. These are... painful, to say the least. Here its all rewritten and cleaned up. I invite you all to compare the `applications\services\dolphin\helpers\dolphin-state.c`, lol

- Up2Date: This firmware receives updates from a few repositories, not just from its Upstream. If there are functional, but yet un-merged Pull requests on another flipper firmware that are good, they will be in here!


-----
<br>
<h2 align="center">Animations:</h2>

This firmware contains NSFW animations. No, actually it consisnts purely of those in stock configuration. 

The animations are tied to the level system. Each level you reach, unlocks a new animation. The higher your level, the more lewd it will become. Rumors have it, I'm  to be found in at least one of those too

| Level  | Animations |
| ------------- | ------------- |
| 1-10  | Try harder. Just sexy clothes |
| 11-20 | Some tits, maybe an ass |
| 21-30 | Fully NSFW, graphic scenes |

If you dont like that, we removed the "Dummy mode" as it serves no purpose, and switched with an SFW toggle. From the main menu, hit `Arrow UP` and select SFW mode.

-----
<br>
<h2 align="center">List of changes:</h2>

Note: This repo is always updated with OFW, Unleashed & Rogue. No need to mention all those here. I will only mention **MY** changes

```txt
[Added]

- SFW Mode
- Jamming Files
- Custom subghz presets
- Subghz and IR signal replication via gpio (allows using external antenas and emitters)
- Honda Keys (CVE-2022-27254)
- NSFW Animations tied to the level system. Read more above
- New API Routes for Locale settings
- Scrolling view for long file names in browser
- Tamagotchi rom
```
```txt
[Updated]

- All graphics
- Folder handling for empty ones (Now indicate they are empty)
- Applications now use the new Locale setting
- Compiler now handles all non-compiled faps during build
- Compiler now accepts WIP SDK 
- Compiler just stfu about non-fatal problems
- Some further NFC stuff
- Weather App
- Applications now use below mentioned API Routes
```
```txt
[Fixed]

- Passport crash on high level
- SFW / Dummy_mode getting you XP
- Leveling system
- Mood system
```
```txt
[REMOVED]

- Unused Dummy Mode
- Broken apps (bad apple, chess, etc.)
- Unused code from FAPs and system calls
```

----
<br>
<h2 align="center">Known Bugs:</h2>

```txt
- Name Changer app crashes when you close it without using it
```

----
<br>
<h2 align="center">Install:</h2>

**NOTE: If you are coming from a different FW, it is recommended to delete / clear your "apps" folder on the SD card prior to updating. This folder houses all the .fap files, which do not update to the correct API versions by default if old ones are present (Thanks flipper devs). This does `NOT` remove any of your saved files!**
<br><br>

<h3> Web Updater / Mobile App / Automatic qFlipper Update</h3>

- Go to the latest release from [The releases tab](https://github.com/ClaraCrazy/Flipper-Xtreme/releases/latest)
- Download the .tgz file
- Run your updater of choice with it

- Enjoy!
<br>

-----
<br>

<h3> Manual Update (Because qFlipper loves to break every now and then for no reason at all)</h3>

- Download the latest release (.zip) from [The releases tab](https://github.com/ClaraCrazy/Flipper-Xtreme/releases/latest)
- Extract the archive. This is now your new Firmware folder
- Open [qFlipper](https://flipperzero.one/update), head to `SD/Update` and simply move the firmware folder there
- On the Flipper, hit the `Arrow Down` button, this will get you to the file menu. Hit `Arrow Left` once, and then simply search for your updates folder
- Inside that folder, select the Firmware you just moved onto it, and run the file thats simply called `Update`

- Enjoy!

----
<br>
<h2 align="center">Build it yourself:</h2>

```bash
To download the needed tools:
$ git clone --recursive https://github.com/ClaraCrazy/Flipper-Xtreme.git
$ cd Flipper-Xtreme/

To flash directly to the Flipper (Needs to be connected via USB, qFlipper closed)
$ ./fbt flash_usb

To just compile firmware
$ ./fbt updater_package

If building FAPS:
$ ./fbt fap_dist

If building image assets:
$ ./fbt resources icons dolphin_ext
```

----
<p align="center"> We're not waiting for a war... We bring the war to them! </p>
