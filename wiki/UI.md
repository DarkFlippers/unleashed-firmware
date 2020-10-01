# Controls 

Flipper Zero have 5 button directional pad.  
Buttons `← Left`, `→ Right`, `↑ Up`, `↓ Down` can be pressed only one at a time, and **CAN'T BE used in combination**.  
Buttons `◉ OK` and `⮌ Back` is independed from other buttons and can be used for combinations.

```
        Up
        ↑
Left ← (OK) → Right
        ↓
       Down         ⮌ Back   
```

![](./../wiki_static/ui/controls.png)

On directinal pad only one button out of 4 can be pressed at once. `◉ OK` and `⮌ Back` buttons are indepandent.  

# Reboot combination

Reset combination triggers hard reboot that cannot be intercepted by any software. Used when application freezes or as normal way to reboot device.
 
 1. Press and hold `Left ←` + `⮌ Back` for reset
 2. Release to normal boot

![Reboot combination](../wiki_static/ui/reboot-combo.png)

# Reboot to Bootloader (Firmware update mode)

Same as hard reboot, but stay in Firmware Update mode after boot. Useful when firmware is broken and user cannot boot in normal mode.

1. Press and hold `Left ←` + `⮌ Back` for reset
2. Release `Left ←` and keep holding `⮌ Back` until `Firmware Update` mode appears
3. Release `⮌ Back`

To exit from `Firmware Update` mode use [reboot combination](#reboot-combination) 

![Reboot to Bootloader](./../wiki_static/ui/reboot-to-bootloader.png)

# DFU mode (Rescue mode)

Reboot to rescue DFU mode. Useful when firmware and bootloader is broken.  

1. Press and hold `Left ←` + `⮌ Back` + `◉ OK`
2. Release `Left ←` + `⮌ Back` and keep `◉ OK` pressed
3. Release `◉ OK`

To exit from `Firmware Update` mode use [reboot combination](#reboot-combination) 

![](../wiki_static/ui/reboot-to-dfu.png)

# Menu structure

![](../wiki_static/ui/menu-navigation.png)

# Standby screen 

On standby screen we can see battery, status bar and dolphin animation. Animation can cover the status bar according to the battery charge width.

![](../wiki_static/ui/UI-Standby.png)

![](../wiki_static/ui/UI-Standby-status-1.png)

![](../wiki_static/ui/UI-Standby-status-88.png)

# Main menu

Active row is always in the middle of the screen.

![](./../wiki_static/ui/UI-main-menu-screen.png)

### Active row

Font: `Born2bSportyV2`  
Size: 16 px  

![](./../wiki_static/ui/UI-active-row-text.png)

### Non-active row

Font: `Helvetipixel`  
Size: 16 px  

![](./../wiki_static/ui/UI-non-active-row-text.png)

### Icon

![](./../wiki_static/ui/UI-icon.png)

### Scrollbar

Scrollbar has 32 dots and a simple thumb.

![](./../wiki_static/ui/UI-Scrollbar-and-thumb.png)

# Application menu

Font: `Helvetipixel`  
Size: 16 px

![](../wiki_static/ui/UI-app-menu.png)

# Application screen with buttons

Font: `Helvetipixel`  
Size: 16 px

Minimum button width is 38 px.  
Maximum button width is 63 px.

![](../wiki_static/ui/UI-app-menu-buttons.png)

# Pass-code lock

Flipper will store your secrets like U2F token and house keys. What if Flipper gets lost?

Well, no problem! For this case, we made a locking feature. You can lock Flipper and unlock it with the special pass-combo, like in good old fighting games (for example, →↑↑←↓↑↓). Users can set a pass-combo of any length, and all functions will be blocked until the combo is entered, including firmware flashing.

![](https://ksr-ugc.imgix.net/assets/030/153/925/13404091a9c1bb3390a67afe279a0051_original.gif?ixlib=rb-2.1.0&w=700&fit=max&v=1597158235&auto=format&gif-q=50&q=92&s=06a640ecaa809487b004c1bead0fd9cc)