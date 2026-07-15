# RFID — Test Cases

## 1. Checking cards and protocols. EM Marine 4100

Steps:

1. Go to RFID.
2. Shared step [1]: Read.
    1. Read the subject using the flipper.

        **Expected result:**
        1. The subject read successfully.
        2. The information on the read key corresponds to the original.

3. Shared step [2]: Emulation.
    1. Emulate the read key and read it as another flipper.

        **Expected result:**
        1. The key was considered another flipper.
        2. The information on the key is identical to the original.

4. Shared step [3]: Save.
    1. Save previously read key.

        **Expected result:**
          1. The key was saved with the specified name.
          2. After saving, the scene changed to the "Saved" directory.

5. Shared step [4]: Write.
    1. Write a previously saved key to another storage device using the "write" function.

        **Expected result:** the card was recorded, a success scene appeared.

6. Shared step [5]: Read the written.
    1. Read the media on which the key was previously written.

        **Expected result:**
          1. The information from the carrier was considered.
          2. The data is identical to the stored key that was written.

## 2. Checking cards and protocols. Motorola Indala26

Steps:

1. Go to RFID.
2. Shared step [4]: Write.
    1. Write a previously saved key to another storage device using the "write" function.

        **Expected result:** the card was recorded, a success scene appeared.

3. Shared step [2]: Emulation.
    1. Emulate the read key and read it as another flipper.

        **Expected result:**
        1. The key was considered another flipper.
        2. The information on the key is identical to the original.

4. Shared step [3]: Save.
    1. Save previously read key.

        **Expected result:**
        1. The key was saved with the specified name.
        2. After saving, the scene changed to the "Saved" directory.

5. Shared step [4]: Write.
    1. Write a previously saved key to another storage device using the "write" function.

        **Expected result:** the card was recorded, a success scene appeared.

6. Shared step [5]: Read the written.
    1. Read the media on which the key was previously written

        **Expected result:**
        1. The information from the carrier was considered.
        2. The data is identical to the stored key that was written.

## 3. Checking cards and protocols. HID H10301 (Picopass)

Steps:

1. Go to RFID.
2. Shared step [1]: Read.
    1. Read the subject using the flipper.

        **Expected result:**
        1. The subject read successfully.
        2. The information on the read key corresponds to the original.

3. Shared step [2]: Emulation.
    1. Emulate the read key and read it as another flipper.

        **Expected result:**
        1. The key was considered another flipper.
        2. The information on the key is identical to the original.

4. Shared step [3]: Save.
    1. Save previously read key.

        **Expected result:**
        1. The key was saved with the specified name.
        2. After saving, the scene changed to the "Saved" directory.

5. Shared step [4]: Write.
    1. Write a previously saved key to another storage device using the "write" function.

        **Expected result:** the card was recorded, a success scene appeared.

6. Shared step [5]: Read the written.
    1. Read the media on which the key was previously written.

        **Expected result:**
        1. The information from the carrier was considered.
        2. The data is identical to the stored key that was written.

## 4. Checking cards and protocols. Indala26

Steps:

1. Go to RFID.
2. Shared step [1]: Read.
    1. Read the subject using the flipper.
        
        **Expected result:**
        1. The subject read successfully.
        2. The information on the read key corresponds to the original.

3. Shared step [2]: Emulation.
    1. Emulate the read key and read it as another flipper.

        **Expected result:**
        1. The key was considered another flipper.
        2. The information on the key is identical to the original.

4. Shared step [3]: Save.
    1. Save previously read key.

        **Expected result:**
        1. The key was saved with the specified name.
        2. After saving, the scene changed to the "Saved" directory.

5. Shared step [4]: Write.
    1. Write a previously saved key to another storage device using the "write" function.

        **Expected result** the card was recorded, a success scene appeared.

6. Shared step [5]: Read the written.
    1. Read the media on which the key was previously written.

        **Expected result**
        1. The information from the carrier was considered.
        2. The data is identical to the stored key that was written.

## 5. Checking cards and protocols. Troyka/Podorozhnik

Steps:

1. Shared step [1]: Read.
    1. Read the subject using the flipper.

        **Expected result:**
        1. The subject read successfully.
        2. The information on the read key corresponds to the original.

2. Check the work of the troika parser.
   
    **Expected result:** when reading a card, a list of additional information specific to cards of this type is loaded.

3. Shared step [2]: Emulation.
    1. Emulate the read key and read it as another flipper.

        **Expected result:**
        1. The key was considered another flipper
        2. The information on the key is identical to the original.

4. Shared step [3]: Save.
    1. Save previously read key.

        **Expected result**
        1. The key was saved with the specified name.
        2. After saving, the scene changed to the "Saved" directory.

## 6. RFID Reading

Steps:

1. Go to RFID → Reading, count cards of standards: indala, em-marine, HID.

    **Expected result:** cards of all standards were considered.

2. Save all read cards.

    **Expected result:** cards saved.

## 7. RFID recording on T5577

Steps:

1. Read RFID card or select a previously saved one.
2. Write the selected card to the T5577 More → Write.

    **Expected result:** the card has written.

3. Write 3 different types of cards and check reading from T5577.

    **Expected result:** the reader identified the cards, the data matched.

## 8. RFID Add Manually

Steps:

1. Go to RFID → Add Manually.
2. Create EM4100 card.
3. Create map H10301.
4. Create card I40134.
5. Read created cards by another flipper or on a stand.

    **Expected result:** the cards were counted, the data agreed.

## 9. RFID Emulation

Steps:

1. Go to RFID → Saved, Select previously saved keys.
2. Select the saved card Indala → Emulate, read with the second flipper, with another firmware.

    **Expected result:** the ID on the reading flipper matches the ID that the Flipper emulates.

3. Select the saved card EM-Marine → Emulate, read with the second flipper, with another firmware.
  
    **Expected result:** the ID on the reading flipper matches the ID that the Flipper emulates.

4. Select the saved card HID → Emulate, read with the second flipper, with another firmware.

    **Expected result:** the ID on the reading flipper matches the ID that the Flipper emulates.
