## Check LF RFID

1. Go to LF RFID workaround app
2. See "LF RFID/OFF/125 kHz" on the screen
3. Read Flipper by EM4100 reader
4. Press ok
5. See "ON" on the screen
6. Check 125 kHz signal on RFID antenna by oscilloscope
7. Take EM4100 tag, place 20 mm from antenna
8. Get signal on demodulator output (RFID_IN)
9. Exit LF RFID app

## Check NFC

1. Go to NFC app
2. See "NFC timeout" on the screen
3. Place NFC tag directly on the antenna
4. See "NFC device found" and check type
5. Exit NFC app

For compatibility check:

1. make LF RFID test
2. make NFC test
3. Reboot device (issue with app loader don't allow run many apps)
4. make NFC test
5. make LF RFID test
