# Infrared Captures

**Credits go to @gsurkov, @skotopes, @knrn-ai, @DrZlo13 and @ahumeniy for making and contributing to the original `UniversalRemotes.md` Documentation located [Here](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/documentation/UniversalRemotes.md).** 

**slightly adapted by @amec0e**

## Televisions, Fans, Audio and Projectors

Each signal is recorded using the following process:

1. Get the remote and point it to Flipper's IR receiver.
2. Start learning a new remote if it's the first button or press `+` to add a new button otherwise.
3. Do a Quick Press of a remote button and save it under a corresponding name. **(NOTE: Don't hold the remote button down, this will result in long captures and long playbacks ultimately slowing down the universal remotes performance)**
4. Repeat steps 2-3 until all required signals are saved.

The signal names are self-explanatory. Remember to make sure that every recorded signal does what it's supposed to.

**NOTE:** It's possible some devices around you will cause interference and may force your capture into raw data instead of a parsed code. 
If you notice you get a parsed code when capturing it's best to click "Retry" a few times on the flipper when capturing to ensure the device is not suffering from any interference, and that the cleanest capture is possible.

## Types of data

**Parsed data**

This is the cleanest type of data because it means it is a recognised code.

```
name: EXAMPLE
type: parsed
protocol: NEC
address: 07 00 00 00
command: 02 00 00 00
```

**Raw Data**

With raw data its important not to hold the remotes button down when capturing on your flipper as this increases not only the size of the capture but the repeats and also how long it takes to send the signal back. Below is an ideal button capture.

```
# 
name: EXAMPLE
type: raw
frequency: 38000
duty_cycle: 0.330000
data: 2410 597 1189 599 592 600 1186 602 589 603 1183 606 595 597 593 598 1208 605 596 596 594 597 593 599 592 25604 2403 604 1182 606 595 597 1189 599 591 601 1185 603 618 573 617 575 1211 602 588 603 588 605 596 596 594 25605 2402 604 1192 596 594 597 1189 599 592 601 1185 628 593 598 593 600 1186 602 589 603 588 604 597 595 596
```

**Capturing Raw Data:**

If you are sure your remote is using raw data the best way to capture it will be to do a quick button press **(don't hold the remotes button down)** and look at how many samples you get, the general idea here is to get the lowest amount of raw data samples captured (around 100 samples is about right) while making sure that the playback on the device is still working. This is usually accomplished by doing a quick button press on the remote when capturing.

## Air Conditioners

Air conditioners differ from most other infrared-controlled devices because their state is tracked by the remote.
The majority of A/C remotes have a small display that shows the current mode, temperature, and other settings.
When the user presses a button, a whole set of parameters is transmitted to the device, which must be recorded and used as a whole.

In order to capture a particular air conditioner, there is a particular process require to capturing and this is done using the following process:

1. Get the remote and press the **Power Button** so that the display shows that A/C is ON.
2. Set the A/C to the corresponding mode (see table below), leaving other parameters such as fan speed or vane on **AUTO** (if applicable).
3. Press the **POWER** button to switch the A/C off.
4. Start learning a new remote on Flipper if it's the first button or press `+` to add a new button otherwise.
5. Point the remote to Flipper's IR receiver as directed and press **POWER** button once again.
6. Save the resulting signal under the specified name.
7. Repeat steps 2-6 for each signal from the table below.

| Signal  |    Mode    | Temperature | Note                                |
| :-----: | :--------: | :---------: | ----------------------------------- |
|   Dh    | Dehumidify |     N/A     |                                     |
| Cool_hi |  Cooling   |  See note   | Lowest temperature in cooling mode  |
| Cool_lo |  Cooling   |    23°C     |                                     |
| Heat_hi |  Heating   |  See note   | Highest temperature in heating mode |
| Heat_lo |  Heating   |    23°C     |                                     |

Finally, record the `Off` signal:

1. Make sure the display shows that the A/C is ON.
2. Start learning a new signal on Flipper and point the remote towards the IR receiver.
3. Press the **POWER** button so that the remote shows the OFF state.
4. Save the resulting signal under the name `Off`.

Test the file against the actual device. Make sure that every signal does what it's supposed to.
