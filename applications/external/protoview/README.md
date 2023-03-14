ProtoView is a digital signal detection, visualization, editing and reply tool for the [Flipper Zero](https://flipperzero.one/). The Flipper default application, called Subghz, is able to identify certain RF protocols, but when the exact protocol is not implemented (and there are many undocumented and unimplemented ones, such as the ones in use in TPMS systems, car keys and many others), the curious person is left wondering what the device is sending at all. Using ProtoView she or he can visualize the high and low pulses like in the example image below (showing a TPMS signal produced by a Renault tire):

![ProtoView screenshot raw signal](/images/protoview_1.jpg)

This is often enough to make an initial idea about the encoding used
and if the selected modulation is correct. For example, in the signal above
you can see a set of regular pulses and gaps used for synchronization, and then
a sequence of bits encoded in [Manchester](https://en.wikipedia.org/wiki/Manchester_code) line code. If you study these things for five minutes, you'll find yourself able to decode the bits with naked eyes.

## Decoding capabilities

Other than showing the raw signal, ProtoView is able to decode a few interesting protocols:

* TPMS sensors: Renault, Toyota, Schrader, Citroen, Ford.
* Microchip HSC200/300/301 Keeloq protocol.
* Oregon thermometer protocol 2.
* PT2262, SC5262 based remotes.
* ... more will be implemented soon, hopefully. Send PRs :)

![ProtoView screenshot Renault TPMS data](/images/protoview_2.jpg)

The app implements a framework that makes adding and experimenting with new
protocols very simple. Check the `protocols` directory to see how the
API works, or read the full documentation at the end of this `README` file.
The gist of it is that the decoder receives the signal already converted into
a bitmap, where each bit represents a short pulse duration. Then there are
functions to seek specific sync/preamble sequences inside the bitmap, to decode
from different line codes, to compute checksums and so forth.

## Signals transmission capabilities

Once ProtoView decodes a given message, it is able to *resample* it
in pulses and gaps of the theoretical duration, and send the signal again
via the Flipper TX capabilities. The captured signal can be sent
to different frequencies and modulations than the ones it was captured
from.

For selected protocols, that implement the message creation methods,
ProtoView is also able to edit the message received, modify fields,
and finally re-detect the new produced signal and resend it. Signals
can also be produced from scratch, by setting all the fields to appropriate
values.

## A well-documented app for the Flipper

The secondary goal of ProtoView is to provide a well-documented application for the Flipper (even if ProtoView is a pretty atypical application: it doesn't make use of the standard widgets and other abstractions provided by the framework).
Most apps dealing with the *subghz subsystem* of the Flipper (the abstraction used to work with the [CC1101 chip](https://www.ti.com/product/CC1101)) tend to be complicated and completely undocumented.
Unfortunately, this is also true for the firmware of the device.
It's a shame, because especially in the case of code that talks with hardware peripherals there are tons of assumptions and hard-gained lessons that can [only be captured by comments and are in the code only implicitly](http://antirez.com/news/124).

However, the Flipper firmware source code is well written even if it
lacks comments and documentation (and sometimes makes use of abstractions more convoluted than needed), so it is possible to make some ideas of how things work just grepping inside. In order to develop this application, I ended reading most parts of the firmware of the device.

## Detection algorithm

In order to detect and show unknown signals, the application attempts to understand if the samples obtained by the Flipper API (a series of pulses that are high
or low, and with different duration in microseconds) look like belonging to
a legitimate signal, and aren't just noise.

We can't make assumptions about
the encoding and the data rate of the communication, so we use a simple
but relatively effective algorithm. As we check the signal, we try to detect
long parts of it that are composed of pulses roughly classifiable into
a maximum of three different duration classes, plus or minus a given percentage.
Most encodings are somewhat self-clocked, so they tend to have just two or
three classes of pulse lengths.

However, often, pulses of the same theoretical
length have slightly different lengths in the case of high and low level
(RF on or off), so the detector classifies them separately for robustness.

Once the raw signal is detected, the registered protocol decoders are called
against it, in the hope some of the decoders will make sense of the signal.

# Usage

In the main screen, the application shows the longest coherent signal detected so far. The user can switch to other views pressing the LEFT and RIGHT keys. The BACK key will return back to the main screen. Long pressing BACK will quit the application.

## Main raw signal screen

* A long press of the OK button resets the current signal, in order to capture a new one.
* The UP and DOWN buttons change the scale. Default is 100us per pixel, but it will be adapted to the signal just captured.
* A long press of the LEFT and RIGHT keys will pan the signal, to see what was transmitted before/after the current shown range.
* A short press to OK will recenter the signal and set the scale back to the default for the specific pulse duration detected.

Under the detected sequence, you will see a small triangle marking a
specific sample. This mark means that the sequence looked coherent up
to that point, and starting from there it could be just noise. However the
signal decoders will not get just up to this point, but will get more:
sometimes the low level detector can't make sense of a signal that the
protocol-specific decoder can understand fully.

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

## Info screen

If a signal was detected, the info view will show the details about the signal. If the signal has more data that a single screen can fit, pressing OK will show the next fields. Pressing DOWN will go to a sub-view with an oscilloscope-alike representation of the signal, from there you can:

1. Resend the signal, by pressing OK.
2. Save the signal as `.sub` file, by long pressing OK.

When resending, you can select a different frequency and modulation if you
wish.

## Frequency and modulation screen

In this view you can just set the frequency and modulation you want to use.
There are special modulations for TPMS signals: they need an higher data
rate.

* Many cheap remotes (gate openers, remotes, ...) are on the 433.92Mhz or nearby and use OOK modulation.
* Weather stations are often too in the 433.92Mhz OOK.
* For car keys, try 433.92 OOK650 and 868.35 Mhz in OOK or 2FSK.
* For TPMS try 433.92 in TPMS1 or TPMS2 modulations (FSK and OOK custom modulations optimized for these signals, that have a relatively high data rate).

## Signal creator

In this view, you can do two things:

1. Select one of the protocols supporting message creation, and create a signal from scratch.
2. If there is already a detected signal, you can modify the signal.

This is how it works:

1. Select one of the protocols (the one of the currently detected signal will be already provided as default, if any, and if it supports message creation).
2. Fill the fields. Use LEFT and RIGHT to change the values of integers, or just press OK and enter the new value with the Fliper keyboard widget.
3. When you are done, long press OK to build the message. Then press BACK in order to see it.
4. Go to the INFO view, and then DOWN to the signal sending/saving subview in order to send or save it.

## Direct sampling screen

This final screen shows in real time the high and low level that the Flipper
RF chip, the CC1101, is receiving. This will makes very easy to understand
if a given frequency is targeted by something other than noise. This mode is
fun to watch, resembling an old CRT TV set.

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

# Protocols decoders API

Writing a protocol decoder is not hard, and requires to write three
different methods.

1. `decode()`. This is mandatory, and is used in order to turn a known signal into a set of fields containing certain informations. For instance for a thermometer sending data via RF, a raw message will be decoded into fields like temperature, humidity, station ID and so forth.
2. `get_fields()`. Optional, only needed if the protocol supports creating and editing signals. This method just returns the fields names, types and defaults. The app will use this list in order to allow the user to set values. The populated fields will be passed to the `build_message()` method later.
3. `build_message()`. This method gets a set of fields representing the parameters of the protocol, as set by the user, and will create a low level signal composed of pulses and gaps of specific durations.

## `decode()` method

    bool decode(uint8_t *bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo *info);

The method gets a bitmap `bits` long `numbytes` bytes but actually containing `bumbits` valid bits. Each bit represents a pulse of gap of the duration of the shortest time detected in the protocol (this is often called *te*, in the RF protocols jargon). So, for instance, if a signal is composed of pulses and gaps of around 500 and 1000 microseconds, each bit in the bitmap will represent 500 microseconds.

Continuing with the example above, if the received signal was composed of a 1000 microseconds gap, then a 500 microsecond pulse, then a 500 microsecond gap and finally a 1000 microseconds pulse, its bitmap representation will be:

    001011

To access the bitmap, the decoder can use the following API:

    bool bitmap_get(uint8_t *b, uint32_t blen, uint32_t bitpos);

The `blen` parameter will be set to what the decode method gets
as `numbytes`, and is used to prevent overflows. This way if `bitpos`
is out of range, nothing bad happens.

There are function to match and seek specific patterns inside the signal:

    bool bitmap_match_bits(uint8_t *b, uint32_t blen, uint32_t bitpos, const char *bits);
    uint32_t bitmap_seek_bits(uint8_t *b, uint32_t blen, uint32_t startpos, uint32_t maxbits, const char *bits);

Finally, there are functions to convert from different line codes:

    uint32_t convert_from_line_code(uint8_t *buf, uint64_t buflen, uint8_t *bits, uint32_t len, uint32_t offset, const char *zero_pattern, const char *one_pattern);
    uint32_t convert_from_diff_manchester(uint8_t *buf, uint64_t buflen, uint8_t *bits, uint32_t len, uint32_t off, bool previous);

This method can also access the short pulse duration by inspecting the
`info->short_pulse_dur` field (in microseconds).

Please check the `b4b1.c` file for an easy to understand example of the decoder implementation.

If the decoder actually detected a message, it will return `true` and will return a set of fields, like thata:

    fieldset_add_bytes(info->fieldset,"id",d,5);
    fieldset_add_uint(info->fieldset,"button",d[2]&0xf,4);

## `get_fields()` method.

    static void get_fields(ProtoViewFieldSet *fieldset);

This method will be basically a copy of the final part of `decode()`, as
it also needs to return the set of fields this protocol is composed of.
However instead of returning the values of an actual decoded message, it
will just provide their default values for the signal creator view.

Note that the `build_message()` method is guaranteed to receive the
same exact fields in the same exact order.

## `build_message()` method.

    void build_message(RawSamplesBuffer *samples, ProtoViewFieldSet *fs);

This method is responsible of creating a signal from scratch, by
appending gaps and pulses of the specific duration into `samples`
using the following API:

    raw_samples_add(RawSamplesBuffer *samples, bool level, uint32_t duration);

Level can be true (pules) or false (gap). Duration is in microseconds.
The method receives a set of fields in `fs`. Each field is accessible
directly accessing `fs->fields[... field index ...]`, where the field
index is 0, 1, 2, ... in the same order as `get_fields()` returned the
fields.

For now, you can access the fields in the raw way, by getting the
values directly from the data structure representing each field:

```
typedef struct {
    ProtoViewFieldType type;
    uint32_t len;       // Depends on type:
                        // Bits for integers (signed,unsigned,binary,hex).
                        // Number of characters for strings.
                        // Number of nibbles for bytes (1 for each 4 bits).
                        // Number of digits after dot for floats.
    char *name;         // Field name.
    union {
        char *str;          // String type.
        int64_t value;      // Signed integer type.
        uint64_t uvalue;    // Unsigned integer type.
        uint8_t *bytes;     // Raw bytes type.
        float fvalue;       // Float type.
    };
} ProtoViewField;

```

However later the app will likely provide a set of macros to do it
in a more future-proof way.

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

The application icon was designed by Stefano Liuzzo.
