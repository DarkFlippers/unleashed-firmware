# SubGHz — Test Cases

## 1. Sub 1GHz Read Raw

Steps:

1. Sub-GHz → Read RAW → Config.
2. Set frequencies 315, 433, 868 one by one.
3. Record signals from different remote controls.
4. Record the remote control signal from an encrypted protocol (for example, Came Atomo, gray remote control with black buttons).
5. Transmit signals to receivers from the stand one by one.

   **Expected result:** signals were received by receivers on the stand.

## 2. Sub-GHz Frequency Analyzer

Steps:

1. Sub-GHz → Frequency Analyzer.
2. Bring remote controls with different transmission frequencies to the flipper by pressing the button on them.

   **Expected result:** the analyzer correctly displays the frequency at which the remote control operates.

## 3. Sub-GHz External Module

Description:

Testing on external module CC1101.
The module is designed for frequency 433.92, therefore tests must be carried out on it. The module cannot be turned off during operation, this may cause the flipper to freeze.

Steps:

1. Install external module.

   **Expected result:** the module has changed to external.
2. Go to Read.

   **Expected result:** the icon in the upper right corner has changed to an external module.

3. Go to "Read RAW" and receive a signal from the 433 remote control.

   **Expected result:** the signal became stronger than on the built-in antenna.

4. Switch the module to internal, leaving the module in the flipper and go to Read.

   **Expected result:** reading occurs from the internal module.

## 4. Sub-GHz operations with a saved file

Steps:

1. Receive a Sub-GHz signal and save it.

   **Expected result:** file saved.

2. Receive a RAW Sub-GHz signal and save it.

   **Expected result:** file saved.

3. Add previously saved files to favorites.
4. Launch files from the "favorites" submenu.

   **Expected result:** keys are sent without artifacts.

5. Go to Sub-GHz → Saved, and rename previously created files, and go to favorites.

   **Expected result:** the files were renamed and remained in favorites with a new name.

6. Delete previously renamed files.

   **Expected result:** when deleting, a dialog box appeared, after deleting, the "deleted" picture appeared and we were thrown into the "saved" menu.

## 5. Sub-GHz reception

Description:

Checking the reception of radio signals from different remote controls.

Frequencies:
* 315.000 MHz  
* 433.920 MHz
* 868.350 MHz

Steps:

1. Launch the Sub-1 GHz → Read application, tune in one by one to frequencies 315, 433, 868 MHz.

2. Transmit a signal from the remote control with a frequency of 315.000 MHz.

   **Expected result:** a signal with a frequency of 315.000 MHz is received.

3. Transmit a signal from the remote control with a frequency of 433.920 MHz

   **Expected result:** a signal with a frequency of 433.920 MHz is received.

4. Transmit a signal from the remote control with a frequency of 868.350 MHz.

   **Expected result:** a signal with a frequency of 868.350 MHz is received.

5. Go to config and enable hopping. After this, repeat reading all signals.

   **Expected result:** all signals from each frequency were counted.

## 6. Sub-GHz transmission

Description:

Checking signal transmission from the flipper. To send, you can use the keys saved upon receipt. To check, you can use either a second flipper or a stand. 

Frequencies:
* 315.000 MHz
* 433.920 MHz
* 868.350 MHz

Steps:

1. If the keys are already saved, then go to Saved and select the saved keys or take signals from the keys one by one and press "send".

   **Expected result:** the relays on the stand are activated.

## 7. Sub-GHz Add Manually

Steps:

1. Sub-GHz → Add manually.
2. Create signals for all available protocols in this section.
3. Transmit signals to receivers from the stand.

   **Expected result:** signals are being received.
