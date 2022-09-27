# Universal Remotes
## Air Conditioners
### Recording signals
Air conditioners differ from most other infrared-controlled devices because their state is tracked by the remote.
The majority of A/C remotes have a small display which shows current mode, temperature and other settings.
When the user presses a button, a whole set of parameters is transmitted to the device, which must be recorded and used as a whole.

In order to add a particular air conditioner to the universal remote, 6 signals must be recorded: `Off`, `Dh`, `Cool_hi`, `Cool_lo`, `Heat_hi`, `Heat_lo`.
Each signal (except `Off`) is recorded using the following algorithm:

1. Get the remote and press the **Power Button** so that the display shows that A/C is ON.
2. Set the A/C to the corresponding mode (see table below), while leaving other parameters such as fan speed or vane on **AUTO** (if applicable).
3. Press the **POWER** button to switch the A/C off.
4. Start learning a new remote on Flipper if it's the first button or press `+` to add a new button otherwise.
5. Point the remote to Flipper's IR receiver as directed and press **POWER** button once again.
6. Save the resulting signal under the specified name.
7. Repeat the steps 2-6 for each signal from the table below.

| Signal  | Mode       | Temperature | Note                                |
| :-----: | :--------: | :---------: | ----------------------------------- |
| Dh      | Dehumidify | N/A         | |
| Cool_hi | Cooling    | See note    | Lowest temperature in cooling mode  |
| Cool_lo | Cooling    | 23°C        | |
| Heat_hi | Heating    | See note    | Highest temperature in heating mode |
| Heat_lo | Heating    | 23°C        | |

Finally, record the `Off` signal:
1. Make sure the display shows that A/C is ON.
2. Start learning a new signal on Flipper and point the remote towards the IR receiver.
3. Press the **POWER** button so that the remote shows the OFF state.
4. Save the resulting signal under the name `Off`.

The resulting remote file should now contain 6 signals. Any of them can be omitted, but that will mean that this functionality will not be used.
Test the file against the actual device. Every signal must do what it's supposed to.

If everything checks out, append these signals **to the end** of the [A/C universal remote file](/assets/resources/infrared/assets/ac.ir).

The order of signals is not important, but they must be preceded by a following comment: `# Model: <Your model name>` in order to keep the library organised.

When done, open a pull request containing the changed file.
