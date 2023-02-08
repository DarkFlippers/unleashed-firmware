# Universal Remotes

## Televisions

Adding your TV set to the universal remote is quite straightforward. Up to 6 signals can be recorded: `Power`, `Mute`, `Vol_up`, `Vol_dn`, `Ch_next`, and `Ch_prev`. Any of them can be omitted if not supported by your TV.

Each signal is recorded using the following algorithm:

1. Get the remote and point it to Flipper's IR receiver.
2. Start learning a new remote if it's the first button or press `+` to add a new button otherwise.
3. Press a remote button and save it under a corresponding name.
4. Repeat steps 2-3 until all required signals are saved.

The signal names are self-explanatory. Remember to make sure that every recorded signal does what it's supposed to.

If everything checks out, append these signals **to the end** of the [TV universal remote file](/assets/resources/infrared/assets/tv.ir).

## Audio players

Adding your audio player to the universal remote is done in the same manner as described above. Up to 8 signals can be recorded: `Power`, `Play`, `Pause`, `Vol_up`, `Vol_dn`, `Next`, `Prev`, and `Mute`. Any of them can be omitted if not supported by the player.

The signal names are self-explanatory.
On many remotes, the `Play` button doubles as `Pause`. In this case, record it as `Play` omitting the `Pause`.
Make sure that every signal does what it's supposed to.

If everything checks out, append these signals **to the end** of the [audio player universal remote file](/assets/resources/infrared/assets/audio.ir).

## Projectors

Adding your projector to the universal remote is really simple. Up to 4 signals can be recorded: `Power`, `Mute`, `Vol_up`, `Vol_dn`. Any of them can be omitted if not supported by your projector.
To save time, please make sure every recording has been named accordingly.
In case of omitting, on most projectors with the 4 following buttons, you should not have a problem.


## Air conditioners

Air conditioners differ from most other infrared-controlled devices because their state is tracked by the remote.
The majority of A/C remotes have a small display that shows the current mode, temperature, and other settings.
When the user presses a button, a whole set of parameters is transmitted to the device, which must be recorded and used as a whole.

In order to add a particular air conditioner to the universal remote, 6 signals must be recorded: `Off`, `Dh`, `Cool_hi`, `Cool_lo`, `Heat_hi`, and `Heat_lo`.
Each signal (except `Off`) is recorded using the following algorithm:

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

The resulting remote file should now contain 6 signals. You can omit any of them, but you then won't be able to use their functionality.
Test the file against the actual device. Make sure that every signal does what it's supposed to.

If everything checks out, append these signals **to the end** of the [A/C universal remote file](/assets/resources/infrared/assets/ac.ir).

## Final steps

The order of signals is not important, but they should be preceded by the following comment: `# Model: <Your model name>` in order to keep the library organized.

When done, open a pull request containing the changed file.
