# Sub-GHz Supported Protocols

This file lists all supported Sub-GHz protocols available in Unleashed Firmware, both tested and untested.

That list is only for default SubGHz app, apps like *Weather Station* have their own protocols list


## Static & Dynamic protocols list

*433 MHz usually means `433.92MHz` and 868 MHz = `868.35MHz`*

*If you see no frequency after protocol name, that means we don't know it, let us know in issues tab!*

*`AM650`, `FM`, `FSK476` - means modulation to use when reading the remote* 

*`FM` means you should try any existing FM modulations, `FSK???` means `FM???` in SubGHz - Read - Config*

### Garage Door Openers & Gate Openers (Boom barriers, roller shutters, etc.)
- Alutech AT-4N `433.92MHz` `AM650` (72 bits, Dynamic)
- AN-Motors (Alutech) AT4 `433.92MHz` `AM650` (64 bits, Pseudo-Dynamic, KeeLoq based)
- Ansonic `433MHz` `FM` (12 bits, Static)
- BETT `433.92MHz` `AM650` (18 bits, Static)
- BFT Mitto `433.92MHz` `AM650` (64 bits, Dynamic, KeeLoq based with Seed)
- CAME Atomo `433.92MHz, 868MHz` `AM650` (62 bits, Dynamic)
- CAME TWEE `433.92MHz` `AM650` (54 bits, Static)
- CAME `433.92MHz, 868MHz` `AM650` (12, 24 bits, Static)
- Prastel `433.92MHz, 868MHz` `AM650` (25, 42 bits, Static)
- Airforce `433.92MHz, 868MHz` `AM650` (18 bits, Static)
- Chamberlain Code `AM650` (10 bits, Static)
- Clemsa `AM650` (18 bits, Static)
- Dickert MAHS `AM650` (36 bits, Static)
- Doitrand `AM650` (37 bits, Dynamic)
- Elplast/P-11B/3BK/E.C.A `433MHz` `AM650` (18 bits, Static)
- FAAC SLH `433.92MHz, 868MHz` `AM650` (64 bits, Dynamic)
- Gate TX `433.92MHz` `AM650` (64 bits, Static)
- Hormann `868MHz` `AM650` (44 bits, Static)
- HCS101 `AM650` (64 bits, Simple Dynamic, KeeLoq-like)
- IDO `433MHz` `AM650` (48 bits, Dynamic)
- KingGates Stylo 4k `433.92MHz` `AM650` (89 bits, Dynamic, KeeLoq based)
- Mastercode `AM650` (36 bits, Static)
- Megacode `AM650` (24 bits, Static)
- Nero Sketch `AM650` (40 bits, Static)
- Nice Flo `433.92MHz` `AM650` (12, 24 bits, Static)
- Nice FloR-S `433.92MHz` `AM650` (52 bits, Dynamic)
- Nice One `433.92MHz` `AM650` (72 bits, Dynamic)
- Revers RB2 (Реверс РБ-2 (М)) `433.92MHz` `AM650` (64 bits, Static)
- Roger `433.92MHz` `AM650` (28 bits, Static)
- V2 Phoenix (Phox) `433.92MHz` `AM650` (52 bits, Dynamic)
- Marantec `433.92MHz, 868MHz` `AM650` (49 bits, Static)
- Marantec24 `868MHz` `AM650` (24 bits, Static)
- Somfy Keytis `433.92MHz, 868MHz` `AM650` (80 bits, Dynamic)
- ZKTeco `430.5MHz` `AM650` (24 bits, Static - Princeton based) - (Button codes (already mapped to arrow keys): `0x30 (UP)`, `0x03 (STOP)`, `0x0C (DOWN)`) 
- Linear `300MHz` `AM650` (10 bits, Static)
- Linear Delta3 `AM650` (8 bits, Static)
- Nero Radio `434MHz` `AM650` (56 bits, Static mode only, Dynamic is unsupported)
- Security+1.0 `315MHz, 433.92MHz, 390MHz` `AM650` (42 bits, Dynamic)
- Security+2.0 `310MHz, 390MHz, 868MHz` `AM650` (62 bits, Dynamic)

### Sensors & Smart home
- Intertechno V3 `AM650` (32 bits, Static) - Lights, sockets, other.
- Dooya `AM650` (40 bits, Static) - Electric blinds
- Power Smart `AM650` (64 bits, Static) - Blinds, shutters
- Legrand `AM650` (18 bits, Static) - Doorbells
- Somfy Telis `433.92MHz` `AM650` (56 bits, Dynamic)
- Feron `433.92MHz` `AM650` (32 bits, Static) - RGB LED remotes, other.
- Honeywell `AM650` (64 bits, Dynamic) - Alarm, Sensor
- Honeywell WDB `AM650` (48 bits, Dynamic) - Doorbell
- Magellan `433.92MHz` `AM650` (32 bits, Static) - Sensor, alarm

