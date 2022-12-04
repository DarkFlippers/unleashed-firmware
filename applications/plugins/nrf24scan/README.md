#  NRF24 scanner with logging and resend ability for Flipper Zero

An [NRF24](https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf) driver for the [Flipper Zero](https://flipperzero.one/) device. The NRF24 is a popular line of 2.4GHz radio transceivers from Nordic Semiconductors.<br>
NRF24L01+ Enhanced ShockBurst packet decoder example using Python: [nrf24_packet_decoder.py](https://raw.githubusercontent.com/vad7/nrf24scan/master/nrf24_packet_decoder.py)<br>
<br>
Flipper Zero FAP file: [Nrf24_Scanner.fap](https://raw.githubusercontent.com/vad7/nrf24scan/master/Nrf24_Scanner.fap)<br>
                                        
Settings file (default addr.txt) format:<br>

Rate: 0/1/2 - rate in Mbps (=0.25/1/2)<br>
Ch: 0..125 - default channel          <br>
ESB: 0/1 (1 - Enhanced ShockBurst)    <br>
DPL: 0/1 (1 - Dynamic Payload Length) <br>
CRC: 0/1/2 (CRC length)<br>
Payload: 1..32 (bytes)                <br>
P0: address pipe #0 in hex (max 5 bytes, LSB last)<br>
P1: address pipe #1 in hex (max 5 bytes, LSB last)<br>
P2: address pipe #2, LSB in hex (1 byte)   <br>
P3: address pipe #3, LSB in hex (1 byte)   <br>
P4: address pipe #4, LSB in hex (1 byte)   <br>
P5: address pipe #5, LSB in hex (1 byte)   <br>
captured data in raw format, first byte = address # 0..5, Payload len if DPL<br>
... up to MAX_LOG_RECORDS-1<br>
<br>
In the list of the received:<br>
Press OK - send the packet,<br>
Long press OK - view addresses.<br>
<br>
<br>
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-1.png">
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-2.png">
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-3.png">
<br><br>
Decode the Packet Control Field and check CRC (long press OK in the list and then press '<' / '>').<br>
ESB (Enhanced Shockburst) option must be turned off.
Press '>' to decode CRC.<br>
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-4.png">
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-5.png">
<br>
1 - pipe #<br>
2 - Payload length (for valide packet must be 1..20 or 33 hex)<br>
3 - PID (2 bit) + NO_ACK (1 bit)<br>
<br>
<br>
<br>
## PinOut from from NoComp/Frog
<img src="https://media.discordapp.net/attachments/937479784726949900/994495234618687509/unknown.png?width=567&height=634">

# NRF24 pinout by UberGuidoZ
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

