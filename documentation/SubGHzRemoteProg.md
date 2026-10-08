# Using the Flipper as a new Sub-GHz remote

This guide is about making the Flipper into a **brand new remote** that you register in your gate, garage or blind receiver - the same way you would register a spare remote you bought in a shop.

It is **not** about cloning, and for the systems in this guide that matters:

- **Rolling code (dynamic) remotes** - BFT, Erreka, FAAC, Nice, CAME Atomo, Somfy, KeeLoq and the rest of this guide. Every press sends a new code from a counter, and the receiver only accepts a counter that moved forward. A clone shares the original's counter, so the two fight over it - whichever one you pressed last works, and the other needs several presses to catch up. That is why you make a **new** remote instead: it gets its own serial and counter, and lives alongside your original ones.
- **Static (fixed) code remotes** - CAME 12/24 bit, Nice Flo, Princeton, Marantec, Holtek, Roger, Gate TX and the other static protocols. There is no counter, so **cloning is fine here** - a copy is simply identical to the original and both keep working, forever, in any order. Just read the remote with `SubGHz` -> `Read` and save it. You do not need this guide for those, unless you want a remote with its own separate code.

So: if your remote is static, read and save it. If it is rolling code, find your system below.

---

## Read this first

**What `Add Manually` does.** `SubGHz` -> `Add Manually` builds a brand new remote with a randomly generated serial number and saves it as a file. Nothing is sent until you open that file and press a key. Your original remotes are not touched.

**If your system is not in this guide, the Flipper may still support it.** Open `SubGHz` -> `Add Manually` and scroll the list. If your brand is there, create a remote and follow the programming steps from your *receiver's own manual* - this guide only collects the ones people have already worked out. The full protocol list is in [SubGHzSupportedSystems.md](/documentation/SubGHzSupportedSystems.md).

**`Read` and `Add Manually` are different jobs.** `Read` captures and replays a remote you already have. `Add Manually` creates a new remote that can be registered in the receiver. Many protocols support `Read` only.

**The Flipper's keys are the original remote's buttons.** When a remote file is open:

| Flipper key | What it sends |
|---|---|
| `OK` / centre (shown as `Send`) | the remote's main button |
| Up / Down / Left / Right arrows | the remote's other buttons |

