To communicate with the real world systems, Flipper Zero has a built-in radio module based on TI CC1101 chip. It supports both transmitting and receiving digital signals within the 300-928 MHz frequency range. This is the operating range for a wide class of devices and access control systems such as garage doors remotes, boom barriers, IoT sensors, and remote keyless systems.

Out of the box, Flipper Zero can emulate remotes for popular garage doors and barriers. You can keep hundreds of remotes in Flipper's memory as well as create a blank remote for the new wireless gate. Just select the right brand of the system in the Flipper menu, register a new key in your garage/barrier receiver, and give it a unique name for easy navigation between your remotes.

CC1101 is well known universal transceiver designed for low-power wireless applications. And with a ready-to-use open-source library, developers can interact with the radio subsystem without limitations. You can write any wireless application, like custom protocol or decoder, as well as use it for connecting with IoT devices and access systems.


## Frequency Scaner (Spectrum analyzer)

User should be able to detect which frequency used in specific device. For example detect if unknown remote transmit on 433MHz or 868MHz

![frequency scan example](https://github.com/Flipper-Zero/wiki/raw/master/images/rf_scan.png)

## Signal Analyzer

Detect frequency and modulation:

* ASK
* FSK

## Protocol Decoder

* Works on 315/433/868 MHz

Flipper Zero has an integrated decoder for popular remote control algorithms such as Keeloq and others, so you can analyze an unknown radio system to figure out the protocol under the hood.

## Signal recorder

Furthermore, Flipper can record the samples of radio signals to analyze it later with more sophisticated tools on the computer, as well as replay the saved samples. Many remotes and IoT devices such as doorbells, sensors, and radio sockets don't use any encryption at all — in this case, Flipper can replay the signal, even if the protocol wasn't recognized.


### Protocol Decoder demo

https://www.youtube.com/watch?v=AeCGLFKsxCU
* Detect protocol and parse it
* Analyze if it's secure on not


## Common Dummy Remote

User can use flipper as normal dummy remote for their own reciever like garage door.  
For this flipper should have library of popular remotes brands.

# UI

## Main menu

* Sniffer
* Saved