# Bootloader test

1. # Clean flash
2. `make -C bootloader flash` `# Load bootloader`
3.  `# reboot device`
    1. Press right
    2. Press left
    3. Wait 0.1 s
    4. Release left
    5. Release right
4. Wait 0.5 s
5. `# Expect no FW`
    1. Expect: no uart welcome message
    2. Expect: red led on
    3. Expect: no USB
6. `# reboot device and go to DFU`
    1. Press left 
    2. Press right
    3. Wait 0.1 s
    4. Release left
    5. Wait 0.5 s 
    6. Release right
7. Wait 0.5 s
8. `# Expect DFU`
    1. Expect: blue led on
    2. Expect: USB: DFU
9. `target_f2/deploy-dfu.sh` `# load FW`
10. `# reboot device`
    1. Press right
    2. Press left
    3. Wait 0.1 s
    4. Release left
    5. Release right
11. Wait 0.5 s
12. `# Expect FW`
    1. Expect: uart welcome message
    2. Expect: USB Flipper CDC