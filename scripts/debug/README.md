## Installing udev rules

On Linux, unprivileged users need to be in the `dialout` group to access serial ports and other USB devices. 

To add your user to the `dialout` group, run the following command:

```bash
sudo usermod -a -G dialout $USER
```

To install the udev rules needed for debugging & CLI access to Flipper, run the following command:

```bash
sudo cp 41-flipper.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Note that not all possible debug interfaces are listed the `41-flipper.rules` file. If your interface is not supported out of the box, you may need to add a a rule for it. You can do so by adding a new line to the file according to udev rules syntax. Use `lsusb -v` to find the vendor and product IDs of your device.