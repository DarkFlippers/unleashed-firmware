![](./../../wiki_static/tools/st-link-v2.jpg)

# Install legacy stlink utils v.1.5.1

### macOS

```
brew tap zhovner/stlink-legacy
brew install stlink-legacy
```

# Downgrade ST-Link firmware 

Some Chinese ST-Link clones won't work with latest `stlink` utils and firmware. Download firmware to fix this

1. Connect St-Link adapter to Windows PC and install driver [ST-Link_Windows_driver.zip](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/tools/en.stsw-link009.zip)
2. Run [Firmware upgrade util](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/tools/stlink-V2.J21.S4.zip)

![](./../../wiki_static/tools/st-link-upgrade-util-windows.jpg)