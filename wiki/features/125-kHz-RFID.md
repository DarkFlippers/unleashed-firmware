![Flipper Zero RFID](https://habrastorage.org/webt/jd/fb/yb/jdfbybzgw6qvg5kjeuvfg8w0iou.png)

Low-frequency proximity cards are widely used in access control systems around the world. It's pretty dumb, keeps only a short few-byte ID, and has no authentication mechanism, allowing it to be easily read, cloned, and emulated by anyone. A 125 kHz antenna is located on the bottom of Flipper's body.

## Card types supported

### Mandatory
* EM400x, EM410x, EM420x
* HID Prox (Proxcard, ISOProx, ProxKey). Not project Keysy told: _Emulation not supported on multiClass readers, can still clone to rewritable_
* HID Indala (Motorola Indala)
* Write to T55x7 compatible keycard/keyfob


### Optional
* Noralsy (KCP3000)
* Farpointe Pyramid
* Keri (KC-10X, MT-10X, PKT-10X)
* Kantech ioProx
* DoorKing (DKProx) [Not DKProx Long Range]
* AWID (Low frequency only – CS-AWID, GR-AWID, KT-AWID, PW-AWID)
 Keysy note: (Emulation not supported, can still clone to rewritable)
* Radio Key (SecuraKey RKKT-01, RKKT-02)
* Viking
* Visa2000
* Schlage IBF iButton (RFID portion only)

### Exotic
* Pet tags

## Features

### Reading
* Read specific card type, save ID
* card type detection (testing all protocols one by one, and read ID if found) 

### Emulation
* Emulate saved cards
* Enter card ID and card type manually to saved ID library
_You can also emulate the card by entering its ID manually, so you can easily send it to your friend in a text format. Thus, Flipper owners can exchange card dumps with each other remotely without ever touching a physical card._

### Write card
* Write to T55x7 compatible keycard/keyfob


## Links

* https://scanlime.org/2008/09/using-an-avr-as-an-rfid-tag/
* [ESP8266 em4100 emulator](https://github.com/Crypter/ESP-RFID)
* https://www.kickstarter.com/projects/1708444109/rfidler-a-software-defined-rfid-reader-writer-emul
* https://github.com/AlexMalov/EasyKeyDublicatorRFID
* https://shop.hak5.org/products/keysy

# UI

## Main menu

* Read
* Saved Keys
* Enter manually