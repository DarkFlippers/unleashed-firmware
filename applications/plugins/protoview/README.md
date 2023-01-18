ProtoView is a digital signal detection and visualization tool for the
[Flipper Zero](https://flipperzero.one/). The Flipper is able to identify
a great deal of RF protocols, however when the exact protocol is not
implemented (and there are many proprietary ones, such as the ones of
the car keys), the curious person is left wondering what the device is
sending at all. Using ProtoView she or he can visualize the high and low pulses
like in the example image below (showing a Volkswagen key in 2FSK):

![ProtoView screenshot raw signal](/images/protoview_1.jpg)

This is often enough to make an initial idea about the encoding used
and if the selected modulation is correct.

Other than that, ProtoView is able to decode a few interesting protocols:

* TPMS sensors: Renault, Toyota, Schrader, Citroen, Ford.
* Microchip HSC200/300/301 Keeloq protocol.
* Oregon thermometer protocol 2.
* PT2262, SC5262 based remotes.
* ... more will be implemented soon, hopefully. Send PRs :)

![ProtoView screenshot Renault TPMS data](/images/protoview_2.jpg)

The app implements a framework that makes adding and experimenting with new
protocols very simple. Check the `protocols` directory to see how the
API works.

The secondary goal of ProtoView is to provide a somewhat-documented application
for the Flipper (even if ProtoView is a pretty atypical application: doesn't make use of the standard widgets and other abstractions provded by the framework).
Many apps dealing with the *subghz subsystem* (the Flipper
abstraction to work with the [CC1101 chip](https://www.ti.com/product/CC1101))
tend to be complicated and completely undocumented. This is unfortunately
true for the firmware of the device itself. It's a shame because especially
in the case of code that talks with hardware peripherals there are tons
of assumptions and hard-gained lessons that can [only be captured by comments and are in the code only implicitly](http://antirez.com/news/124).

However, the Flipper firmware source code is well written even if it
lacks comments and documentation, so it is possible to make some ideas of
how things work just grepping inside.

# Detection algorithm

In order to show unknown signals, the application attempts to understand if
the samples obtained by the Flipper API (a series of pulses that are high
or low, and with different duration in microseconds) look like belonging to
a legitimate signal, and aren't just noise.

We can't make assumptions about
the encoding and the data rate of the communication, so we use a simple
but relatively effective algorithm. As we check the signal, we try to detect
long parts of it that are composed of pulses roughly classifiable into
a maximum of three different classes of lengths, plus or minus 10%. Most
encodings are somewhat self-clocked, so they tend to have just two or
three classes of pulse lengths.

However often pulses of the same theoretical
length have slightly different lengths in the case of high and low level
(RF on or off), so we classify them separately for robustness.

# Usage

The application shows the longest coherent signal detected so far.

* The OK button resets the current signal.
* The UP and DOWN buttons change the scale. Default is 100us per pixel.
* The LEFT and RIGHT buttons switch to settings.

Under the detected sequence, you will see a small triangle marking a
specific sample. This mark means that the sequence looked coherent up
to that point, and starting from there it could be just noise.

If the protocol is decoded, the bottom-left corner of the screen
will show the name of the protocol, and going in the next screen
with the right arrow will show information about the decoded signal.

In the bottom-right corner the application displays an amount of time
in microseconds. This is the average length of the shortest pulse length
detected among the three classes. Usually the *data rate* of the protocol
is something like `1000000/this-number*2`, but it depends on the encoding
and could actually be `1000000/this-number*N` with `N > 2` (here 1000000
is the number of microseconds in one second, and N is the number of clock
cycles needed to represent a bit).

Things to investigate:

* Many cheap remotes (gate openers, remotes, ...) are on the 433.92Mhz or nearby and use OOK modulation.
* Weather stations are often too in the 433.92Mhz OOK.
* For car keys, try 433.92 OOK650 and 868.35 Mhz in OOK or 2FSK.
* For TPMS try 433.92 in TPMS modulation (FSK optimized for these signals).

# Installing the app from source

* Download the Flipper Zero dev kit and build it:
```
mkdir -p ~/flipperZero/official/
cd ~/flipperZero/official/
git clone --recursive  https://github.com/flipperdevices/flipperzero-firmware.git  ./
./fbt
```
* Copy this application folder in `official/applications_user`.
* Connect your Flipper via USB.
* Build and install with: `./fbt launch_app APPSRC=protoview`.

# Installing the binary file (no build needed)

Drop the `protoview.fap` file you can find in the `binaries` folder into the
following Flipper Zero location:

    /ext/apps/Tools

The `ext` part means that we are in the SD card. So if you don't want
to use the Android (or other) application to upload the file,
you can just take out the SD card, insert it in your computer,
copy the fine into `apps/Tools`, and that's it.

# License

The code is released under the BSD license.

# Disclaimer

This application is only provided as an educational tool. The author is not liable in case the application is used to reverse engineer protocols protected by IP or for any other illegal purpose.

# Credits

A big thank you to the RTL433 author, [Benjamin Larsson](https://github.com/merbanan). I used the code and tools he developed in many ways:
* To capture TPMS data with rtl433 and save to a file, to later play the IQ files and speedup the development.
* As a sourve of documentation for protocols.
* As an awesome way to visualize and understand protocols, via [these great web tools](https://triq.org/).
* To have tons of fun with RTLSDR in general, now and in the past.
