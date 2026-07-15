# How to use Flipper as a new SubGHz remote (not clone of original remote)

### If your system is not added here that doesn't mean flipper don't support it! Look into add manually menu, and search for your manufacturers inscturctions!
### Also many supported systems can be used only from `Read` mode, `Add Manually` is used only to make new remotes that can be binded with receiver

## FAAC SLH (NEW!)
1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> FAAC SLH (select your frequency)
2. Open your new remote file
3. Open your receiver box, find programming button on the receiver board.
4. Hold Up arrow button on the flipper to send programming signal - at same time press and hold programming button on the receiver board.
5. Led on the receiver board will light on, then off, then on, then off again then on again
6. Release all buttons
7. Press send button on the flipper couple times holding it for 1-3 seconds
8. Done!
Watch this video to learn more : https://www.youtube.com/watch?v=NfZmMy37XUs

...
How to get Seed value from your original remote or bind new remote using existing (master) remote? 
1. Go to SubGHz -> Read - Select frequency 868.35 or 433.92 and modulation AM650
2. Hold two buttons on the original master remote until led turns on
3. Click one button that you want to get seed from (Seed is unique for each button on original remote!)
4. You will get signal in the read screen on flipper, open that and see your original remote seed for button you used
5. You can create new remote using that seed and bind that to receiver without opening the box! Faac has procedure that allows to bind new remotes using master remote, you can use flipper for that 
6. Go to SubGHz -> Add Manually -> FAAC SLH Man. (your Freq)
7. Enter those values -> REPLACE `R` with any random digits like 1,2,3..
FIX -> A0 RR RR R6
COUNTER -> 00 00 02
SEED -> Your seed from the remote button you got earlier
8. Flipper will act as new remote, press Send button couple times near the receiver to register new remote
9. Done!

## Dea Mio
1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> Dea Mio 433Mhz
2. Open your new remote file
3. Right arrow button on the flipper simulates press of hidden button in original remote
4. Send button simulates one of basic buttons of the remote, can be programmed into the receiver
5. Follow manufacturer instructions on new remotes programming

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

With access to the receiver box:
1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> KL: Doorhan 433Mhz or 315Mhz depends on your receiver (find out by reading your existing remote or follow guide below)
- Finding frequency

There are 2 frequencies for DoorHan: 315.00 / 433.92. To determine them it is enough to create a DoorHan remote control with one of the frequencies via Sub-GHz -> Add manually, press the button and watch the receiver's reaction. If you have guessed the frequency, the light bulb will turn on when we press the button on the FZ and turn off when we release it.

2. Binding the remote control

Once you have access to the receiver (removed the protective cover), look at the buttons:
- If there are 4 buttons (Radio, Reverse, Auto, ...) then press and hold Radio until the LED lights up, then press the FZ button 2 times and the LED goes out;
- If there are 4 buttons (R, P, +, -) and display, press R, then press 2 times the button on FZ and wait +/- 10 seconds;
- If there are 4 buttons (+, -, F, TR) and display, press TR, then press 2 times the button on FZ and wait +/- 10 seconds;
- In other cases there is a “universal” instruction: Press and hold the button “P” +/- 2 seconds until the LED flashes, then press 2 times the button on the FZ and the LED goes out.

In all cases it is recommended to wait until the receiver returns to normal mode.

With existing remote:
1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> KL: Doorhan 433Mhz or 315Mhz depends on your receiver (find out by reading your existing remote)
2. Open your new remote file
3. For next steps be close to the receiver board, around 1-2 meters
4. Press second button (lowest one) on the old remote, do not release second button and press 1st (upper) button, hold buttons for 1 sec and release them
5. Press working button on the old remote (the button you use for operating the receiver, aka opening the gate, etc) hold for 1 sec and release 
6. Actions with old remote must be done in 5 seconds time, do not hold buttons for too long, and do not make it very fast
7. Receiver will beep, you will have 10 seconds to add new remote, now press Send on new remote on flipper two times holding for at least 1 sec
8. Receiver will beep again telling that new remote is added sucessfuly!
9. Done!

With copy of existing remote on flipper:
1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> KL: Doorhan 433Mhz or 315Mhz depends on your receiver (find out by reading your existing remote)
2. Open your existing remote (original) file
3. For next steps be close to the receiver board, around 1-2 meters
4. Press left button (0x8) on the flipper, hold for 1 sec and release the button and press right (0xA) button, hold button for 1 sec and release
5. Press working button on the flipper, should be center one aka Send (the button you use for operating the receiver, aka opening the gate, etc) hold for 1 sec and release 
6. Actions with original remote copy must be done in 5 seconds time, do not hold buttons for too long, and do not make it very fast
7. Receiver will beep, now hold back and open new remote file, you will have 10 seconds to add new remote, press Send on new remote on flipper two times holding for at least 1 sec
8. Receiver will beep again telling that new remote is added sucessfuly!
9. Done!

Watch this videos to learn more (videos in Russian language): https://www.youtube.com/watch?v=wZ5121HYv50 / https://www.youtube.com/watch?v=1ucrDKF3vWc

## Somfy Telis

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> Somfy Telis 433Mhz
2. Open your new remote file
3. Long press (hold) the ‘Prog’ button on a remote that is already registered to the device, until the blinds move shortly up and down.
4. Press and hold the ‘Prog’ button on the flipper (Left Arrow), until the blinds move shortly up and down again.
5. Done?

## BFT Mitto 

