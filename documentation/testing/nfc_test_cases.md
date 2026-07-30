# NFC — Test Cases

## 1. NFC. Extra Actions. Unlock Ntag/Ultralight

Steps:

1. Auth as. Both types work identically. Bring cards of both types, and wrong type card.

   **Expected result:**
   1. If the card type is correct, the card is successfully readed.

   2. If the card type is incorrect, a message about invalid card type will appear and we will be returned to the reading screen.

2. Save scanned cards.

   **Expected result:** cards saved, after one and a half seconds the scene will change to the "saved" menu.

3. Enter password manually, by attaching a card of the correct type and a card of the wrong type.

   **Expected result:**

     1. A warning window appeared, click continue.
     2. With the correct card, the card can be completely unblocked and not.
     3. When you apply a card of the wrong type, the scene will change to a warning window about the wrong card type.

4. Save scanned cards.

   **Expected result:** cards saved, after one and a half seconds the scene will change to the "saved" menu.

## 2. NFC Detect Reader

Steps:

1. Go to NFC → Detect Reader.
2. Bring the flipper to the active NFC Reader (for example, another flipper).

   **Expected result:** nonces were considered with a light-vibration signal.

3. Press OK.

   **Expected result:** a screen with a QR code appears, which is correctly read by the camera.

## 3. NFC Emulation

Steps:

1. Launch NFC → Saved card → Emulate.
2. Check that Flipper is readable by different NFC readers.

   **Expected result:**
   1. Flipper 10 mm
   2. ProxMark

## 4. NFC Reading

Steps:

1. Launch NFC → Read.
2. Check that Flipper reads different NFC cards.

   **Expected result:** Successful reading of cards:
   * MF Classic
   * Ultralight
   * NTAG
   * DESFire
   * NFC-F
   * EMV bank card

## 5. NFC Timing testing

Steps:

1. Connect proxmark and run the client for it (pm3).
2. Run Mifare Ultralight emulation on the flipper.
3. Read the card and output the log using the commands:
   
   `hf 14a raw -sc 3000`

   `trace list -t 14a --frame -u`

4. Check that timings and communication match the reference:

   **Expected result:** Mifare Ultralight timings are the same.

5. Launch Mifare Classic emulation and execute the commands from step 3 and compare with the reference.

   **Expected result:** Mifare Classic timings are the same.

## 6. NFC Add Manually

Steps:

1. Go to NFC → Add manually.
2. Create maps of all types and save.

   **Expected result:** all cards are preserved.

3. Go to Saved cards and emulate the created cards.

   **Expected result:** the cards were counted and the data matches.

## 7. NFC Extra Actions

Steps:

1. Press "Read Specific card type", and read the key types of cards.

   **Expected result:** the cards were counted, the reading is done in a standard way.

2. Press Mifare Classic Keys, and add some key.

   **Expected result:** keys have been added, a "List" button has appeared on the main screen.

3. Click the "list" button and delete any key.

   **Expected result:** the deleted key disappeared from the list, if the last key is deleted, the "list" button disappears from the main menu.

4. Press "Unlock SLIX", and Enter Password.

   **Expected result:** if the password is correct, by placing the card it will be unlocked.

5. View info, simulate and save map.

   **Expected result:** the info is correct, the emulation works, the card is saved.

6. Apply the Tommy Box toy card, repeat steps 4 and 5 without entering the password.

   **Expected result:** results of steps 4 and 5.
