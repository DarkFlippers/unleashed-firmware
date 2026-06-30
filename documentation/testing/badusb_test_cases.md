# BadUSB — Test Cases

## 1. Bad USB Mode

Steps:

1. Connect the flipper to the Mac and open Bad usb app.

   **Expected result:** a list of scripts for all platforms is displayed.
2. Make sure that the layout is EN.
3. Run demo script for Mac.

   **Expected result:** the script started and executed on the PC.
4. Go to the config and change the script layout.
5. Rerun the script.

   **Expected result:** the script started and executed on the PC.
6. Run the qFlipper download script.

   **Expected result:** the command line has started with the process of downloading the qFlipper, after which the application will start.
7. Go to the config and switch the operating mode to BLE (BT must be enabled), and connect the device.

   **Expected result:**
     1. The device was identified in the list of PC devices.
     2. The device connected via BT.

8. Rerun the script.

   **Expected result:** the script started and executed on the PC.

## 2. Bad USB Mode Windows

Steps:

1. Connect the flipper to the Mac and open Bad usb app.

   **Expected result:** a list of scripts for all platforms is displayed.

2. Make sure that the layout is EN.
3. Run demo script for Mac.

   **Expected result:** the script started and executed on the PC

4. Go to the config and change the script layout.
5. Rerun the script.

   **Expected result:** the script started and executed on the PC.

6. Run the qFlipper download script.

   **Expected result:** the command line has started with the process of downloading the qFlipper, after which the application will start.

7. Go to the config and switch the operating mode to BLE (BT must be enabled), and connect the device.

   **Expected result:**
     1. The device was identified in the list of PC devices.
     2. The device connected via BT.

8. Rerun the script.

   **Expected result:** the script started and executed on the PC
