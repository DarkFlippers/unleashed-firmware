# Reading RAW RFID data {#lfrfid_raw}

Flipper Zero has the option to read RAW data from 125 kHz cards that allows you to record the card's data and save it, similar to how a dictaphone records sound.

To use this function, you need to activate the Debug mode on your Flipper Zero by doing the following:

1. Go to **Main Menu** → **Settings** → **System**.

2. Set **Debug** to **ON**.

Once the Debug mode is activated on your Flipper Zero, you can read RAW data from 125 kHz RFID cards:

1. Go to **Main Menu** → **125 kHz RFID** → **Extra Actions**.

2. Select **RAW RFID** data and name the raw file.

3. Read instructions and press **OK**.

4. Apply the card to Flipper Zero's back.

5. Once the reading is finished, press **OK**.

Two files with data (with ASK and PSK modulations) will be saved in the `lfrfid` folder on the microSD card. Now, you can share it and the card's photo with developers by creating an issue on GitHub.
