## Supported card types

Flipper Zero can read and emulate: 

* ISO-14443A/B
* Mifare Classic 1k/4k
* Mifare Ultralight
* NFC Forum protocols (NDEF)


## Card detector

![](./../../wiki_static/applications/NFC/nfc-card-detector.png)

Card type is often unknown. Card detector runs tests against the unknown card to determine it's type. These tests are not 100% accurate, but they help to start exploring. If the test finished successfully, it can recommend to run a suitable application for the card type.

### Card detector routine:

1. Check if card `ISO-14443` `A` or `B` or `FeliCa`
2. Check the 6 byte of `SAK` to determine if `ISO-14443-4` compliant
3. Match combination of UID, SAK, ATQA, ATS, ATR from database
4. Try to authenticate as Mifare, EMV, etc..
5. Return founded type and suggest suitable application or return error

## Reader detector

![](./../../wiki_static/applications/NFC/nfc-wall-reader.png)

Wall readers usually looks the same, but may accept various types type of cards. With reader detector feature we can emulate dummy card on Flipper and sniff commands that reader send to card.  


### Reader detector

1. Silently read WUPA (0x52) or REQA (0x26) without triggering SELECT on reader
2. Emulate dummy card
    2.1 Answer on ATQA
    2.2 Answer on SELECT 
    2.3 ...
3. Consistently emulate popular cards before valid authentication from reader found 

## Reader mode

* Mifare classic reader  _How to select dictionary?_
  * Brute force
  * Save dump
  * Write dump to SD-card
* Mifare Ultralight reader
  * Save dump
  * Write dump?
* EMV reader
* Simple UID reader

## Write/Emulate mode

Saved dumps stored on SD-card and accessible from `Saved dumps` menu. All saved dumps can be emulated or written to card if possible. User can view dump info and choose the action: 

* Emulate
* Write
* Edit? _(Editing big dumps can be can be difficult)_ 

# USB NFC Reader [Not implemented]
![](./../../wiki_static/applications/NFC/usb-nfc-reader.png)

There are many use cases that impossible to run directly on Flipper Zero. Most of these cases require powerful CPU for cryptographic attacks:  

- Mifare classic attacks: `mfoc` (Nested), `mfcuk` (Dark Side)
- Mifare Plus attack: Hard Nested

We can use Flipper Zero as a regular USB NFC adapter along with `LibNFC` library, so all existing software will work out of the box without any modifications. This mode must be run from menu `NFC -> USB NFC adapter`. In this mode all commands from PC should be forwarded directly to NFC chip `ST25R3916` via USB serial interface. 

<details>
  <summary>Chat log with <a href="https://github.com/doegox">Philippe @doegox Teuwen</a> about LibNFC driver for ST25R3916</summary>

> [Pavel Zhovner]: About ST chip support in libnfc:  I understand that libnfc is tightly tied to PNxxx and I can't fully imagine how big this work will be :slight_smile:  Our main goal is to keep compatibility for user space applications like mfterm, mfoc and so on. I don't know much about Libnfc userspace API, and in my imagination, we just need to write a low level driver for ST25R3916 and the rest will work out of the box, maybe I'm wrong. Here how I imagine this. We already start to implementing commands forwarding daemon.  

![](../../wiki_static/applications/NFC/libnfc_proxy_scheme.png)

> [doegox]: ther are intermediate APIs within libnfc, as I said yesterday maybe not super well layered
with directories buses/chips/drivers
and struct like this:

```
const struct nfc_driver pn53x_usb_driver = {
  .name                             = PN53X_USB_DRIVER_NAME,
  .scan_type                        = NOT_INTRUSIVE,
  .scan                             = pn53x_usb_scan,
  .open                             = pn53x_usb_open,
  .close                            = pn53x_usb_close,
  .strerror                         = pn53x_strerror,
      
  .initiator_init                   = pn53x_initiator_init,
  .initiator_init_secure_element    = NULL, // No secure-element support
  .initiator_select_passive_target  = pn53x_initiator_select_passive_target,
  .initiator_poll_target            = pn53x_initiator_poll_target,
  .initiator_select_dep_target      = pn53x_initiator_select_dep_target,
  .initiator_deselect_target        = pn53x_initiator_deselect_target,
  .initiator_transceive_bytes       = pn53x_initiator_transceive_bytes,
  .initiator_transceive_bits        = pn53x_initiator_transceive_bits,
  .initiator_transceive_bytes_timed = pn53x_initiator_transceive_bytes_timed,
  .initiator_transceive_bits_timed  = pn53x_initiator_transceive_bits_timed,
  .initiator_target_is_present      = pn53x_initiator_target_is_present,

  .target_init           = pn53x_target_init,
  .target_send_bytes     = pn53x_target_send_bytes,
  .target_receive_bytes  = pn53x_target_receive_bytes,
  .target_send_bits      = pn53x_target_send_bits,
  .target_receive_bits   = pn53x_target_receive_bits,

  .device_set_property_bool     = pn53x_usb_set_property_bool,
  .device_set_property_int      = pn53x_set_property_int,
  .get_supported_modulation     = pn53x_usb_get_supported_modulation,
  .get_supported_baud_rate      = pn53x_get_supported_baud_rate,
  .device_get_information_about = pn53x_get_information_about,

  .abort_command  = pn53x_usb_abort_command,
  .idle           = pn53x_idle,
  .powerdown      = pn53x_PowerDown,
};
```

> [doegox]: so if you can write ST equivalents to these pn53x_*, and map them to the generic names of the left column, that should work :) in a new driver
</details>




# Schematic

![](./../../wiki_static/applications/NFC/ST25R3916-schematic.png)


# Menu
![](./../../wiki_static/applications/NFC/nfc-ui.png)

<!--- Menu structure -->
# NFC

## Detect card

- Activating RF field and wait for card. 
Run tests when card found.
- Found match
  - Mifare Classic found.
Run Mifare reader? 
- No found
  - Cannot detect card type  

## Reader detector

- Passive listen for WUPA/REQA-B
  - Commands live stream 
    0x26
    0x26
    0x26
- Start active emulating 
- .
  - Mifare reader found
- .
  - EMV reader found

## Read card

- Mifare Classic
- Mifare Ultralight
- EMV get PAN
- what more?

## Saved dumps

- mfc_a6b804bf
- home
- mfu_04bfac72
- emv_paywave_1007

## USB NFC Reader

- Activates instantly.
No more display updates
<!--- Menu structure end -->
