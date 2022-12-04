**BIG CHANGES COMING TO THIS! Need to update info as the below is no longer (entirely) true.**

Some files that are fun to use. Starting to get a better idea on the structure (Flipper format) of NFC files and payloads.

----------------------------------------------

**Flipper Zero can read the following**:
- T5777 Card Freq 125K
- 125 kHz RFID (MiFare Classic Card)
- NFC MiFare Classic Clear Tag
- NFC HID Card Freq 125KHZ
- 125 kHz RFID FourPoints.Com Card NFC
- NFC Mifare Ultral/NTAG Gen1A 1k S50 HF RFID Card
- NFC UID Freq 13.56MHZ Card | NFC Hotel KeyCards
- NFC ValuProx 125kHz ISO

----------------------------------------------

![Block_0_Explained](https://user-images.githubusercontent.com/57457139/182469731-5b5f2ad0-8ebc-4418-9375-ab97f19a3aeb.png)

----------------------------------------------

Example using the [RickRoll.nfc](https://github.com/UberGuidoZ/Flipper/blob/main/NFC/Fun_Files/RickRoll.nfc) file. Leave the data in pages 1 through 6 alone (though not always true, but it is for YouTube links.)

```
 Page 7: 79 6F 75 74
 Page 8: 75 2E 62 65
 Page 9: 2F 64 51 77
Page 10: 34 77 39 57
Page 11: 67 58 63 51
```

This is simply the YouTube "[share](https://support.google.com/youtube/answer/57741)" link encoded into HEX and split into 4 byte chunks. See for yourself...

HEX from above: `79 6F 75 74 75 2E 62 65 2F 64 51 77 34 77 39 57 67 58 63 51` <br>
[Converted](https://www.binaryhexconverter.com/hex-to-ascii-text-converter): youtu.be/dQw4w9WgXcQ

The last byte in page 6 (0x04) defines what type of encoding ([data sheet](https://www.nxp.com/docs/en/data-sheet/NTAG213_215_216.pdf) and [URI identifier codes](https://learn.adafruit.com/adafruit-pn532-rfid-nfc/ndef)). To convert your link to HEX, use anything such as an [online tool](https://onlinehextools.com/convert-ascii-to-hex). Read up on [generic cloner passwords](https://github.com/RfidResearchGroup/proxmark3/blob/master/doc/cloner_notes.md) as they may exist and may be needed to access information.

One limitation is the URL will be truncated if it goes beyond page 11! If your URL is less than exact, pad it with 00, making sure page 12 stays as "FE 00 00 00". Note that [TinyURL](https://tinyurl.com/app) links appear to fit well and conversion/use is free. If your link doesn't launch automatically when scanned, try using a different URI identifier.

----------------------------------------------

[NTAG Differences](https://www.rfidfuture.com/difference-between-ntag213-ntag215-and-ntag216.html) | [NTAG Datasheet](https://www.nxp.com/docs/en/data-sheet/NTAG213_215_216.pdf) | [ASCII to HEX](https://onlinehextools.com/convert-ascii-to-hex) | [HEX to Decimal](https://www.binaryhexconverter.com/hex-to-decimal-converter) (Other converters at both links.)

![NFC_Mem_Opt](https://user-images.githubusercontent.com/57457139/168696250-31ce8633-54cb-4ed7-96fb-f71723f686dd.png)

Value&nbsp;&nbsp;&nbsp;&nbsp;Protocol &nbsp;&nbsp;&nbsp;&nbsp;([SOURCE](https://learn.adafruit.com/adafruit-pn532-rfid-nfc/ndef))<br>
------&nbsp;&nbsp;&nbsp;&nbsp;---------- <br>
0x00&nbsp;&nbsp;&nbsp;&nbsp; No prepending is done ... the entire URI is contained in the URI Field <br>
0x01&nbsp;&nbsp;&nbsp;&nbsp; `http://www.` <br>
0x02&nbsp;&nbsp;&nbsp;&nbsp; `https://www.` <br>
0x03&nbsp;&nbsp;&nbsp;&nbsp; `http://` <br>
0x04&nbsp;&nbsp;&nbsp;&nbsp; `https://` <br>
0x05&nbsp;&nbsp;&nbsp;&nbsp; `tel:` <br>
0x06&nbsp;&nbsp;&nbsp;&nbsp; `mailto:` <br>
0x07&nbsp;&nbsp;&nbsp;&nbsp; `ftp://anonymous:anonymous@` <br>
0x08&nbsp;&nbsp;&nbsp;&nbsp; `ftp://ftp.` <br>
0x09&nbsp;&nbsp;&nbsp;&nbsp; `ftps://` <br>
0x0A&nbsp;&nbsp;&nbsp;&nbsp; `sftp://` <br>
0x0B&nbsp;&nbsp;&nbsp;&nbsp; `smb://` <br>
0x0C&nbsp;&nbsp;&nbsp;&nbsp; `nfs://` <br>
0x0D&nbsp;&nbsp;&nbsp;&nbsp; `ftp://` <br>
0x0E&nbsp;&nbsp;&nbsp;&nbsp; `dav://` <br>
0x0F&nbsp;&nbsp;&nbsp;&nbsp; `news:` <br>
0x10&nbsp;&nbsp;&nbsp;&nbsp; `telnet://` <br>
0x11&nbsp;&nbsp;&nbsp;&nbsp; `imap:` <br>
0x12&nbsp;&nbsp;&nbsp;&nbsp; `rtsp://` <br>
0x13&nbsp;&nbsp;&nbsp;&nbsp; `urn:` <br>
0x14&nbsp;&nbsp;&nbsp;&nbsp; `pop:` <br>
0x15&nbsp;&nbsp;&nbsp;&nbsp; `sip:` <br>
0x16&nbsp;&nbsp;&nbsp;&nbsp; `sips:` <br>
0x17&nbsp;&nbsp;&nbsp;&nbsp; `tftp:` <br>
0x18&nbsp;&nbsp;&nbsp;&nbsp; `btspp://` <br>
0x19&nbsp;&nbsp;&nbsp;&nbsp; `btl2cap://` <br>
0x1A&nbsp;&nbsp;&nbsp;&nbsp; `btgoep://` <br>
0x1B&nbsp;&nbsp;&nbsp;&nbsp; `tcpobex://` <br>
0x1C&nbsp;&nbsp;&nbsp;&nbsp; `irdaobex://` <br>
0x1D&nbsp;&nbsp;&nbsp;&nbsp; `file://` <br>
0x1E&nbsp;&nbsp;&nbsp;&nbsp; `urn:epc:id:` <br>
0x1F&nbsp;&nbsp;&nbsp;&nbsp; `urn:epc:tag:` <br>
0x20&nbsp;&nbsp;&nbsp;&nbsp; `urn:epc:pat:` <br>
0x21&nbsp;&nbsp;&nbsp;&nbsp; `urn:epc:raw:` <br>
0x22&nbsp;&nbsp;&nbsp;&nbsp; `urn:epc:` <br>
0x23&nbsp;&nbsp;&nbsp;&nbsp; `urn:nfc:` <br>

-----

Acknowledgements: [RogueMaster](https://github.com/RogueMaster/) | cyanic | Null Silvry | [Equip](https://github.com/equipter/) | DDVL (for discussions, testing, and any files.)

-----

I've had so many asking for me to add this.<br>
![Flipper_Blush](https://user-images.githubusercontent.com/57457139/183561666-4424a3cc-679b-4016-a368-24f7e7ad0a88.jpg) ![Flipper_Love](https://user-images.githubusercontent.com/57457139/183561692-381d37bd-264f-4c88-8877-e58d60d9be6e.jpg)

**BTC**: `3AWgaL3FxquakP15ZVDxr8q8xVTc5Q75dS`<br>
**BCH**: `17nWCvf2YPMZ3F3H1seX8T149Z9E3BMKXk`<br>
**ETH**: `0x0f0003fCB0bD9355Ad7B124c30b9F3D860D5E191`<br>
**LTC**: `M8Ujk52U27bkm1ksiWUyteL8b3rRQVMke2`

So, here it is. All donations of *any* size are humbly appreciated.<br>
![Flipper_Clap](https://user-images.githubusercontent.com/57457139/183561789-2e853ede-8ef7-41e8-a67c-716225177e5d.jpg) ![Flipper_OMG](https://user-images.githubusercontent.com/57457139/183561787-e21bdc1e-b316-4e67-b327-5129503d0313.jpg)

Donations will be used for hardware (and maybe caffeine) to further testing!<br>
![UberGuidoZ](https://cdn.discordapp.com/emojis/1000632669622767686.gif)
