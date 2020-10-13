<img width="450" src="https://habrastorage.org/webt/la/fp/fz/lafpfzh4fsihzkdx0x_e5hofmdi.png" />

Flipper Zero support **optional** micro SD-card for expanding file system. It can store additional assets, plugins, libraries and so on. There is official SD-card image supplied with firmware updates, user can upload it using desktop firmware update util

# Supported cards

- micro SD HC class 1? **TODO: What actually types are NOT supported?**
- FAT/exFAT filesystem
- GPT and MBR partitioning table **TODO: Not clear in FatFS library docs**
- Max size: up to 2TB **TODO: not tested, on 8GB confirmed**
- Read/Write speed: up to 500 kbit/s **TODO: not clearly tested**
- Built-in filesystem **TODO: not sure**

# File manager

File manager allows user to:  

- See information of filesystem
- Format sd-card to exFAT
- View files list
- View file info 
- Run executable file: `.bin`, `.py`
- Delete file

## SD-card not inserted 

If SD-card is not inserted, statusbar is empty. File manager application menu only shows help text "SD-card not found".   
**TODO:** Do we need `Scan for sd-card` action when card not found automatically?  

![](./../../wiki_static/applications/sd-card/sd-card-not-found.jpg)

## SD-card inserted and mounted correctly 

When SD-card with correct fylesystem inserted, Flipper automatically trying to mount filesystem. If filesystem mounted correcly, the normal SD-card icon brings in statusbar.  

![](./../../wiki_static/ui/status-bar-sdcard-ok.png)

![](./../../wiki_static/applications/sd-card/sd-card-file-manager-ok.jpg)

## SD-card inserted and mount failed

If SD-card cannot be mounted because of not supported filesystem or any other reason, statusbar icon indicates this error.  User can go to `File Manager` and see the info about failed card and the exact error code or full message. Also can format the whole card to supported filesystem and partition table.

![](./../../wiki_static/ui/status-bar-sdcard-fail.png)

![](./../../wiki_static/applications/sd-card/sd-card-error.jpg)


### Card Info 

Press `← Left` to see the card info:

- Size
- Partition type: GPT, MBR
- Partitions with title and size

![](./../../wiki_static/applications/sd-card/sd-card-info.jpg)

### Format (erase card)

Press `→ Right` to format card. One action should completely erase card and create one parition with recommended filesystem (exFAT?).

![](./../../wiki_static/applications/sd-card/sd-card-format.jpg)