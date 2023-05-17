# How to use Flipper as a new SubGHz remote (not clone of original remote)

### If your system is not added here that doesn't mean flipper don't support it! Look into add manually menu, and search for your manufacturers inscturctions!
### Also many supported systems can be used only from `Read` mode, `Add Manually` is used only to make new remotes that can be binded with receiver

## AN-Motors AT4

**This instruction for older boards, if your has no** `Learn` **button but has buttons** `F`, `CL`, `+`, `-` **read instruction from Alutech AT4N**
1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> AN-Motors AT4 433Mhz
2. Open your new remote file
3. Open your receiver box, find button `Learn` click it one time, led will turn on.
4. Press `Send` on your flipper one time, led on receiver board will turn off.
5. Press `Send` on your flipper again, led on receiver will start flashing, wait couple seconds until led turns off.
6. Done

Watch this video to learn more (video in Russian language): https://www.youtube.com/watch?v=URVMtTELcnU

## Alutech AT4N (AN-Motors)

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> Alutech AT4N 433Mhz
2. Open your new remote file
3. Open your receiver box, find button `F` press it for ~3sec, display will show `Pr`.
4. Click `F` button couple times until you see `Lr` on screen
5. Using buttons `+` / `-` select free number that has no remotes in it (if it has remote programmed on that number, it will show a red dot on the down right corner)
6. Press `Send` on your flipper one time, display on receiver board will flash and red dot will appear next to remote number.
7. Press button `F` on receiver board for ~3sec to exit programming mode
8. Done

Watch this video to learn more and see how different boards can be programmed (video in Russian language): https://www.youtube.com/watch?v=XrOVVYhFXDg

## Aprimatic TR

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> KL: Aprimatic 433Mhz
2. Open your new remote file
3. Push all 4 buttons at same time on your existing remote thats already works with receiver
4. Receiver makes a continuous beep
5. Press `Send` on your flipper for ~2 seconds
6. Wait until receiver stops beeping 
7. Done?

## Doorhan

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> KL: Doorhan 433Mhz or 315Mhz depends on your receiver (find out by reading your existing remote)
2. Open your new remote file
3. Push `P` button for ~2 sec, led will start flashing
4. Press `Send` on your flipper for ~2 seconds
5. Led on the receiver board will flash and turn off
6. Done!

Also you can program new remote using old remote on newer boards! See first video below:
Watch this videos to learn more (videos in Russian language): https://www.youtube.com/watch?v=wZ5121HYv50 / https://www.youtube.com/watch?v=1ucrDKF3vWc

## Somfy Telis

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> Somfy Telis 433Mhz
2. Open your new remote file
3. Long press (hold) the ‘Prog’ button on a remote that is already registered to the device, until the blinds move shortly up and down.
4. Press and hold the ‘Prog’ button on the flipper (Left Arrow), until the blinds move shortly up and down again.
5. Done?

## BFT Mitto 

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> BFT Mitto 433Mhz
2. Open your new remote file
3. You need to be in minimum 3 meters to receiver
4. Original Remote: Press hidden button on back of remote with a pin or paper clip OR press Button 1 & 2 together until remote LED lights.
5. Original Remote: Momentarily press button that opens device
6. Long press (Right Arrow) - (0xF button - Btn:F) on Flipper for like 3-5 sec
7. Done?

## CAME Atomo

1. Use google to find instructions - `how to program new CAME Atomo remote into receiver`
2. Watch this video to learn more (video in Russian language): https://www.youtube.com/watch?v=XeHUwfcSS30

## Nice Flor S

- Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> Nice FloR-S 433Mhz
- Open your new remote file

### Coding using an existing remote
To enter the code of a new remote control without using your receiver, you will need
an authorised remote control (note: the first remote control must always be entered
using the receiver key). Now, with the two remote controls (your already coded
remote, and your new remote), which we shall call NEW (the one whose code we want
to enter) and OLD (the authorised one), position yourself within 3m of the gate/garage
receiver and then:
1. Press and hold the `Send` button on the flipper for at least 5 seconds and then
release.
2. Press the button on the already programmed remote 3 times slowly.
3. Press the `Send` button on the flipper slowly and then release.

### Coding directly to your receiver
Your new remote will program to your receiver as per your original remote
instructions, so please refer to your manual. But for a typical NICE FLOX2R Receiver,
the programming procedure is as follows:
1. Press the learning button on your receiver for 1-2 seconds. The LED will turn on
for 5 seconds. Within 5 seconds, complete the next step.
2. Press a `Send` button on your flipper until the LED on your receiver turns off.
3. Release the remote button and wait for 2 seconds.
4. Press the `Send` button on your flipper again. The LED on your receiver
will now flash 3 times. This indicates that your remote has been successfully
coded. If this does not happen, repeat the whole procedure from the
beginning, and try again.
5. Wait 5 seconds. Press the button on your new remote to test if it opens your
garage/gate.


#### Follow links below to find more detailed instructions!!!

#### Materials used:
- [Somfy Telis](https://pushstack.wordpress.com/somfy-rts-protocol/)
- [BFT Mitto](https://www.retroremotes.com.au/wp-content/uploads/2017/03/BFT-MITTO-2-4-19-6-17.pdf)
- [NICE FLOX2R Receiver Programming](https://apollogateopeners.com/store/pdf/apollo-flor-s-receiver-programming-guide.pdf)
- [Nice Flor S Programming](https://motepro.com.au/Instructions/Nice.pdf)