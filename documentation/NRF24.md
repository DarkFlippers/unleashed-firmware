# flipperzero-nrf24

## Author: [mothball187](https://github.com/mothball187/flipperzero-nrf24/tree/main/mousejacker)

An [NRF24](https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf) driver for the [Flipper Zero](https://flipperzero.one/) device. The NRF24 is a popular line of 2.4GHz radio transceivers from Nordic Semiconductors. This library is not currently complete, but functional.

# How to use
- Connect NRF24 to flipper using provided pinouts
- Open NRF24: Sniffer, and scan channels, switch between modes/channels using buttons
- When you got address -> Open NRF24: Mouse Jacker
- Select Address and open badusb file
- Done

# Demo (YouTube)
[![YouTube](https://img.youtube.com/vi/C5hbyAjuU4k/0.jpg)](https://www.youtube.com/watch?v=C5hbyAjuU4k)

## Warning
These apps are for **educational purposes** only. Please use this code responsibly and only use these apps on your own equipment.

## Acknowledgments
The NRF24 sniffing technique was discovered and shared by Travis Goodspeed in [his blog](http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html).

The mousejack vulnerabilities were discovered and reported by Marc Newlin, see [the blog](https://www.bastille.net/research/vulnerabilities/mousejack/technical-details) for technical details.

Much of the driver code was inspired by [RadioHead's Arduino library](https://www.airspayce.com/mikem/arduino/RadioHead/classRH__NRF24.html).
Much of the mousejack code was inspired by the [Jackit project](https://github.com/insecurityofthings/jackit).


# Pinout from from NoComp/Frog
<img src="https://media.discordapp.net/attachments/937479784726949900/994495234618687509/unknown.png?width=567&height=634">

# Mousejacker / NRF24 pinout by UberGuidoZ
2/A7 on FZ goes to MOSI/6 on nrf24l01<br>
3/A6 on FZ goes to MISO/7 on nrf24l01<br>
4/A4 on FZ goes to CSN/4 on nrf24l01<br>
5/B3 on FZ goes to SCK/5 on nrf24l01<br>
6/B2 on FZ goes to CE/3 on nrf24l01<br>
8/GND on FZ goes to GND/1 on nrf24l01<br>
9/3V3 on FZ goes to VCC/2 on nrf24l01<br>
IRQ/8 is left disconnected on nrf24l01<br>

![NRF_Pins](https://user-images.githubusercontent.com/57457139/178093717-39effd5c-ebe2-4253-b13c-70517d7902f9.png)

If the nRF module is acting a bit flakey, try adding a capacitor to the vcc/gnd lines! 
I've not tried the Plus model so it may have a bigger need for a cap. 
Otherwise, I haven't had any major issues. 
Anything from a 3.3 uF to 10 uF should do. (Watch your positive/negative placement! Negative to ground.) 
I learned if you wanna get fancy, include a 0.1 uF cap in parallel. 
The 3.3 uF to 10 uF will respond to slow freq changes while the 0.1 uF will respond to the high freq switching spikes that the larger one cannot. That said, a single 10 uF will likely suffice for the Mousejack attack. ¯\\\_(ツ)_/¯

![NRF_Capacitor](https://user-images.githubusercontent.com/57457139/178169959-d030f9a6-d2ac-46af-af8b-470ff092c8a7.jpg)
