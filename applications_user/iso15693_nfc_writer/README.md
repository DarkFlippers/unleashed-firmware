# ISO15693-3 NFC Writer

An application for the Flipper Zero device that provides advanced handling of ISO 15693-3 compliant NFC tags (commonly found as ICODE SLIX).
The application allows reading and writing memory data, managing block locks, and modifying special AFI and DSFID registers.
![App](screenshots/app.png)

## Key Features

### Data Reading and Writing
- **Write Single Block**: Writes defined data (default 4 bytes) to a selected block address.
![App](screenshots/write_single.png)
![App](screenshots/write_single1.png)
- **FF to All Blocks**: Quickly formats the tag by writing the value `0xFF` to all memory blocks.
![App](screenshots/formating.png)

- **Write Dump**: Restores or writes the full tag memory content from a text file stored on the SD card.
![App](screenshots/write.png)

- **Read Single Block**: Reads and displays the content of a single memory block at a selected address.
![App](screenshots/read_single.png)
![App](screenshots/read_single1.png)

- **Read Dump**: Reads the entire content of the tag's memory. After reading, the data can be viewed and saved to a file on the SD card.
![App](screenshots/dumping.png)
![App](screenshots/read.png)

### Locking
- **Lock Block**: Permanently locks a selected block against re-writing (irreversible operation).
- **Lock All Blocks**: Sequentially locks all blocks in the tag.

### AFI and DSFID Management
The application allows manipulation of system identifiers:
- **AFI (Application Family Identifier)**: Set, write, and lock the AFI value.
- **DSFID (Data Storage Format Identifier)**: Set, write, and lock the DSFID value.

### Configuration
Configuration options for operation parameters are available in the main menu:
- **Set Data**: Set the data bytes to be written (used for the Write Single Block operation).
![App](screenshots/set_data.png)

- **Set Address**: Select the block address for single block operations (read, write, lock).
![App](screenshots/set_adress.png)
- **Set AFI / Set DSFID**: Enter hexadecimal values for AFI and DSFID identifiers before writing them.

## File Structure
Memory dumps are saved to and read from the following directory on the SD card: /ext/apps_data/ISO15693-3_Writer/

