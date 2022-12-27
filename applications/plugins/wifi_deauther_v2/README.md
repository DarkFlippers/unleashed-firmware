# flipperzero_esp8266_deautherv2
Flipper Zero esp8266 deauther app.


Based off the WiFi Marauder App from 0xchocolate.

Thanks to Roguemaster for fixing some issues I had with the code and didnt get a chance to get to. I have now uploaded these changes into the source. 

I have also successfully built this with unleashed souce as well and included the FAP file here. unleashed version unlshd-020.

https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion

https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/wifi_marauder_companion

uses the Version 2 of the ESP8266 Deauther code.
https://github.com/SpacehuhnTech/esp8266_deauther/tree/v2/esp8266_deauther

This is done so you can use the original deauther v2 firmware on the esp8266.
you can just flash the latest binary.

also a shout out to https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module
This is already in the Roguemaster firmware and just needs to be enabled and compiled. unfortunatly I could not get this past the menu when I compiled his deauther source for the nodemcu. Nice menu though.

I used a nodeMCU board. Wiring is simple. follow the wiring guide on https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module
On mine I connected one G to ground, VIN to 5V, RX to U_TX, TX to U_RX.

NodeMCU---FlipperZero

G---------GND

VIN-------5V

RX--------U_TX

TX--------U_RX



Video in action (old version).
https://youtu.be/_RFzZyPkeR0

New video and install instructions.
https://youtu.be/CKK7t0TaRVQ

If you want to disable the built in WiFi access and web interface (only use flipper to send serial commands) then select "set webinterface false", "save settings" and "reboot". When it starts back up you wont see the pwned AP any more.

I installed this into Roguemaster to test.

git clone --recursive https://github.com/RogueMaster/flipperzero-firmware-wPlugins.git
cd flipperzero-firmware-wPlugins/

copy folder into applications.
add "APPS_wifi_deauther", to the meta/application.fam file.

compile
./fbt resources icons
./fbt updater_package
