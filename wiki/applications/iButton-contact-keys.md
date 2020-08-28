Flipper Zero has a built-in 1-Wire pad to read iButton (DS1990A) keys, also known as TouchMemory or Dallas keys. This technology is quite old but still widely used around the world. It's based on 1-Wire protocol and doesn't have any authentication, so Flipper can easily read these keys, save IDs into the memory, write IDs to blank keys, and emulate the key itself.

* **Reading & Writing & Emulating**
* **Protocol detection**: юзер может определить тип ключа поднося ключ к флипперу и тип считывателя поднося флиппер к домофону (цифрал, даллас и т.д.)

<img width="300" src="https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/ibutton/P1020756.jpg" />

## Reading
`iButton(1-Wire) -> Reading`

Flipper will wait for iButton tag.  While waiting red LED is blinking.  
Lean tag on iButton reader in the back side of Flipper Zero:

<img width="300" src="https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/ibutton/read1.jpeg" />

# UI

## Main menu

* Read
* Saved Keys
* Enter manually
