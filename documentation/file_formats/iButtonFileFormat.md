# iButton key file format

## Example

```
Filetype: Flipper iButton key
Version: 2
Protocol: DS1992
Rom Data: 08 DE AD BE EF FA CE 4E
Sram Data: 4E 65 76 65 72 47 6F 6E 6E 61 47 69 76 65 59 6F 75 55 70 4E 65 76 65 72 47 6F 6E 6E 61 4C 65 74 59 6F 75 44 6F 77 6E 4E 65 76 65 72 47 6F 6E 6E 61 52 75 6E 41 72 6F 75 6E 64 41 6E 64 44 65 73 65 72 74 59 6F 75 4E 65 76 65 72 47 6F 6E 6E 61 4D 61 6B 65 59 6F 75 43 72 79 4E 65 76 65 72 47 6F 6E 6E 61 53 61 79 47 6F 6F 64 62 79 65 4E 65 76 65 72 47 6F 6E 6E 61 54 65 6C 6C 41 4C 69 65
```

## Description

Filename extension: `.ibtn`

The file stores a single iButton key, complete with all data required by the protocol.

## Version history
### 2. Current version.
Changelog:
- Added support for different Dallas protocols
- Fields after `Protocol` are protocol-dependent for flexibiliy

#### Format fields

| Name      | Type   | Description                                  |
| --------- | ------ | -------------------------------------------- |
| Protocol  | string | Currently supported: DS1990, DS1992, DS1996, DSGeneric*, Cyfral, Metakom |
| Rom Data  | hex    | Read-only memory data (Dallas protocols only) |
| Sram Data | hex    | Static RAM data (DS1992 and DS1996 only)
| Data      | hex    | Key data (Cyfral & Metakom only)              |

NOTE 1: DSGeneric is a catch-all protocol for all unknown 1-Wire devices. It reads only the ROM and does not perform any checks on the read data. 
It can also be used if a key with a deliberately invalid family code or checksum is required.

NOTE 2: When adding new protocols, it is not necessarily to increase the format version, define the format in the protocol implementation instead.

### 1. Initial version.
Deprecated, will be converted to current version upon saving.

#### Format fields

| Name     | Type   | Description                                  |
| -------- | ------ | -------------------------------------------- |
| Key type | string | Currently supported: Cyfral, Dallas, Metakom |
| Data     | hex    | Key data                                     |



