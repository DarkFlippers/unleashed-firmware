# CLI — Test Cases

## 1. CLI. Control commands

Steps:

1. Run Power command.

   **Expected result:** a list of available commands is displayed.

2. Execute all commands from the list.

   **Expected result:**

     1. Power OFF - when the cable is disconnected, the flipper will turn off.
     2. Power reboot - the flipper will reboot.
     3. Power reboot to dfu - the flipper will switch to dfu mode.
     4. Power 5V 1 - will supply power to the 5V line of the top comb.

3. Run GPIO command.

   **Expected result:** a list of available commands is displayed.

4. Execute all commands from the list

   **Expected result:**
     1. Mode - 1 - transmit mode, 0 - receive mode.
     2. gpio_set PA7 1 When the debug board is inserted, the required LED lights up.

5. Run LED command, for example led b 255.

   **Expected result:** LED lights up blue.

6. Command vibro

   **Expected result:**
     1. vibro 1 - vibration turns on.
     2. vibro 0 - vibration turns off.

## 2. CLI. Information commands

Steps:

1. Call CLI minicom -D /dev/ttyACM0 -b 230400 or Open a terminal with Putty.
2. Enter the log command and enter the option that is needed, for example debug.

    **Expected result:** when you press keys, enter menus, open applications, all this is displayed in the log.

3. Press Ctrl + C to end logging.

    **Expected result:** logging has stopped.

4. On the main screen, press DOWN for 3 seconds to open the FW version info and Boot version info menu.

5. Run the device_info or ! command and compare the data.

    **Expected result:**
      1. FW data matches.
      2. Bootloader open" data matches.

6. Run the command bt hci_info.

    **Expected result:** after the bt command cmd list is displayed.

7. Compare the obtained valueswith the reference ones:

   Ret: 0, HCI_Version: 11, HCI_Revision: 16484, LMP_PAL_Version: 11, Manufacturer_Name: 48,
   LMP_PAL_Subversion: 8548

    **Expected result:** data matches.

8. Run DATE command.

    **Expected result:** real time and date displayed.

9. Run the commands help and ?.

    **Expected result** a description of all available commands appears.

10. Execute the FREE command.

    **Expected result:** free space in internal memory is displayed.

11. Run the FREE_BLOCKS command.

    **Expected result:** block addresses appeared.

## 3. CLI. Storage Commands

Steps:

1. Insert SD card with data.

2. Run the command storage info /ext.

   **Expected result:** there is information about free space.

3. Run the command storage format /ext.

   **Expected result:** confirmation that the card has been formatted appears.

4. Refuse formatting.

   **Expected result:** the card is not formatted, the free space has not changed.

5. Run the storage format /ext command again and confirm formatting.

   **Expected result:** the card is formatted.

6. Run the command storage info /ext.

   **Expected result:** while the card is inside, SD status: OK.

7. Remove the SD card and send the command storage info /ext.

   **Expected result:** when the card is removed: SD card not mounted: If an SD card is inserted, pull it and reinsert it.

8. Run the factory_reset command, and when asked All data will be lost. Are you sure (y/n)? enter n.

   **Expected result:** a data loss warning appears and nothing happens if you disagree.

9. Run the factory_reset command again, when asked All data will be lost. Are you sure (y/n)? enter y.

   **Expected result:**
     1. When confirmed, the data is erased.
     2. A welcome message appears on the main screen, settings are reset.
     3. Data on SD card is not erased.
     4. Settings reset.
     5. Time hasn't reset.