Which arrow is which depends on the protocol. Where a receiver wants a *hidden* or *programming* button - a recessed pin hole, two buttons held together, pads you bridge inside the case - the Flipper sends it as **one single key press**. Each section below tells you which key that is, and there is a full table in [Programming / hidden button reference](#programming--hidden-button-reference).

**Hold the key down.** The Flipper transmits only while a key is held. A quick tap often sends an incomplete code. Hold for 1-3 seconds unless a step says otherwise.

**Timing matters.** Most receivers open a programming window of 5, 10 or 30 seconds. Read the whole procedure before you start so you are not reading it while the window is closing.

> [!WARNING]
> On many receivers, holding the programming button **too long** erases every remote the receiver knows, not just one. If that happens you have to re-register all of your original remotes. When a step says "about 1 second", do not hold it for five.

> [!NOTE]
> A step ending in `Done!` has been confirmed working. A step ending in `Done?` comes from the manufacturer's manual but nobody has reported back yet - it should work, tell us in the issues tab if it does not.

**`Add Manually [Advanced]`** is the second menu entry, right under `Add Manually`. Use it when you need to type values in by hand instead of getting a random serial. After you pick the type it asks for `SERIAL`, `BUTTON`, `COUNTER` and, on protocols that use one, `SEED` - all in hex.

---

## Find your system

| | | |
|---|---|---|
| [Alutech AT4N](#alutech-at4n-an-motors) | [AN-Motors AT4](#an-motors-at4) | [Aprimatic TR](#aprimatic-tr) |
| [Beninca ARC (TO.GO)](#beninca-arc-togo) | [BFT Mitto](#bft-mitto) | [CAME Atomo](#came-atomo) |
| [Cardin S449](#cardin-s449) | [DEA Mio](#dea-mio) | [Ditec GOL4](#ditec-gol4) |
| [DoorHan](#doorhan) | [Erreka IRIS](#erreka-iris-new) | [FAAC RC, XT](#faac-rc-xt) |
| [FAAC SLH](#faac-slh) | [Genius (SLH)](#genius-slh) | [Genius TX4RC](#genius-tx4rc-bravo--echo) |
| [Hormann EcoStar](#hormann-ecostar) | [Jarolift](#jarolift) | [KingGates Stylo 4k](#kinggates-stylo-4k) |
| [Mhouse](#mhouse) | [Nice Flor S](#nice-flor-s) | [Nice One](#nice-one) |
| [Nice Smilo](#nice-smilo) | [Security+ 1.0 / 2.0](#security-10--20-chamberlain-liftmaster-craftsman) | [Somfy Keytis](#somfy-keytis) |
| [Somfy Telis](#somfy-telis) | [Sommer](#sommer) | [V2 Phoenix (Phox)](#v2-phoenix-phox) |

Not listed? See [Programming / hidden button reference](#programming--hidden-button-reference) - it covers every other entry in the `Add Manually` menu.

---

## Alutech AT4N (AN-Motors)

- **Create it:** `SubGHz` -> `Add Manually` -> `Alutech AT4N 433MHz`

This is for boards with a display and `F`, `CL`, `+`, `-` buttons. If your board has a `Learn` button instead, use [AN-Motors AT4](#an-motors-at4).

1. Open your new remote file
2. Open the receiver box, find the `F` button and hold it for ~3 sec - the display shows `Pr`
3. Press `F` a few more times until the display shows `Lr`
4. With `+` / `-` pick a free remote number. A number that already has a remote on it shows a red dot in the bottom right corner
5. Press `Send` on the Flipper one time - the display flashes and a red dot appears next to that number
6. Hold `F` on the receiver board for ~3 sec to leave programming mode
7. Done!

[Video walkthrough, also shows other board types (Russian)](https://www.youtube.com/watch?v=XrOVVYhFXDg)

---

## AN-Motors AT4

- **Create it:** `SubGHz` -> `Add Manually` -> `AN-Motors AT4 433MHz`

This is for older boards with a `Learn` button. If your board has no `Learn` button but has `F`, `CL`, `+`, `-`, use [Alutech AT4N](#alutech-at4n-an-motors) instead.

1. Open your new remote file
2. Open the receiver box, find the `Learn` button and click it one time - the led turns on
3. Press `Send` on the Flipper one time - the led on the receiver board turns off
4. Press `Send` again - the led starts flashing, wait a couple of seconds until it turns off
5. Done!

[Video walkthrough (Russian)](https://www.youtube.com/watch?v=URVMtTELcnU)

---

## Aprimatic TR

- **Create it:** `SubGHz` -> `Add Manually` -> `KL: Aprimatic 433MHz`
- **Programming key:** Right Arrow = `0xF` (on the original remote this is all 4 buttons held together)

1. Open your new remote file
2. On your existing remote that already works with the receiver, push all 4 buttons at the same time
3. The receiver makes a continuous beep
4. Press `Send` on the Flipper and hold for ~2 sec
5. Wait until the receiver stops beeping
6. Done?

---

## Beninca ARC (TO.GO)

- **Also sold as:** TO.GO 2VA / TO.GO 4VA (previously TO.GO 2WV / 4WV) - receivers WB, WI, and receivers built into the control panel
- **Create it:** `SubGHz` -> `Add Manually` -> `Beninca ARC 433MHz`
- **Programming key:** **Down Arrow** = `0x0`, the hidden button

On a 2 button original remote the hidden button means holding both buttons at once, which is awkward or impossible to do reliably. On the Flipper it is just the Down Arrow.

### With a remote that already works - receivers WB / WI

1. Stand close to the receiver
2. Open the file of the remote that already works, press **Down Arrow** and hold ~3 sec - the hidden function is now active (on an original remote its led blinks at this point)
3. Within 5 seconds press `Send` on that same remote, for the channel you want to share
4. Within 5 seconds open your new remote file and press `Send` - this is the button that will control that channel
5. The receiver leaves programming mode, test the new remote
6. Done?

### With a remote that already works - receiver built into the control panel

Here the hidden button has to be sent from **both** remotes:

1. Already working remote: **Down Arrow**, then `Send` within 5 seconds
2. New remote: **Down Arrow**, then `Send` within 5 seconds
3. Done?

> [!NOTE]
> ARC and HCS (Rolling Code) remotes cannot share one receiver. The first remote you save decides which type the receiver will accept from then on. Changing it needs a full receiver reset.

---

## BFT Mitto

- **Create it:** `SubGHz` -> `Add Manually` -> `BFT Mitto 433MHz`
- **Programming key:** Right Arrow = `0xF` (on the original this is the pin hole on the back, or buttons 1+2 held together)

### With a remote that already works

1. Open your new remote file
2. Stand at least 3 meters from the receiver
3. **Original remote:** press the hidden button on the back with a pin or paper clip, OR press buttons 1 & 2 together, until the remote's led lights up
4. **Original remote:** briefly press the button that opens the device
5. On the Flipper, long press **Right Arrow** (`0xF` - `Btn:F`) for 3-5 sec
6. Press the button on the Flipper that you want to use for opening the device
7. Press **Right Arrow** again
8. Done?

### With access to the receiver box

1. Open your new remote file
2. Open the receiver board box
3. [Watch this video first](https://www.youtube.com/watch?v=5QXMBKI_-Ls) - it shows the board side of the procedure
4. On the Flipper, long press **Right Arrow** (`0xF` - `Btn:F`) for 3-5 sec. This acts exactly like holding buttons 1 & 2 on the original remote as shown in the video
5. Done?

### Making a full clone instead (reads the Seed off your original)

> [!CAUTION]
> This makes a copy that **fights with your original remote**. Both use the same counter, so whichever one you press last works and the other needs several presses to catch up. Your original may even stop working until it is re-registered at the receiver board.
>
> Only do this if the original is broken or worn out and you want the Flipper to replace it. To simply add the Flipper as an extra remote, use one of the two procedures above instead.

1. Open `SubGHz` -> `Read` on your Flipper
2. **On the original remote only:** hold every button at the same time - on a 2 button remote hold both, on a 4 button remote hold the two in the top row - or press the hidden button on the back with a pin
3. You receive a signal. Open it and look at `Fix:` - it starts with `F`, for example `F00F1C9B`. The leading `F` is the button code, meaning "programming button pressed"
4. The `Hop:` value of that same signal is your Seed
5. Write the `Hop` value down and replace its first digit `F` with `0`
6. Now press the button on your remote that you actually want to clone, and read that signal
7. Write down its `Fix:` value. Its first digit matches the `Btn:` shown for that signal
8. Create the remote: `SubGHz` -> `Add Manually [Advanced]` -> `BFT Mitto 433MHz`, then enter:
   - `SERIAL` - the `Fix` from step 7 with its **first** digit replaced by `0`
   - `BUTTON` - the first digit of that `Fix` (the one you just replaced)
   - `COUNTER` - `FF F9`
   - `SEED` - the value from step 5
9. A high counter like `FF F9` jumps past your original remote's current counter. That is what makes the clone work immediately - and what desyncs the original

**Or edit a saved file instead of typing values in.** Save the signal of your original remote (it will be listed as `KL: Unknown`), copy the file to a PC and add these two lines after the `Key: ...` line:

```
Seed: 0X XX XX XX
Manufacture: BFT
```

Replace the `X`s with the digits of the Seed from step 4, save, and copy the file back to the Flipper. You now have an exact clone with the same counter - a few presses push it past the original, and from then on only one of the two works at a time.

---

## CAME Atomo

- **Also sold as:** TOP42R / TOP44R / TOP44RGR (806TS-0130)
- **Create it:** `SubGHz` -> `Add Manually` -> `CAME Atomo 433MHz` or `CAME Atomo 868MHz`

> [!IMPORTANT]
> When using CAME Atomo from the Flipper, always hold `Send` for at least 2 seconds. The Flipper transmits only while the key is held, and this protocol needs the time to get the whole code out.

### With a remote that already works (or a copy of it on the Flipper)

1. Open your new remote file
2. Stand at least 3 meters from the receiver
3. **Original remote:** press and hold the button that is bound to that receiver - the one you normally use - for about 10 seconds
4. You now have about 20 seconds to add the new remote
5. Long press `Send` on the Flipper in the new remote file for 3-4 sec and release - this registers it
6. Wait 20 seconds, then press and hold `Send` again - this should now operate the receiver
7. Done!

### With access to the receiver box

1. Open your new remote file
2. Open the receiver box and find the programming button for the channel you need. Some receivers have several independent channels - an RE432M / RE862M for example has two, each with its own remotes and buttons. Press `1` or `2` on the receiver board to enter programming mode for that channel
3. Long press `Send` on the Flipper for 3-4 sec and release - this registers the new remote
4. Press `CLEAR` on the receiver board once to leave programming mode, or just wait ~20 seconds and it leaves by itself
5. Done!

> [!NOTE]
> Static 12/24 bit CAME remotes and CAME TWEE remotes cannot trigger programming mode in the receiver, and they cannot be registered into a programming mode that an Atomo remote opened. Only Atomo remotes can. Static remotes have their own remote-to-remote cloning option, but it needs the first remote to be registered at the receiver board.

[Video walkthrough (Russian)](https://www.youtube.com/watch?v=XeHUwfcSS30)

---

## Cardin S449

- **Also sold as:** S449 QZ1 / QZ2 / QZ4, TXQ449100 / TXQ449200 - receivers RCQ449
- **Read with:** modulation `FM12K`, **not** AM650
- **Create it:** `SubGHz` -> `Add Manually` -> `KL: Cardin S449 433MHz`
- **Programming key:** Right Arrow = `0xD` (on the original this is the button in the small hole under the keys)

1. Open your new remote file
2. Stand 1-2 meters from the receiver
3. Open the file of the remote that already works, press **Right Arrow** and release it when the receiver makes a short beep
4. Press `Send` on that same remote one time - the receiver makes another short beep
5. Open your new remote file and press `Send` - the receiver makes 2 beeps in a row
6. Release the key - the receiver makes one longer beep
7. Wait at least 10 seconds, then try the new remote
8. Done?

**If it fails,** or if you hear a long beep before step 5, wait 20 seconds and start again from step 3.

---

## DEA Mio

- **Create it:** `SubGHz` -> `Add Manually` -> `KL: DEA Mio 433MHz`
- **Programming key:** Right Arrow = `0xF`, the hidden button on the original remote

1. Open your new remote file
2. `Send` acts as one of the normal buttons of the remote - this is the one you register into the receiver
3. **Right Arrow** acts as the hidden button of the original remote
4. Follow your manufacturer's instructions for adding a new remote, using those two keys in place of the original remote's buttons

---

## Ditec GOL4

- **Also sold as:** GOL4 / GOL4C - receivers BIXLG4, BIXLP2, BIXLS2, BIXR2
- **Create it:** `SubGHz` -> `Add Manually` -> `Ditec GOL4 433MHz`
- **Programming key:** Right Arrow = `0x0`, the hidden button on the original remote

1. Open your new remote file
2. Open the receiver box, press and release the `PRG` button - the `SIG` led lights up and stays on
3. Press `Send` on the Flipper for the channel you want to store
4. The `SIG` led flashes while storing. When it is steady again you can add another remote
5. Storing ends 10 seconds after the last remote, or press `PRG` again to leave straight away
6. Done?

> [!NOTE]
> On some boards a jumper on the receiver has to be set before programming works at all. Check your receiver manual.

---

## DoorHan

- **Create it:** `SubGHz` -> `Add Manually` -> `KL: DoorHan 433MHz` or `KL: DoorHan 315MHz`

### Finding your frequency first

DoorHan uses either 315.00 or 433.92 MHz. To find out which, create a DoorHan remote on one of them, open it, press `Send` and watch the receiver. If you guessed right, the light on the receiver turns on while you hold the key and off when you release it. If nothing happens, try the other frequency.

You can also just read your existing remote in `SubGHz` -> `Read`.

### With access to the receiver box

Take the protective cover off and look at the buttons on the board:

- **4 buttons (`Radio`, `Reverse`, `Auto`, ...):** press and hold `Radio` until the led lights up, then press `Send` on the Flipper 2 times - the led goes out
- **4 buttons (`R`, `P`, `+`, `-`) and a display:** press `R`, then press `Send` on the Flipper 2 times, then wait about 10 seconds
- **4 buttons (`+`, `-`, `F`, `TR`) and a display:** press `TR`, then press `Send` on the Flipper 2 times, then wait about 10 seconds
- **Anything else:** press and hold `P` for about 2 seconds until the led flashes, then press `Send` on the Flipper 2 times - the led goes out

In all cases, wait until the receiver returns to normal mode before testing.

### With an original remote in your hand

1. Open your new remote file
2. Stand 1-2 meters from the receiver board
3. On the old remote, press the second button (the lower one), keep holding it and also press the first button (the upper one). Hold both for 1 sec, then release
4. Press the working button on the old remote - the one you use to open the gate - hold for 1 sec and release
5. Steps 3 and 4 must be done within 5 seconds of each other. Do not hold the buttons too long, and do not rush either
6. The receiver beeps. You now have 10 seconds: press `Send` on the Flipper's new remote 2 times, holding for at least 1 sec each time
7. The receiver beeps again - the new remote is registered
8. Done!

### With a copy of your original remote on the Flipper

1. Open your **existing (original)** remote file
2. Stand 1-2 meters from the receiver board
3. Press **Left Arrow** (`0x8`) on the Flipper, hold 1 sec, release. Then press **Right Arrow** (`0xA`), hold 1 sec, release
4. Press the working button - the centre `Send` key - hold 1 sec and release
5. Steps 3 and 4 must be done within 5 seconds of each other. Do not hold too long, and do not rush
6. The receiver beeps. Press `Back`, open your **new** remote file - you have 10 seconds - and press `Send` 2 times, holding at least 1 sec each time
7. The receiver beeps again - the new remote is registered
8. Done!

[Video walkthroughs (Russian)](https://www.youtube.com/watch?v=wZ5121HYv50) and [second one](https://www.youtube.com/watch?v=1ucrDKF3vWc)

---

## Erreka IRIS (NEW!)

- **Also sold as:** IRIS IR02 / IR04 on 433.92 MHz, IR02/868 / IR04/868 on 868.35 MHz
- **Create it:** `SubGHz` -> `Add Manually` -> `Erreka 433MHz`
- **Programming key:** Right Arrow = `0xF` (on the original this means bridging pins `1` and `5` of the 5 way connector inside the remote)

Erreka IRIS is a Roller Code remote - KeeLoq with a secret Seed. The manufacturer gives two ways to register a new remote, and both work from the Flipper.

> [!NOTE]
> `Add Manually` creates a 433.92 MHz remote. For an IR02/868 or IR04/868 remote, change the frequency of the created signal to 868.35 MHz before you use it.

### With access to the receiver box

1. Open your new remote file
2. Open the receiver box and activate programming mode - usually a button on the receiver board, check your receiver manual
3. Press `Send` on the Flipper one time - the receiver beeps **twice** if the remote was stored
4. Repeat step 3 for any other remotes you want to add
5. Turn programming mode off on the receiver when you are done
6. Done!

### With a remote that already works

The original IRIS opens the programming window by bridging pins `1` and `5` of the 5 way connector inside its case - that makes it transmit its Seed in the clear, and the receiver answers with a single beep. On the Flipper the same thing is just the Right Arrow, no wires needed.

The remote that opens the window has to be one the receiver already knows. That means either the first Flipper remote you registered with the receiver button above, or your original remote with its Seed recovered (see below).

1. Open the file of the Erreka remote that is already registered
2. Long press **Right Arrow** (`0xF` - `Btn:F`) for ~2 sec - the receiver beeps **once**, it is ready to store codes
3. You have 10 seconds: press `Back` and open your new remote file
4. Press `Send` - the receiver beeps **twice** if the remote was stored
5. If 10 seconds pass with nothing stored, the receiver leaves programming mode. Start again from step 2
6. Done?

### If the new remote will not register

Some installations give **every remote the same Seed** - the installer programmed them as a set. On those, a remote with a freshly generated random Seed is never accepted, no matter how exactly you follow the procedure above.

The fix is to keep the existing Seed and change only the serial, so you get a genuinely new remote that still belongs to the installation:

1. Get the Seed off a remote that already works - either way below
2. Create the remote with `SubGHz` -> `Add Manually [Advanced]` -> `Erreka 433MHz` and enter:
   - `SERIAL` - `0X XX XX XX`, any value that is not one of your existing remotes. **Keep the leading `0`** - that nibble is the button, which the Advanced screen asks for separately
   - `BUTTON` - `02`
   - `COUNTER` - `00 02`
   - `SEED` - the Seed from step 1
3. Run the registration procedure again, either one

> [!TIP]
> **Keep the leading zeros in the serial.** Some systems have turned up using only the bottom 3 or 4 digits of the serial, so their remotes look like `00 00 0A BC` rather than a full length number. If your installation is one of those, a long random serial may simply be refused.
>
> Read one of your working remotes first and copy the shape of its serial - same number of significant digits, leading zeros kept - and only change the last digits to make it unique.

> [!TIP]
> If two of your original remotes read with the same `Seed` once you have recovered it, that is this kind of installation, and reusing the Seed is the right move rather than a workaround.

### Getting the Seed off your original IRIS remote

Erreka cannot be decoded without the Seed - a normal button press from an original remote reads as `KL: Unknown` until you know it. There are two ways to get it.

**Option 1 - by shorting the pins and reading it.** This is the quick one, the remote transmits the Seed in the clear:

1. Open `SubGHz` -> `Read`, set the frequency to 433.92 or 868.35 and the modulation to `AM650`
2. Open your original IRIS remote and bridge pins `1` and `5` of the 5 way connector
3. You receive a signal. The manufacturer will show as **`Unknown`** - that is expected, not an error. The Flipper cannot name the manufacturer yet precisely because it does not know the Seed
4. Open the signal and look at `Fix:` - it starts with `F`, the button code meaning "programming button pressed"
5. **The `Hop:` value of that signal is your Seed**, in the clear
6. From here you can either type it into `Add Manually [Advanced]` as above, or turn the captured signal into a working file: save it, copy it to a PC and add these two lines after the `Key: ...` line:

```
Seed: 0X XX XX XX
Manufacture: Erreka
```

Replace the `X`s with the digits of the Seed from step 5, save, and copy the file back to the Flipper. It decodes now, and you can use it to open the programming window for new remotes.

**Option 2 - with the Seed Capturer app**, when you do not want to open the remote at all. Pick `Erreka` and your frequency and press one button repeatedly - see [Recovering a Seed with the Seed Capturer app](#recovering-a-seed-with-the-seed-capturer-app) for the whole procedure. This one needs an offline recovery step on a PC afterwards, so option 1 is faster if you can get the case open.

---

## FAAC RC, XT

- **Also sold as:** XT2 / XT4 433 RC / 868 RC, and the older RC coding
- **Create it:** `SubGHz` -> `Add Manually` -> `KL: FAAC RC,XT 433MHz` or `KL: FAAC RC,XT 868MHz`
- **Programming key:** Right Arrow = `0xB` (on the original this is buttons 1+2 held together, the "master" press)

> [!IMPORTANT]
> These are KeeLoq, not SLH. Read your remote first: if the Flipper calls it `KL: FAAC_RC,XT` you are in the right place. If it says `FAAC SLH`, use [FAAC SLH](#faac-slh) instead.

1. Open your new remote file
2. Stand close to the receiver
3. Open the file of the remote that already works, press **Right Arrow** and hold until the original remote's led would start flashing, about 1-2 sec, then release
4. While the window is open, press `Send` on that same remote for the button you want to copy
5. Open your new remote file and press `Send`
6. Repeat steps 3-5 for every other button you want
7. Done?

> [!NOTE]
> Only a **master** remote can hand its system code over. On an original remote you can tell them apart by the led - a short blink before it goes steady means master, steady straight away means slave. A slave cannot be used for this, so with a slave you have to register the new remote at the receiver instead.

---

## FAAC SLH

- **Create it:** `SubGHz` -> `Add Manually` -> `FAAC SLH 433MHz` or `FAAC SLH 868MHz`
- **Programming key:** **Up Arrow** sends the programming signal

### With access to the receiver box

1. Open your new remote file
2. Open the receiver box and find the programming button on the receiver board
3. Hold **Up Arrow** on the Flipper to send the programming signal, and at the same time press and hold the programming button on the receiver board
4. The led on the receiver board goes on, off, on, off, then on again
5. Release everything
6. Press `Send` on the Flipper a couple of times, holding each press for 1-3 seconds
7. Done!

[Video walkthrough](https://www.youtube.com/watch?v=NfZmMy37XUs)

### With a master remote - no need to open the box

FAAC has a procedure for registering new remotes from an existing master remote, and the Flipper can take part in it. First you read the Seed off the master.

1. Open `SubGHz` -> `Read`, set the frequency to 868.35 or 433.92 and the modulation to `AM650`
2. Hold two buttons on the original master remote until its led turns on
3. Click the one button you want the Seed for. **The Seed is different for every button on the original remote**
4. You receive a signal on the read screen. Open it and read the Seed for the button you used
5. Create the remote: `SubGHz` -> `Add Manually [Advanced]` -> `FAAC SLH 433MHz` (or 868), then enter:
   - `SERIAL` - `0A 0R RR RR`, replacing each `R` with any digit you like
   - `BUTTON` - `06`
   - `COUNTER` - `00 00 00 02`
   - `SEED` - the Seed you read in step 4
6. The Flipper now acts as a new remote. Press `Send` a couple of times near the receiver to register it
7. Done!

### If neither of those is possible

Two situations leave you stuck:

- **The receiver's remote programming is disabled.** Some installers switch it off, and then nothing you send will register a new remote
- **Your remote is a slave, not a master.** Only a master can transmit the programming code. On an original remote, press any button and watch the led - a short blink before it goes steady means master, steady straight away means slave

In both cases the only remaining route is a **clone** of a remote the receiver already knows, and that needs the Seed. Get it with the [Seed Capturer app](#recovering-a-seed-with-the-seed-capturer-app), then build the clone with the **original remote's** `SERIAL` and `BUTTON` instead of your own.

> [!TIP]
> `SERIAL` and `BUTTON` together make up the `Fix` value you see when reading a signal, and where the button sits inside `Fix` depends on the protocol:
> - **FAAC SLH and Genius:** `BUTTON` is the **last** digit of `Fix`. `SERIAL` is the rest, with a `0` put in front.
> - **KeeLoq protocols** (BFT, Erreka, DoorHan, Cardin and the other `KL:` entries): `BUTTON` is the **first** digit of `Fix`. `SERIAL` is that same `Fix` with the first digit replaced by `0`.

---

## Genius (SLH)

- **Also sold as:** Echo TX2 433 SLH / Echo TX4 433 SLH / KILO TX2 / TX4 / JLC / Amigo
- **Create it:** `SubGHz` -> `Add Manually` -> `Genius 433MHz` or `Genius 868MHz`
- **Programming key:** **Up Arrow** sends the programming signal

Genius SLH is the same frame as FAAC SLH with its own manufacturer key, so **[the FAAC SLH procedures](#faac-slh) apply exactly as written** - both the receiver button one and the master remote one.

1. Create the remote from the `Genius` entry for your frequency
2. Follow the [FAAC SLH](#faac-slh) steps

If the receiver's remote programming is disabled, or your remote is a slave and cannot send the programming code, the Seed route applies here too - capture it with the [Seed Capturer app](#recovering-a-seed-with-the-seed-capturer-app) (pick `Genius`) and follow [If neither of those is possible](#if-neither-of-those-is-possible) under FAAC SLH.

> [!IMPORTANT]
> Do not confuse this with `KL: Genius TX4RC 433M.`. The RC models are KeeLoq, not SLH, and they have [their own section](#genius-tx4rc-bravo--echo). If you are not sure which you have, read the remote first and see what the Flipper calls it.

---

## Genius TX4RC (Bravo / Echo)

- **Also sold as:** Echo TX2 433 RC / Echo TX4 433 RC / TE443H Bravo
- **Create it:** `SubGHz` -> `Add Manually` -> `KL: Genius TX4RC 433M.`
- **Programming key:** Right Arrow = `0xB`, the programming mode button of the original remote

1. Open your new remote file
2. Open the receiver box and press `SW1` for channel 1 or `SW2` for channel 2 - `LED1` / `LED2` lights up and stays on, that is learning mode
3. Within 10 seconds press `Send` on the Flipper and hold for at least 1 sec
4. The led blinks twice - the remote is stored
5. The receiver stays in learning mode, so you can add more remotes by repeating step 3. Press `SW1` / `SW2` again to leave, or it leaves by itself 10 seconds after the last remote
6. Done?

---

## Hormann EcoStar

- **Also sold as:** RSC2 / RSE2 / RSZ1 on 433.92 MHz - EcoStar Liftronic / Portronic drives
- **Create it:** `SubGHz` -> `Add Manually` -> `KL: Hor. EcoStar 433MHz`
- **Programming key:** **Down Arrow** = `0x6` - note this one is on Down, not Right

1. Open your new remote file
2. Find the clear programming button on the drive, usually underneath the motor, and hold it for ~1 sec - the indicator led on the motor starts flashing slowly
3. Press `Send` on the Flipper and hold for ~2 sec - the motor led speeds up
4. Press `Send` again and hold for ~2 sec - the motor led flashes very fast then goes out. The remote is programmed
5. Done?

> [!WARNING]
> Holding that clear button for ~5 seconds erases **every** remote the drive knows, not just one.

---

## Jarolift

- **Also sold as:** TDEF radio tube motors - TDRC / TDRCE remotes
- **Create it:** `SubGHz` -> `Add Manually` -> `Jarolift 433MHz`

The keys are already mapped to the original remote's buttons:

| Flipper key | Jarolift button |
|---|---|
| `Send` | Down |
| Left Arrow | Up |
| Down Arrow | Stop |
| **Up Arrow** | **Learn** (`0x1`) |

`Learn` is a single code. On the original remote you produce it by holding Up + Down together and then pressing Stop - on the Flipper it is one key press.

### With access to the motor head

1. Open your new remote file
2. Press the learning button on the motor head - the motor vibrates briefly, it is now in learning mode for 5 seconds
3. If you cannot reach that button, disconnect the motor from the mains and reconnect it - it enters learning mode automatically for 5 seconds
4. Within those 5 seconds press **Up Arrow** (`Learn`) on the Flipper
5. The motor vibrates again - the code was learned
6. Done!

### With a remote that already works

1. Open your new remote file
2. On the already working remote, hold Up + Down together, then press Stop 8 times - the motor vibrates briefly
3. Within 5 seconds press **Up Arrow** (`Learn`) on the Flipper
4. The motor vibrates again - the code was learned
5. Done?

> [!TIP]
> If the already working remote is also on your Flipper, step 2 is just **Up Arrow** (`Learn`) on that remote's file.

---

## KingGates Stylo 4k

- **Also sold as:** Stylo 2K / Stylo 4K (10S001) - receivers Fred, Myo
- **Create it:** `SubGHz` -> `Add Manually` -> `KingGates Stylo4k 433M.`

There is no separate programming button on this one. The four keys are simply the four channels of the original remote:

| Flipper key | Channel code |
|---|---|
| `Send` | `0xE` |
| Up Arrow | `0xD` |
| Down Arrow | `0xB` |
| Left Arrow | `0x7` |

1. Open your new remote file
2. Open the receiver box. On a `Fred` 2 channel receiver the led colour tells you the channel - green is channel 1, red is channel 2
3. Press the receiver button once for channel 1 (green) or twice for channel 2 (red) - this starts learning mode
4. Within 5 seconds press `Send` on the Flipper for ~1 sec
5. The led turns off then on again - the remote was learned
6. Learning mode ends after 5 seconds with no signal, or press the receiver button again to leave
7. Done?

---

## Mhouse

- **Also sold as:** GTX4 / GTX4C / TX3 / TX4 - also Moovo and Nice Home ECCO
- **Create it:** `SubGHz` -> `Add Manually` -> `KL: Mhouse 433MHz`
- **Programming key:** Right Arrow = `0xF`, the extra (hidden) button of the original remote

### With access to the receiver box

1. Open your new remote file
2. Press and hold the `P1` button on the receiver until the `P1` led stays on, then release
3. Press `Send` on the Flipper and hold until the `P1` led flashes 3 times
4. Release the key. This registers all buttons of the remote at once (Nice "mode 1")
5. Wait 10 seconds before testing the new remote
6. Done?

### With a remote that already works

Mhouse is the same Nice receiver family as [Nice Smilo](#nice-smilo), so the procedure is the same:

1. Stand close to the receiver
2. Open your new remote file, press and hold `Send` for at least 5 sec, then release
3. On the already working remote, press and slowly release its button 3 times
4. Open your new remote file again, press `Send` for 1 sec and release
5. Wait 10 seconds before testing the new remote
6. Done?

---

## Nice Flor S

- **Create it:** `SubGHz` -> `Add Manually` -> `Nice FloR-S 433MHz`

### With a remote that already works

You do not need the receiver for this, but **the very first remote always has to be registered with the receiver button.** You need one remote that is already authorised. Below, `OLD` is the authorised remote and `NEW` is the Flipper. Stand within 3 m of the gate or garage receiver.

1. Open your new remote file, press and hold `Send` for at least 5 seconds, then release
2. On the `OLD` remote, press its button 3 times slowly
3. Press `Send` on the Flipper slowly, then release
4. Done?

### With access to the receiver box

Your new remote registers exactly as your original remote's instructions describe, so use your own manual. For a typical NICE FLOX2R receiver it goes like this:

1. Open your new remote file
2. Press the learning button on the receiver for 1-2 seconds. The led turns on for 5 seconds - do the next step inside those 5 seconds
3. Press `Send` on the Flipper until the led on the receiver turns off
4. Release the key and wait 2 seconds
5. Press `Send` again. The led on the receiver flashes 3 times - the remote is registered. If it does not, start over from step 2
6. Wait 5 seconds, then press `Send` to test that it opens your gate or garage
7. Done!

---

## Nice One

- **Create it:** `SubGHz` -> `Add Manually` -> `Nice One 433MHz`

Nice One is the Nice FloR-S family with different keying, and the receiver side is identical - **[both Nice Flor S procedures](#nice-flor-s) apply exactly as written**.

1. Create the remote from the `Nice One 433MHz` entry
2. Follow the [Nice Flor S](#nice-flor-s) steps

> [!TIP]
> If your remote reads as FloR-S but the counter looks wrong, it may be **Nice O-Code** instead - that is FloR-S keyed with a 16 bit installer code. Use the `Nice O-Code` app (`Apps` -> `Sub-GHz`), it recovers the key from 4 or more captures.

---

## Nice Smilo

- **Also sold as:** SM2 / SM4 - receivers SMXI, SMXIS, OXI set to Smilo coding
- **Create it:** `SubGHz` -> `Add Manually` -> `KL: Nice Smilo 433MHz`
- **Programming key:** Right Arrow = `0xB`, the extra (hidden) button of the original remote

Nice receivers can store a remote in one of two modes. **Mode I** gives each remote button the matching receiver output - button 1 works output 1, button 2 works output 2. **Mode II** lets you pick which output a single button controls.

### With access to the receiver box - mode I

1. Open your new remote file
2. Press and hold the button on the receiver for at least 3 sec
3. Release it when the led lights up
4. Within 10 seconds press `Send` on the Flipper and hold for at least 2 sec
5. The led on the receiver flashes 3 times - the remote is stored
6. To add more remotes, repeat step 4 within another 10 seconds. Otherwise the phase ends by itself
7. Done!

### With access to the receiver box - mode II

1. Open your new remote file
2. Press the button on the receiver as many times as the output you want - twice for output 2, for example
3. Check that the led flashes that same number of times
4. Within 10 seconds press the Flipper key you want to bind to that output and hold for at least 2 sec
5. The led on the receiver flashes 3 times - the remote is stored
6. Done!

### With a remote that already works

You do not need the receiver box, but the first remote always has to be registered with the receiver button. The new remote **inherits the mode** of the old one, so if the old one was stored in mode II you have to press the matching buttons on both remotes.

1. Stand close to the receiver
2. Open your new remote file, press and hold `Send` for at least 5 sec, then release
3. On the already working remote, press and slowly release its button 3 times
4. Open your new remote file again and press and slowly release `Send` one time
5. Done?

> [!NOTE]
> These receivers cannot delete one single remote - only all of them at once.

---

## Security+ 1.0 / 2.0 (Chamberlain, LiftMaster, Craftsman)

- **Create it:** `SubGHz` -> `Add Manually` -> `Security+2.0` or `Security+1.0` with your frequency

**Which one do you have?** The colour of the `Learn` button on the motor head tells you:

| Learn button | Create this |
|---|---|
| Yellow | `Security+2.0` |
| Purple | `Security+1.0 315MHz` |
| Green | `Security+1.0 390MHz` |
| Red / orange | Billion Code - not Security+ |
| Grey / white | dip switch remote - not Security+ |

1. Open your new remote file
2. Find the `Learn` button on the motor head - usually on the back or the side panel, sometimes behind the light lens
3. Press and release it once. The learn indicator led glows steadily for 30 seconds
4. Within those 30 seconds, press and hold `Send` on the Flipper
5. Release it when the motor lights flash or you hear two clicks - the remote is programmed
6. Press `Send` again to check the door moves
7. Done?

> [!TIP]
> If the button on the motor head will not cooperate, some systems let you program from the wall console instead.

---

## Somfy Keytis

- **Also sold as:** Keytis NS 2 RTS / KeyGo 4 RTS
- **Create it:** `SubGHz` -> `Add Manually` -> `Somfy Keytis 433MHz` (this is **433.42 MHz**, not 433.92)
- **Programming key:** **Up Arrow** = `0x3` - `Prog` (`Send` is `Key_1`)

On an original Keytis there is no `Prog` key on the front - you have to bridge the two pads marked `PROG` on the back of the board with a screwdriver. The Flipper just needs the Up Arrow.

### With access to the receiver

1. Open your new remote file
2. Press and hold the `PROG` button on the receiver for ~3 sec, until it blinks
3. Press **Up Arrow** (`Prog`) on the Flipper
4. The blind or gate moves briefly up and down - the remote is registered
5. Done?

### With a remote that already works

1. Open your new remote file
2. Put the already registered remote into programming mode until the blind or gate moves briefly up and down. On an original Keytis that means bridging the `PROG` pads on the back of the board. On the Flipper it is **Up Arrow**
3. Open your new remote file and press **Up Arrow** (`Prog`) until the blind or gate moves briefly up and down again
4. Done?

> [!WARNING]
> On many Somfy receivers, holding the programming button for more than ~7 seconds erases every remote and sensor the receiver knows.

> [!NOTE]
> Keytis uses the newer `NS` radio protocol. An older non-NS RTS receiver will not accept it until the remote's protocol is switched over - see Somfy's own documentation for your receiver.

---

## Somfy Telis

- **Create it:** `SubGHz` -> `Add Manually` -> `Somfy Telis 433MHz` (this is **433.42 MHz**, not 433.92)
- **Programming key:** **Left Arrow** = `0x8` - `Prog`

1. Open your new remote file
2. Long press the `Prog` button on a remote that is already registered to the device, until the blinds move briefly up and down
3. Press and hold **Left Arrow** (`Prog`) on the Flipper, until the blinds move briefly up and down again
4. Done?

---

## Sommer

- **Also sold as:** TX03-868-4 / TX03-868-2 (SOMloq) - Pearl, Duo, Sprint, Marathon drives
- **Read with:** modulation `FM12K` or `FM476`, whichever one decodes your remote
- **Create it:** pick the entry that matches how your remote read - the `fm2` entries are the `FM12K` ones, the plain `KL: Sommer` entries are `FM476`:
  - `KL: Sommer fm2 868Mhz` / `KL: Sommer fm2 434Mhz` for FM12K
  - `KL: Sommer 868MHz` / `KL: Sommer 434MHz` for FM476
- **Programming key:** Right Arrow = `0x6`

### With access to the receiver

1. Open your new remote file
2. Find the `learn` / `code` button on the receiver. Where it lives depends on the drive - under the light cover on a Duo, behind the switch box cover on a Sprint, under a clear plastic cover next to the code button on a Marathon
3. Press it briefly - the receiver led lights up
4. Press `Send` on the Flipper and hold it. The receiver led starts blinking fast - release the key then
5. The receiver led goes out - the remote is programmed
6. Done?

### On a sliding gate operator

Press the `Radio` button on the control unit instead - a red led confirms programming mode. Press it again to pick the channel, then hold `Send` on the Flipper until the channel led flashes and goes dark. The receiver leaves programming mode by itself after ~30 seconds with no signal.

---

## V2 Phoenix (Phox)

- **Also sold as:** Phoenix 2/4, Phox 2/4, Handy 2/4, TXC2-4, TRC2-4, TSC2-4 - receivers RXP, MR2
- **Create it:** `SubGHz` -> `Add Manually` -> `V2 Phoenix 433MHz`
- **Programming key:** Right Arrow = `0x3` - on the original remote this is buttons 1+2 (or 1+3) held together

> [!IMPORTANT]
> This mapping has **not been confirmed by anyone yet**. If it does not work, register the remote at the receiver button instead and let us know in the issues tab.

### With a remote that already works

1. Open your new remote file
2. Open the file of the remote that is already registered, press **Right Arrow** and hold for at least 5 sec, then release
3. Within 5 seconds, open your new remote file and press `Send`
4. Release it. Repeat for any other buttons you want to store
5. Done?

The remote that starts this has to be stored in the receiver already. Everything stored this way inherits the logic of the remote that opened the window - if that one only had button 1 stored, the new remotes can only be stored on button 1.

> [!NOTE]
> Some V2 receivers have an option to enable `Static` mode, which makes them ignore the rolling part of the key.

---

## Recovering a Seed with the Seed Capturer app

**FAAC SLH**, **Genius** and **Erreka** all build their rolling code from a secret per-installation **Seed**. Without it the Flipper cannot decode those remotes at all - a normal button press reads as `KL: Unknown` or will not decode - and it cannot build a remote the receiver will accept.

The **Seed Capturer** app collects the raw material a Seed recovery needs. It does not recover the Seed itself and it **never transmits anything** - it only listens and writes a file.

### Why you would want it

- **The receiver's remote programming is switched off.** Some installers disable radio programming on the receiver. Then no amount of button pressing registers a new remote, and a clone of a remote the receiver already knows is the only way in. A clone needs the Seed
- **Your remote is a slave, not a master.** On FAAC and Genius only a master remote can transmit the programming code that opens the window for new remotes. A slave cannot, so the master-remote procedure is simply unavailable to you. Again, a clone is the way, and a clone needs the Seed
- **You cannot get at the receiver.** It is walled in, in a locked box, or not yours to open
- **Erreka with a shared Seed.** See [Erreka IRIS](#erreka-iris-new) - some installations give every remote the same Seed, and a new remote has to reuse it

### How to use it

1. Open `Apps` -> `Sub-GHz` -> `Seed Capturer`
2. Pick `New capture`
3. Pick your remote type - `FAAC SLH`, `Genius` or `Erreka`
4. Pick your frequency - `433.92 MHz  AM650` or `868.35 MHz  AM650`
5. Press **the same button on the same remote**, over and over. Roughly once a second is fine
6. Watch the screen fill in:
   - `Fix` - the remote's fixed part. The app locks onto the first one it hears and ignores everything else
   - `Hops n/10` - how many **different** rolling parts it has collected, and the raw packet count next to it
   - the status line shows `Hop n: XXXXXXXX` each time a new one lands
7. Once you have at least 2 hops, `Save` appears on the centre key. Press it
8. The app tells you the file name it wrote
9. **The Flipper's part is done here - it does not recover the Seed itself.** Copy the capture file to your phone or PC and open it with the Seed recovery tool in the [qUnleashed](https://github.com/DarkFlippers/qUnleashed) companion app. That tool does the actual search and gives you the Seed
10. Enter that Seed back on the Flipper, see [Once you have the Seed](#once-you-have-the-seed) below

### Reading the screen

| What you see | What it means |
|---|---|
| `Hop n: XXXXXXXX` | a new rolling part was stored, keep pressing |
| `Other remote XXXXXXXX` | that press came from a different remote or a different button, and was ignored |
| `Have 10, enough` | the app is full, press `Save` |
| `Press remote 2+ times` | nothing collected yet |

Press **Left** (`Reset`) to throw the capture away and lock onto a different remote or button.

### How many presses

**Two hops is the minimum to save. More is better** - every extra hop narrows the search down and makes it finish faster. Collect 5-10 if the remote is in your hand.

What matters as much as the count is that they are **consecutive presses with nothing missed**. The recovery works on an unbroken run of the counter, so a press that never reached the Flipper leaves a gap and the search will find nothing. Keep the remote next to the Flipper and watch that the `Hops` counter moves on every single press.

### Where the file goes

`/ext/apps_data/subghz_seed_captures/`, named `<Type>_<Fix>_<date-time>.txt`, for example `Erreka_F00F1C9B_261008-142233.txt`. It is a plain text file holding the manufacturer, protocol, frequency, preset, the `Fix` and every `Hop` in the order they arrived.

### Recovering the Seed in qUnleashed

The capture file is only the input. The Seed itself is found by the recovery tool inside [qUnleashed](https://github.com/DarkFlippers/qUnleashed), the companion app for phone and PC:

1. Copy the capture file (or the whole `subghz_seed_captures` folder) off the Flipper
2. Open **qUnleashed** and go to its Seed recovery tool
3. Load the capture file - the manufacturer, frequency, `Fix` and `Hop` list are all already in it, so there is nothing else to fill in
4. Start the search and wait. It is a brute force search, which is exactly why it runs there and not on the Flipper
5. It returns your Seed, or nothing at all

> [!TIP]
> If it finds nothing, the capture most likely had a gap in it. The search needs an unbroken run of presses, so capture again and be precise about it: hold the remote right next to the Flipper, press the **same button** and nothing else, one steady press at a time, and check the `Hops` counter moves on **every** press. If a press does not register, hit `Reset` (Left) and start the capture over instead of carrying on.

> [!IMPORTANT]
> The Seed belongs to the installation, not to one button. **On FAAC SLH and Genius the Seed is different for every button** on the original remote, so capture the button you actually intend to use. On Erreka the Seed is per remote, and sometimes per whole installation.

### Once you have the Seed

- **To make a new remote** (needs the receiver to accept new remotes): `SubGHz` -> `Add Manually [Advanced]`, pick your type, and enter your own `SERIAL` with the recovered `SEED`
- **To make a clone** (works without any programming, because the receiver already knows that serial): enter the **original remote's** `SERIAL` and `BUTTON`, the recovered `SEED`, and a `COUNTER` a little above the original's current value

> [!CAUTION]
> A clone shares the counter with the original remote, so the two fight over it - whichever you pressed last works and the other needs several presses to catch up. Only clone when making a new remote is not possible.

---

## Programming / hidden button reference

Lots of receivers are programmed by pressing a button the original remote does not show you - a recessed pin hole, two buttons held together, pads you bridge inside the case. The Flipper sends those codes as one single key press.

**How to use this table:** find the "put the receiver into programming mode" step in your own manufacturer's manual, then press the key from this table in place of whatever the manual tells you to press on the original remote.

> [!NOTE]
> These are the keys on a remote as `Add Manually` creates it. If you change the button code by hand in the signal settings, the keys move with it.

| System (name in the `Add Manually` menu) | Code | Key on the Flipper | What it is on the original remote |
|---|---|---|---|
| `FAAC SLH 433/868MHz`, `Genius 433/868MHz` | - | **Up Arrow** | the programming signal |
| `BFT Mitto 433MHz` | `0xF` | Right Arrow | pin hole on the back, or buttons 1+2 held |
| `Erreka 433MHz` | `0xF` | Right Arrow | pins 1 and 5 of the 5 way connector bridged |
| `KL: DEA Mio 433MHz` | `0xF` | Right Arrow | hidden button |
| `KL: Aprimatic 433MHz` | `0xF` | Right Arrow | all 4 buttons held together |
| `KL: Mhouse 433MHz` | `0xF` | Right Arrow | extra / hidden button |
| `KL: Nice Smilo 433MHz` | `0xB` | Right Arrow | extra / hidden button |
| `KL: FAAC RC,XT 433/868MHz` | `0xB` | Right Arrow | buttons 1+2 held (master press) |
| `KL: Genius TX4RC 433M.` | `0xB` | Right Arrow | programming mode button |
| `KL: Monarch 433MHz` | `0xB` | *no arrow sends it* | set the button code by hand in signal settings |
| `KL: Novoferm 433MHz` | `0x9` | Right Arrow | extra / hidden button |
| `KL: Stilmatic 433MHz` | `0x9` | Right Arrow | extra / hidden button |
| `KL: Sommer` 434/868MHz and `fm2` | `0x6` | Right Arrow | extra / hidden button |
| `KL: Hor. EcoStar 433MHz` | `0x6` | **Down Arrow** | extra / hidden button |
| `KL: Cardin S449 433MHz` | `0xD` | Right Arrow | button in the hole under the keys |
| `AN-Motors AT4 433MHz` | `0xC` | Right Arrow | extra / hidden button |
| `Beninca ARC 433MHz` | `0x0` | **Down Arrow** | hidden button (buttons 1+2 held) |
| `Ditec GOL4 433MHz` | `0x0` | Right Arrow | hidden button |
| `Jarolift 433MHz` | `0x1` | **Up Arrow** | `Learn` - Up+Down held, then Stop |
| `Somfy Telis 433MHz` | `0x8` | **Left Arrow** | `Prog` button |
| `Somfy Keytis 433MHz` | `0x3` | **Up Arrow** | `Prog` pads on the back of the board |
| `V2 Phoenix 433MHz` | `0x3` | Right Arrow | buttons 1+2 or 1+3 held *(unconfirmed)* |
| `Nice FloR-S` / `Nice One 433MHz` | `0x3` | Right Arrow | button 3 *(unconfirmed as a programming button)* |

### Everything else in the menu

Anything not in this table and without a section above is registered the plain way: **put the receiver into learning mode with its own button, then press `Send` on the Flipper inside the time window your manufacturer gives.**

That covers most KeeLoq systems - Comunello, Allmatic, Motorline, Centurion, KEY, Jolly Motors, IronLogic, DTM Neo, Gibidi, GSN, HomeGate, Elmes, Normstahl, JCM Tech, Pujol, ET Blue, ATA PTX4, Seav, Wisniowski, Fadini, Mc Garcia, Clemsa Mutancode, Doormatic, Elvox, Verex, CAME Space, Beninca KeeLoq, Superrollo, HCS101 - and every static protocol, such as CAME, Nice Flo, Marantec, Princeton, Prastel, Roger, Telcoma EDGE, Gate TX, BETT, Nord ICE, Revers RB2, Linear, Nero and ZKTeco.

---

## When it does not work

**Nothing happens at all.** Check the frequency first - it is the most common mistake. Read your original remote in `SubGHz` -> `Read` and compare. Some brands sell the same remote on two frequencies (DoorHan on 315 and 433.92, FAAC and Genius on 433.92 and 868.35).

**Wrong modulation.** Most systems here are `AM650`, but a few are not - Cardin S449 is `FM12K`, Sommer is `FM12K` or `FM476`. Set it in `SubGHz` -> `Read` -> `Config`.

**The receiver reacts but never stores the remote.** You are probably missing the time window. Re-read the steps and have the remote file already open before you touch the receiver button.

**It stored the remote but nothing opens.** Hold `Send` longer. The Flipper transmits only while a key is held, and several protocols need 1-3 seconds to get a full code out. CAME Atomo needs at least 2 seconds every time.

**It worked once and then stopped.** On a rolling code system that is a counter problem, and it means you cloned a remote instead of making a new one. Press `Send` a few more times in a row - the receiver accepts a counter that ran ahead, within a window. If you want both remotes working at the same time, create a new remote instead of cloning. This cannot happen on a static code remote, where there is no counter at all - if a static remote stops working, look at the battery, the frequency, or the receiver.

**All of your remotes stopped working.** You most likely held a programming button long enough to trigger the receiver's erase function. Re-register every remote, originals included.

---

#### Sources and further reading

- [FAAC SLH - video walkthrough](https://www.youtube.com/watch?v=NfZmMy37XUs)
- [Somfy RTS protocol writeup](https://pushstack.wordpress.com/somfy-rts-protocol/)
- [Somfy Universal Receiver RTS - installation instructions](https://service.somfy.com/downloads/nam_v5/universalreceiver_rts.pdf)
- [Somfy Keytis NS 2 RTS registration - Somfy's own forum](https://forum.somfy.fr/questions/1792206-programmer-telecommande-keitis-ns2-rts-recepteur-somfy-axorn-50)
- [BFT Mitto manual](https://www.retroremotes.com.au/wp-content/uploads/2017/03/BFT-MITTO-2-4-19-6-17.pdf)
- [NICE FLOX2R receiver programming](https://apollogateopeners.com/store/pdf/apollo-flor-s-receiver-programming-guide.pdf)
- [Nice Flor S programming](https://motepro.com.au/Instructions/Nice.pdf)
- [Nice Smilo SM2/SM4 and SMXI receiver manual](https://www.habitat-automatisme.com/blog/wp-content/uploads/2010/02/SMILO.pdf)
- [Mhouse GTX4 / TX4 / Nice Home ECCO5 programming](https://www.allotelecommande.com/wp-content/uploads/2019/06/Notice-de-programmation-MHOUSE-GTX4-%E2%80%93-TX4-%E2%80%93-MT4-%E2%80%93-NICE-HOME-ECCO5-.pdf)
- Erreka IRIS - `Guia rapida mando IRIS` (MSR-044/03), the quick guide that ships with the remote, from [www.erreka.com](https://www.erreka.com/)
- [Jarolift TDEF motor manual](https://www.jarolift.de/wp-content/themes/jarolift/manuals/motoren/en/Jarolift_TDEF_EN.pdf)
- [Beninca TO.GO transmitter programming](https://www.allotelecommande.com/wp-content/uploads/2023/05/BENINCA-TO.GO_.pdf)
- [Ditec BIX L / BIX A receiver manual](https://www.allotelecommande.com/wp-content/uploads/2023/03/DITEC-BIX-L-DITEC-BIX-A.pdf)
- [Cardin S449 QZ2 programming notice](https://www.allotelecommande.com/wp-content/uploads/2024/02/CARDIN-S449-QZ2.pdf)
- [Cardin RCQ449 receiver manual](https://www.allotelecommande.com/wp-content/uploads/2023/02/Recepteur-Radio-CARDIN-RCQ449-ND00.pdf)
- [Hormann RSC2 hand transmitter manual](https://www.hoermann.de/fileadmin/_country/dok/Handsender_RSC2_%28H%C3%B6rmann%29_TR20L001-A.pdf)
- [Sommer - teaching a transmitter to a sliding gate operator](https://www.sommer.eu/en/magazine/programming-a-handheld-transmitter-to-a-sommer-sliding-gate-operator.html)
- [V2 RXP4 receiver manual - radio learning](https://manualslib.mx/manual/401640/V2-Rxp4.html?page=9)
- [KingGates Fred receiver programming](https://tecnoparking.com/tienda/gb/receptors/2096-outer-receiver-kinggates-fred-2-channels-rollingcode-is-433-mhz.html)
- [FAAC XT2 433 SLH master remote copy procedure](https://dieffematic.com/en/products/original-faac-xt2-433-slh-gate-opener-remote-control-white-master-787007-white)
- [AN-Motors AT4 - video walkthrough (Russian)](https://www.youtube.com/watch?v=URVMtTELcnU)
- [Alutech AT4N - video walkthrough (Russian)](https://www.youtube.com/watch?v=XrOVVYhFXDg)
- [DoorHan - video walkthroughs (Russian)](https://www.youtube.com/watch?v=wZ5121HYv50) and [second one](https://www.youtube.com/watch?v=1ucrDKF3vWc)
- [BFT Mitto - video walkthrough](https://www.youtube.com/watch?v=5QXMBKI_-Ls)
- [CAME Atomo - video walkthrough (Russian)](https://www.youtube.com/watch?v=XeHUwfcSS30)

---

*Docs made for Unleashed FW, please mention the source when copying*
