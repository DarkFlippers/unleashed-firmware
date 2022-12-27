# Changes for the User NFC Key Dictionary (Updated Aug 28, 2022)

Official firmware has introduced a user dict file option which will allow you to update firmware and not lose any added NFC keys! This is great, but requires a slight change in the way things were done previously. I've removed all the NFC keys that are in the original `mf_classic_dict.nfc` provided by Official firmware as of July 26, 2022 and converted [what's left over](https://github.com/UberGuidoZ/Flipper/blob/main/NFC/mf_classic_dict/ReadMe.md#older-method-combined-multiple-mf_classic_dictnfc-files) to the new `mf_classic_dict_user.nfc` [file here](https://github.com/UberGuidoZ/Flipper/blob/main/NFC/mf_classic_dict/mf_classic_dict_user.nfc).

-----

Unleashed / RogueMaster have already merged this into their default dict file so no need to grab it!

-----

To use, add the `mf_classic_dict_user.nfc` file from this repo into the SD Card -> nfc -> assets folder.

![User_dict_file](https://user-images.githubusercontent.com/57457139/181144818-2355e2ba-64f7-4004-9f15-3390c445baee.png)

No need to overwrite anything or rename anything. Do that and you're done! Now verify the new keys are recognized<br>
Head over to **Flipper -> NFC -> Extra Actions -> Mf Classic Keys** and you should see something like this:

![User_Keys](https://user-images.githubusercontent.com/57457139/181145228-bcafeea9-5b0e-40d3-8ff4-0b0adda788b8.png)

You can add more easily using the center button if you discover more!

## Older method (no longer maintained): Combined multiple mf_classic_dict.nfc files:
- [Official firmware](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/assets/resources/nfc/assets/mf_classic_dict.nfc)
- [Unleased firmware](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/assets/resources/nfc/assets/mf_classic_dict.nfc)
- [MuddledBox firmware](https://github.com/MuddledBox/flipperzero-firmware/blob/dev/assets/resources/nfc/assets/mf_classic_dict.nfc)
 
Also included all NFC keys from [Proxmark3 Iceman's dictionaries](https://github.com/RfidResearchGroup/proxmark3/tree/master/client/dictionaries) and [RFIDresearchgroup](https://github.com/RfidResearchGroup), as well as random others online, then removed all dupes. This has almost **tripled** the amount of verified keys and been much more successful in capturing all keys in multiple tests during reads.

To use, replace the existing file `mf_classic_dict.nfc` under SD Card -> nfc -> assets.<br>
You can rename the existing as a backup or download it again from the links above if ever needed.

![File_location](https://user-images.githubusercontent.com/57457139/170588136-dbe99587-a04f-4dfd-8032-5e84b929cf58.png)

NOTE: This has been merged into [Unleased firmware](https://github.com/Eng1n33r/flipperzero-firmware/commit/9e9b33445f0524d6d6ef4641cfc275731d0936c4) as well as [RogueMaster's firmware](https://github.com/RogueMaster/flipperzero-firmware-wPlugins). Also important...

![Bank_Cards](https://user-images.githubusercontent.com/57457139/170762420-3e496885-d167-401a-ba4e-517a08a75214.png)

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
