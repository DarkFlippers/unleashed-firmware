# Mifare Nested Attacks for Flipper Zero

Ported Nested attacks from Proxmark3 (Iceman fork)

This is not original Repo of this app! Please follow this link to find latest original source and support the author!
[Flipper (Mifare) Nested (by AloneLiberty)](https://github.com/AloneLiberty/FlipperNested)

## Currently supported attacks

 - nested attack
 - static nested attack
 - hard nested attack

## Warning

App is still in early development, so there may be bugs. Your Flipper Zero may randomly crash/froze. Please create issue if you find any bugs (one bug = one issue). In original repo! - https://github.com/AloneLiberty/FlipperNested

## Disclaimer

The app provided for personal use only. Developer does not take responsibility for any loss or damage caused by the misuse of this app. In addition, the app developer does not guarantee the performance or compatibility of the app with all tags, and cannot be held liable for any damage caused to your tags/Flipper Zero as a result of using the app. By using this app you confirm that the tag belongs to you, you have permission to preform the attack and you agree to hold the app developer harmless from any and all claims, damages, or losses that may arise from its use.

## I need **your** help!

To successfuly recover keys from nested attack we need to correctly predict PRNG value. But we have a problem with that. Due to lack of my knowlege of Flipper Zero NFC HAL, PRNG can jump by quite large values (not like Proxmark3). So app is trying to find a delay where PRNG can be predicted accurately enough. This is not the best option, because we have to try to recover a bunch of unnecessary keys, which takes a lot of time and RAM and also spend a lot of time on timings. I don't know how to fix it. 

UPD: Chameleon Ultra devs [faced same issue](https://youtu.be/_wfikmXNQzE?t=202). They seems to use same method: [nested.c](https://github.com/RfidResearchGroup/ChameleonUltra/blob/main/software/src/nested.c) (better know from the beginning of development...)

## How to use it?

Detailed guide: [EN](https://github.com/AloneLiberty/FlipperNested/wiki/Usage-guide), [RU](https://github.com/AloneLiberty/FlipperNested/wiki/%D0%93%D0%B0%D0%B9%D0%B4-%D0%BF%D0%BE-%D0%B8%D1%81%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D1%8E).

## FAQ

For frequently asked questions, please refer to the FAQ: [EN](https://github.com/AloneLiberty/FlipperNested/wiki/FAQ), [RU](https://github.com/AloneLiberty/FlipperNested/wiki/%D0%A7%D0%90%D0%92%D0%9E).

## Contacts

Find here: https://github.com/AloneLiberty/FlipperNested
