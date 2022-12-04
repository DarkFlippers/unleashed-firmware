# flipperzero-nrf24

An [NRF24](https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf) driver for the [Flipper Zero](https://flipperzero.one/) device. The NRF24 is a popular line of 2.4GHz radio transceivers from Nordic Semiconductors. This library is not currently complete, but functional.

## Warning
This repo contains two Flipper Zero apps that utilize the NRF24 driver to sniff for NRF24 addresses and perform mousejack attacks. These apps are for **educational purposes** only. Please use this code responsibly and only use these apps on your own equipment.

## Acknowledgments
The NRF24 sniffing technique was discovered and shared by Travis Goodspeed in [his blog](http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html).

The mousejack vulnerabilities were discovered and reported by Marc Newlin, see [the blog](https://www.bastille.net/research/vulnerabilities/mousejack/technical-details) for technical details.

Much of the driver code was inspired by [RadioHead's Arduino library](https://www.airspayce.com/mikem/arduino/RadioHead/classRH__NRF24.html).
Much of the mousejack code was inspired by the [Jackit project](https://github.com/insecurityofthings/jackit).


## PinOut from from NoComp/Frog
<img src="https://media.discordapp.net/attachments/937479784726949900/994495234618687509/unknown.png?width=567&height=634">

# Mousejack / NRF24 pinout by UberGuidoZ
2/A7 on FZ goes to MOSI/6 on nrf24l01<br>
3/A6 on FZ goes to MISO/7 on nrf24l01<br>
4/A4 on FZ goes to CSN/4 on nrf24l01<br>
5/B3 on FZ goes to SCK/5 on nrf24l01<br>
6/B2 on FZ goes to CE/3 on nrf24l01<br>
8/GND on FZ goes to GND/1 on nrf24l01<br>
9/3V3 on FZ goes to VCC/2 on nrf24l01<br>
IRQ/8 is left disconnected on nrf24l01

![NRF_Pins](https://user-images.githubusercontent.com/57457139/178093717-39effd5c-ebe2-4253-b13c-70517d7902f9.png)

If the nRF module is acting a bit flakey, try adding a capacitor to the vcc/gnd lines! I've not tried the Plus model so it may have a bigger need for a cap. Otherwise, I haven't had any major issues. Anything from a 3.3 uF to 10 uF should do. (Watch your positive/negative placement! Negative to ground.) I learned if you wanna get fancy, include a 0.1 uF cap in parallel. The 3.3 uF to 10 uF will respond to slow freq changes while the 0.1 uF will respond to the high freq switching spikes that the larger one cannot. That said, a single 10 uF will likely suffice for the Mousejack attack. ¯\\\_(ツ)_/¯

![NRF_Capacitor](https://user-images.githubusercontent.com/57457139/178169959-d030f9a6-d2ac-46af-af8b-470ff092c8a7.jpg)

Selfmade NRF24 breakoutboard:
![NRF_soldered](https://user-images.githubusercontent.com/22019133/183419103-9c40b28b-8152-4212-98e0-9a3358f69518.jpeg) 
![NRF_soldered2](https://user-images.githubusercontent.com/22019133/183419176-26c0359b-0ecb-4015-8f8b-3a75209502ba.jpeg)

## Practical hints from einstein2150
If you are not successfull with the NRF Sniff Plugin you can try to get the MAC of the device with the crazyradio pa USB dongle. Have a look here: https://github.com/SySS-Research/nrf24-playset


A sample output of the NRF-Research script could be like
```
[2022-08-05 13:28:56.366]  Found nRF24 device with address 38:24:93:C0:07 on channel 75\
[2022-08-05 13:28:56.371]  Checking communication\
[2022-08-05 13:28:59.088]  Scan for nRF24 device\
[2022-08-05 13:28:59.097]  Actively searching for address 07:C0:93:24:38\
```

Be carefull with the byte-order using in mousejacker! The correct byte order is the reverse-byteorder. In my example the reversed one is ```07:C0:93:24:38``` for my Logitech R400 presenter.
The next thing which could make trouble is the datarate. In my case the Logitech-device is only responding at 2Mbit. Commands at 1Mbit are not detected.

Now its time to write all the relevant information in the config file. In my case you are creating a file ```addresses.txt``` in the SD directory of ```/nrfsniff```. The file content corresponding to the results of the research is ```07C0932438,2``` representing the reversed byte order of the MAC and the comma-separated datarate.

Start the Mouse Jacker plugin and select the prepared ```addresses.txt```. If everything is alright you are starting with your MAC ready for attack:

![mousejacker start-screen](https://user-images.githubusercontent.com/22019133/183419533-ef5c8df1-e328-45e3-b85d-376513d69c82.jpeg)

If you have troube you can check if a datarate of 1 Mbit will help. Change ```07C0932438,2``` to ```07C0932438,1``` in this case. Another problem in the practical use is electromagnetical noise. In my case the external USB-3.1-Hub creating massive noise in the 2,4 Ghz-Frequency around a distance of 5 cm. Try using a long USB connection-cable for the receiver. In my case the response of the attack raises significant because the signals of the NRF-breakoutboard is no more covered with noise.