### Alarms
- Hollarm `433.92MHz` `AM650` (42 bits, Static) - Bike alarms
- Scher Khan `433.92MHz` `AM650` (35-82 bits, Dynamic) - Russian external car alarm system (Decode and display only), 200x year
- Kia `433/434MHz` `FSK476` (61 bits, Dynamic) - Car alarm system (Decode and display only)
- Star Line `433.92MHz` `AM650` (KeeLoq based, 64 bits) - Russian external car alarm system, 200x year
- GangQi `433.92MHz` `AM650` (34 bits, Static) - Bike alarms


### Generic any branded remotes
- Holtek `AM650` (40 bits, Static)
- Holtek HT12X `AM650` (12 bits, Static)
- Princeton (PT2262, PT****) `315MHz, 433.92MHz, Any other frequency` `AM650` (24 bits, Static)
- SMC5326 `315MHz, 433.92MHz, Any other frequency` `AM650` (25 bits, Static)
- Hay21 `433.92MHz` `AM650` (21 bits, Dynamic)

---

## KeeLoq Rolling Code Supported Manufacturers list

KeeLoq is a rolling code encryption system used by many garage door openers and gate systems. 
The following manufacturers have KeeLoq support in Unleashed firmware:

*Default value for encryption type "learning" is `simple` and `10bits` for serial part in Hop*

*In case if remote uses other serial bits or different encryption type and it was verified - it will be stated in the end*

### Garage Door Openers & Gate Openers (Boom barriers, roller shutters, etc.)
- Allmatic - `868MHz` `AM650` (KeeLoq, 64 bits) (no serial part in Hop - magic XOR)
- Aprimatic - `433.92MHz` `AM650` (KeeLoq, 64 bits) (12bit serial number art in Hop + 2bit "parity" in front of it replacing first 2 bits of serial)
- Beninca - `433.92MHz, 868MHz` `AM650` (KeeLoq, 64 bits) (no serial part in Hop - magic XOR)
- CAME Space - `433.92MHz` `AM650` (KeeLoq, 64 bits) (12bit serial part in Hop - simple learning)
- Cardin S449 - `433.92MHz` `FSK12K` (KeeLoq, 64 bits) (12bit (original remotes) or 10bit (chinese remotes) serial part in Hop - normal learning)
- Centurion - `433.92MHz` `AM650` (KeeLoq, 64 bits) (no serial in Hop, uses fixed value 0x1CE - normal learning)
- Comunello - `433.92MHz, 868MHz` `AM650` (KeeLoq, 64 bits) (normal learning)
- DEA Mio - `433.92MHz` `AM650` (KeeLoq, 64 bits) (modified serial in Hop, uses last 3 digits modifying first one (example - 419 -> C19) - simple learning)
- DoorHan - 315MHz, `433.92MHz` `AM650` (KeeLoq, 64 bits)
- DTM Neo - `433.92MHz` `AM650` (KeeLoq, 64 bits) (12bit serial part in Hop - simple learning)
- Elmes Poland - `433.92MHz` `AM650` (KeeLoq, 64 bits) (normal learning)
- FAAC RC,XT - `433.92MHz, 868MHz` `AM650` (KeeLoq, 64 bits) (12bit serial part in Hop - normal learning)
- Genius Bravo - `433.92MHz` `AM650` (KeeLoq, 64 bits) (12bit serial part in Hop - normal learning)
- Gibidi - `433.92MHz` `AM650` (KeeLoq, 64 bits)
- GSN - `433.92MHz` `AM650` (KeeLoq, 64 bits) (12bit serial part in Hop - normal learning)
- Hormann EcoStar - `433.92MHz` `AM650` (KeeLoq, 64 bits) (normal learning)
- IronLogic IL-100 - `433.92MHz` `AM650` (KeeLoq, 64 bits)
- IronLogic IL-100 Smart PPult - `433.92MHz` `AM650` (KeeLoq, 64 bits)
- JCM Tech - `433.92MHz` `AM650` (KeeLoq, 64 bits) (8bit serial part in Hop - simple learning)
- Jolly Motors - `433.92MHz` `AM650` (KeeLoq, 64 bits)
- KEY (KeeLoq, 64 bits)
- Monarch - `433.92MHz` `AM650` (KeeLoq, 64 bits) (no serial in Hop, uses fixed value 0x100 - normal learning)
- Motorline - `433.92MHz` `AM650` (KeeLoq, 64 bits) (normal learning)
- Mutanco/Mutancode (KeeLoq, 64 bits) (12bit serial part in Hop - normal learning)
- Mhouse - `433.92MHz` `AM650` (KeeLoq, 64 bits) (8bit serial part in Hop - simple learning)
- Nice Smilo - `433.92MHz` `AM650` (KeeLoq, 64 bits) (8bit serial part in Hop - simple learning)
- Normstahl - `433.92MHz` `AM650` (KeeLoq, 64 bits)
- Novoferm - `433.92MHz` `AM650` (KeeLoq, 64 bits)
- Sommer `434.42MHz, 868.80MHz` `FSK12K (or FSK476)` (KeeLoq, 64 bits) (normal learning)
- Steelmate - `433.92MHz` `AM650` (KeeLoq, 64 bits) (12bit serial part in Hop - normal learning)
- Stilmatic - `433.92MHz` `AM650` (KeeLoq, 64 bits) (normal learning)