How to create new remote and bind it to receiver (will not conflict with original remotes):

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> BFT Mitto 433Mhz
2. Open your new remote file
3. You need to be in minimum 3 meters to receiver
4. Original Remote: Press hidden button on back of remote with a pin or paper clip OR press Button 1 & 2 together until remote LED lights.
5. Original Remote: Momentarily press button that opens device
6. Long press (Right Arrow) - (0xF button - Btn:F) on Flipper for like 3-5 sec
7. Press the button you want to bind to open the device on the flipper
8. Press (Right Arrow) - (0xF button - Btn:F) again
9. Done?

OR

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> BFT Mitto 433Mhz
2. Open your new remote file
3. Open your receiver board box
4. **Watch this video**: https://www.youtube.com/watch?v=5QXMBKI_-Ls
5. Long press (Right Arrow) - (0xF button - Btn:F) on Flipper for like 3-5 sec -> Will act like holding Button 1 & 2 on original remote as shown on video
6. Done?

--

How to get seed to make full clone of your remote (**will conflict with original remote!!!!!**):

**WARNING!!!! This method can desync your original remote, please avoid using it! It can be used in rare cases like when your remote works poorly or has broken buttons and you want to replace it with flipper**

1. Open `Read` in SubGHz on your flipper
2. (ONLY FOR ORIGINAL REMOTES) Hold all buttons on your remote at same time, example -> for 2 button remote - press them both at same time and hold OR press hidden button on back of remote with a pin or paper clip
For 4 buttons remote press & hold two buttons at upper row
3. You will receive signal on your flipper, open that signal and see `Fix:` value, it should start from `F` like `F00F1C9B`
4. If `Fix:` is showing first `F` see `Hop:` value -> This is your remote Seed (except first digit `F` (this is the button code, aka programming button pressed means `F`))
5. Write down Hop value and replace first digit - `F` with `0`
6. Press button on your remote that you want to clone and receive its signal on your flipper
7. Open and write down `Fix:` value where first digit will be same as your button ID `Btn:`
8. Create new remote using BFT Mitto [Manual] - Enter FIX from step 7, enter counter `FF F9`, enter seed from step 5
9. Using counter values like `FF F9` can help bypassing current original remote counter value, and in result it also can fully desync original remote, only one remote can work at same time using this method
10. Also you can do this: Save your signal of the original remote (will say KL: Unknown),
then copy file to the PC and edit it and insert/replace those values after the `Key: 01 23 45 67 89 AB CD EF` (your key will have different value)
```
Seed: 0X XX XX XX
Manufacture: BFT
``` 
Replace `X`'s with digits from your Seed that you obtained by reading two button hold at the first steps, 
Save and copy that file back to the flipper
Now you will have exact clone of your remote that will have same counter, by making couple presses you will make it higher than original and receiver will work with it, but original remote will reguire same amount of presses to work again, and vice versa.
11. Also your original remote may become non working since it needs to be re-added into receiver board if you made counter much higher than original :C

## CAME Atomo
Known names are: TOP42R / TOP44R - TOP44RGR (806TS-0130)

How to create new remote and bind it to receiver (will not conflict with original remotes):

With original remote (or copy of the original remote):

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> CAME Atomo 433MHz or 868MHz
2. Open your new remote file
3. You need to be in minimum 3 meters to receiver
4. Original Remote: Press and hold button that is bound with that receiver (the one you use with it), and hold it for about 10 seconds.
5. You will have about 20 seconds to add new remote
6. Long press Send on Flipper in new remote for like 3-4 sec and release - this will add new remote to the receiver
7. Press and hold Send again after waiting 20 seconds - this will trigger the receiver
8. Done, when using CAME Atomo from flipper please hold Send button for at least 2 seconds to allow code to be fully transmit, flipper transmits only while button is held

Note: Static 24/12 bit or TWEE remotes cannot trigger programming mode in the receiver and cannot be bound if programming mode was triggered by Atomo type remote, only Atomo remotes can be added if remote programming was done by Atomo remote, Static remotes have option to clone from one remote to another, but it requires first remote to be added to the receiver via button on the receiver board

With access to receiver box:

1. Create new remote with randomly generated serial: Go to SubGHz -> Add Manually -> CAME Atomo 433MHz or 868MHz
2. Open your new remote file
3. Open the receiver box and find programming button related to the used channel, for example RE432M/RE862M receiver has two independent channels which can have different remotes / buttons on them, when you found connected channel press "1" or "2" button on the receiver board to enter programming mode
4. Long press Send on Flipper new remote for like 3-4 sec and release - this will add new remote to the receiver
5. Click CLEAR button one time on the receiver board to exit programming mode, or wait about 20 seconds it will exit from programming mode automatically
6. Done, when using CAME Atomo from flipper please hold Send button for at least 2 seconds to allow code to be fully transmit, flipper transmits only while button is held


Watch this video to learn more (video in Russian language): https://www.youtube.com/watch?v=XeHUwfcSS30

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
- [FAAC SLH](https://www.youtube.com/watch?v=NfZmMy37XUs)
- [Somfy Telis](https://pushstack.wordpress.com/somfy-rts-protocol/)
- [BFT Mitto](https://www.retroremotes.com.au/wp-content/uploads/2017/03/BFT-MITTO-2-4-19-6-17.pdf)
- [NICE FLOX2R Receiver Programming](https://apollogateopeners.com/store/pdf/apollo-flor-s-receiver-programming-guide.pdf)
- [Nice Flor S Programming](https://motepro.com.au/Instructions/Nice.pdf)
