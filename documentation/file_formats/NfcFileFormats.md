# NFC Flipper File Formats

## NFC-A (UID) + Header

### Example

    Filetype: Flipper NFC device
    Version: 3
    # Nfc device type can be UID, Mifare Ultralight, Mifare Classic, Bank card
    Device type: UID
    # UID, ATQA and SAK are common for all formats
    UID: 04 85 92 8A A0 61 81
    ATQA: 00 44
    SAK: 00

### Description

This file format is used to store the UID, SAK and ATQA of a NFC-A device. It does not store any internal data, so it can be used for multiple different card types. Also used as a header for other formats.

Version differences:

  1. Initial version, deprecated
  2. LSB ATQA (e.g. 4400 instead of 0044)
  3. MSB ATQA (current version)

UID can be either 4 or 7 bytes long. ATQA is 2 bytes long. SAK is 1 byte long.

## Mifare Ultralight/NTAG

### Example

    Filetype: Flipper NFC device
    Version: 3
    # Nfc device type can be UID, Mifare Ultralight, Mifare Classic
    Device type: NTAG216
    # UID, ATQA and SAK are common for all formats
    UID: 04 85 90 54 12 98 23
    ATQA: 00 44
    SAK: 00
    # Mifare Ultralight specific data
    Data format version: 1
    Signature: 1B 84 EB 70 BD 4C BD 1B 1D E4 98 0B 18 58 BD 7C 72 85 B4 E4 7B 38 8E 96 CF 88 6B EE A3 43 AD 90
    Mifare version: 00 04 04 02 01 00 13 03
    Counter 0: 0
    Tearing 0: 00
    Counter 1: 0
    Tearing 1: 00
    Counter 2: 0
    Tearing 2: 00
    Pages total: 231
    Pages read: 231
    Page 0: 04 85 92 9B
    Page 1: 8A A0 61 81
    Page 2: CA 48 0F 00
    ...
    Page 224: 00 00 00 00
    Page 225: 00 00 00 00
    Page 226: 00 00 7F BD
    Page 227: 04 00 00 E2
    Page 228: 00 05 00 00
    Page 229: 00 00 00 00
    Page 230: 00 00 00 00
    Failed authentication attempts: 0

### Description

This file format is used to store the UID, SAK and ATQA of a Mifare Ultralight/NTAG device. It also stores the internal data of the card, the signature, the version, and the counters. The data is stored in pages, just like on the card itself.

The "Signature" field contains the reply of the tag to the READ_SIG command. More on that can be found here: <https://www.nxp.com/docs/en/data-sheet/MF0ULX1.pdf> (page 31)

The "Mifare version" field is not related to the file format version, but to the Mifare Ultralight version. It contains the responce of the tag to the GET_VERSION command. More on that can be found here: <https://www.nxp.com/docs/en/data-sheet/MF0ULX1.pdf> (page 21)

Other fields are the direct representation of the card's internal state, more on them can be found in the same datasheet.

Version differences:

  1. Current version

## Mifare Classic

### Example

    Filetype: Flipper NFC device
    Version: 3
    # Nfc device type can be UID, Mifare Ultralight, Mifare Classic
    Device type: Mifare Classic
    # UID, ATQA and SAK are common for all formats
    UID: BA E2 7C 9D
    ATQA: 00 02
    SAK: 18
    # Mifare Classic specific data
    Mifare Classic type: 4K
    Data format version: 2
    # Mifare Classic blocks, '??' means unknown data
    Block 0: BA E2 7C 9D B9 18 02 00 46 44 53 37 30 56 30 31
    Block 1: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 2: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 3: FF FF FF FF FF FF FF 07 80 69 FF FF FF FF FF FF
    Block 4: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 5: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 6: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 7: FF FF FF FF FF FF FF 07 80 69 FF FF FF FF FF FF
    ...
    Block 238: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 239: FF FF FF FF FF FF FF 07 80 69 FF FF FF FF FF FF
    Block 240: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 241: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 242: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 243: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 244: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 245: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 246: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 247: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 248: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 249: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 250: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 251: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 252: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 253: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 254: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    Block 255: FF FF FF FF FF FF FF 07 80 69 FF FF FF FF FF FF

### Description