### Alarms, unknown origin, etc.
- APS-1100/APS-2550 (KeeLoq, 64 bits)
- Alligator (KeeLoq, 64 bits)
- Alligator S-275 (KeeLoq, 64 bits)
- Cenmax (KeeLoq, 64 bits)
- Cenmax St-5 (KeeLoq, 64 bits)
- Cenmax St-7 (KeeLoq, 64 bits)
- Faraon (KeeLoq, 64 bits)
- Guard RF-311A (KeeLoq, 64 bits) (normal learning)
- Harpoon (KeeLoq, 64 bits)
- KGB/Subaru (KeeLoq, 64 bits) (magic serial type 1)
- Leopard (KeeLoq, 64 bits)
- Magic 1 (KeeLoq, 64 bits) (magic serial type 2)
- Magic 2 (KeeLoq, 64 bits) (magic serial type 2)
- Magic 3 (KeeLoq, 64 bits) (magic serial type 3)
- Magic 4 (KeeLoq, 64 bits) (magic serial type 3)
- Merlin (KeeLoq, 64 bits) (no serial part in Hop - simple XOR)
- Mongoose (KeeLoq, 64 bits) (normal learning)
- Pantera (KeeLoq, 64 bits)
- Pantera CLK (KeeLoq, 64 bits)
- Pantera XS/Jaguar (KeeLoq, 64 bits)
- Partisan RX (KeeLoq, 64 bits)
- Pecinin (KeeLoq, 64 bits) (12bit serial part in Hop - simple learning)
- Reff (KeeLoq, 64 bits)
- Rossi (KeeLoq, 64 bits) (12bit serial part in Hop - simple learning)
- Rosh (KeeLoq, 64 bits) (12bit serial part in Hop - simple learning)
- Sheriff (KeeLoq, 64 bits)
- SL A2-A4 (KeeLoq, 64 bits)
- SL A6-A9/Tomahawk 9010 (KeeLoq, 64 bits)
- SL B6,B9 dop (KeeLoq, 64 bits)
- Teco (KeeLoq, 64 bits)
- Tomahawk TZ-9030 (KeeLoq, 64 bits)
- Tomahawk Z,X 3-5 (KeeLoq, 64 bits)
- ZX-730-750-1055 (KeeLoq, 64 bits)

*Note: Most KeeLoq manufacturers operate in the 433 MHz and 868 MHz frequency bands with AM650 modulation. Some operate at other frequencies or modulations. Not all KeeLoq systems are supported for full decoding or emulation.*

---

## Protocol Type Reference

Unleashed firmware supports various protocol types:

- **Static Code**: Fixed transmission codes (e.g., Roger, Princeton, Marantec, Revers RB2)
- **Rolling Code (Dynamic) (KeeLoq)**: Dynamic codes with rolling counter using KeeLoq encryption (60+ manufacturer systems supported)
- **Rolling Code (Dynamic)**: Other dynamic systems with custom encoding (e.g., CAME Atomo, Nice Flor S, Somfy Telis, FAAC SLH, Alutech AT-4N, Security+)

For more information on how to use some of these protocols, see also [SubGHzRemoteProg.md](/documentation/SubGHzRemoteProg.md) and the main [ReadMe.md](/ReadMe.md).

---

*Docs made for Unleashed FW, please mention source when copying*