# LF RFID key file format {#lfrfid_file_format}

## Example

```
Filetype: Flipper RFID key
Version: 1
Key type: EM4100
Data: 01 23 45 67 89
```

## Description

Filename extension: `.rfid`

The file stores a single RFID key of the type defined by the `Key type` parameter.

### Version history

1. Initial version.

### Format fields

| Name     | Description           |
| -------- | --------------------- |
| Key type | Key protocol type     |
| Data     | Key data (HEX values) |

### Supported key types

| Type        | Full name         |
| ----------- | ----------------- |
| EM4100      | EM-Micro EM4100   |
| H10301      | HID H10301        |
| Idteck      | IDTECK            |
| Indala26    | Motorola Indala26 |
| IOProxXSF   | Kantech IOProxXSF |
| AWID        | AWID              |
| FDX-A       | FECAVA FDX-A      |
| FDX-B       | ISO FDX-B         |
| HIDProx     | Generic HIDProx   |
| HIDExt      | Generic HIDExt    |
| Pyramid     | Farpointe Pyramid |
| Viking      | Viking            |
| Jablotron   | Jablotron         |
| Paradox     | Paradox           |
| PAC/Stanley | PAC/Stanley       |
| Keri        | Keri              |
| Gallagher   | Gallagher         |
| GProxII     | Guardall GProx II |