This file format is used to store the NFC-A and Mifare Classic specific data of a Mifare Classic card. Aside from the NFC-A data, it stores the card type (1K/4K) and the internal data of the card. The data is stored in blocks, there is no sector grouping. If the block's data is unknown, it is represented by '??'. Otherwise, the data is represented as a hex string.

Version differences:

  1. Initial version, has Key A and Key B masks instead of marking unknown data with '??'.

Example:

    ...
    Data format version: 1
    # Key map is the bit mask indicating valid key in each sector
    Key A map: 000000000000FFFF
    Key B map: 000000000000FFFF
    # Mifare Classic blocks
    ...
  
  2. Current version

## Mifare DESFire

### Example

    Filetype: Flipper NFC device
    Version: 3
    # Nfc device type can be UID, Mifare Ultralight, Mifare Classic
    Device type: Mifare DESFire
    # UID, ATQA and SAK are common for all formats
    UID: 04 2F 19 0A CD 66 80
    ATQA: 03 44
    SAK: 20
    # Mifare DESFire specific data
    PICC Version: 04 01 01 12 00 1A 05 04 01 01 02 01 1A 05 04 2F 19 0A CD 66 80 CE ED D4 51 80 31 19
    PICC Free Memory: 7520
    PICC Change Key ID: 00
    PICC Config Changeable: true
    PICC Free Create Delete: true
    PICC Free Directory List: true
    PICC Key Changeable: true
    PICC Max Keys: 01
    PICC Key 0 Version: 00
    Application Count: 1
    Application IDs: 56 34 12
    Application 563412 Change Key ID: 00
    Application 563412 Config Changeable: true
    Application 563412 Free Create Delete: true
    Application 563412 Free Directory List: true
    Application 563412 Key Changeable: true
    Application 563412 Max Keys: 0E
    Application 563412 Key 0 Version: 00
    Application 563412 Key 1 Version: 00
    Application 563412 Key 2 Version: 00
    Application 563412 Key 3 Version: 00
    Application 563412 Key 4 Version: 00
    Application 563412 Key 5 Version: 00
    Application 563412 Key 6 Version: 00
    Application 563412 Key 7 Version: 00
    Application 563412 Key 8 Version: 00
    Application 563412 Key 9 Version: 00
    Application 563412 Key 10 Version: 00
    Application 563412 Key 11 Version: 00
    Application 563412 Key 12 Version: 00
    Application 563412 Key 13 Version: 00
    Application 563412 File IDs: 01
    Application 563412 File 1 Type: 00
    Application 563412 File 1 Communication Settings: 00
    Application 563412 File 1 Access Rights: EE EE
    Application 563412 File 1 Size: 256
    Application 563412 File 1: 13 37 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

### Description

This file format is used to store the NFC-A and Mifare DESFire specific data of a Mifare DESFire card. Aside from the NFC-A data, it stores the card type (DESFire) and the internal data of the card. The data is stored per-application, and per-file. Here, the card was written using those pm3 commands:

    hf mfdes createapp --aid 123456 --fid 2345 --dfname astra
    hf mfdes createfile --aid 123456 --fid 01 --isofid 0001 --size 000100
    hf mfdes write --aid 123456 --fid 01 -d 1337

Version differences:
  None, there are no versions yet.

## Mifare Classic Dictionary

### Example

    # Key dictionary from https://github.com/ikarus23/MifareClassicTool.git

    # More well known keys!
    # Standard keys
    FFFFFFFFFFFF
    A0A1A2A3A4A5
    D3F7D3F7D3F7
    000000000000

    # Keys from mfoc
    B0B1B2B3B4B5
    4D3A99C351DD
    1A982C7E459A
    AABBCCDDEEFF
    714C5C886E97
    587EE5F9350F
    A0478CC39091
    533CB6C723F6
    8FD0A4F256E9
    ...

### Description

This file contains a list of Mifare Classic keys. Each key is represented as a hex string. Lines starting with '#' are ignored as comments. Blank lines are ignored as well.

## EMV resources

### Example

    Filetype: Flipper EMV resources
    Version: 1
    # EMV currency code: currency name
    0997: USN
    0994: XSU
    0990: CLF
    0986: BRL
    0985: PLN
    0984: BOV
    ...

### Description

This file stores a list of EMV currency codes, country codes, or AIDs and their names. Each line contains a hex value and a name separated by a colon and a space.

Version differences:

  1. Initial version
