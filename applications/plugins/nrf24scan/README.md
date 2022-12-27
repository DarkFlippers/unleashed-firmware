#  NRF24 scanner with logging and resend ability for Flipper Zero

An [NRF24](https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf) driver for the [Flipper Zero](https://flipperzero.one/) device. The NRF24 is a popular line of 2.4GHz radio transceivers from Nordic Semiconductors.<br>
NRF24L01+ Enhanced ShockBurst packet decoder example using Python: [nrf24_packet_decoder.py](https://raw.githubusercontent.com/vad7/nrf24scan/master/nrf24_packet_decoder.py)<br>
<br>
Flipper Zero FAP file: [Nrf24_Scanner.fap](https://raw.githubusercontent.com/vad7/nrf24scan/master/Nrf24_Scanner.fap)
<br><br>
___________________________________________________________________________
<br>
<b>Приложение для Flipper Zero, читающее эфир на радиомодулях nRF24L01.</b><br>
Выбор пукта меню - стрелки вверх/вниз, стрелки влево/вправо либо изменют настройки либо управляют видом списка, кнопка ОК либо выбирает режим (короткое нажатие), либо выполняет дополнительное действие (длительное нажатие).<br><br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-1.png"><br>
По умолчанию при запуске включается режим поиска (sniff) - ищутся все валидные пакеты с корректным заголовком и CRC.<br>
Размер CRC и тип пакета (Enhanced ShockBurst или нет) задается. CRC может быть или 1 или 2 байта.<br>
Так как пакеты читаются в RAW формате, то длина полезной нагрузки не может быть больше 23 байт, пакеты с большей длинной не будут пойманы.<br>
В настройках задается минимальный размер нагрузки (payload)<br>
После принятия, пакет сдвигается побитно и валидируется. Побитный сдвиг сильно увеличивает вероятность отлова пакета, но так же увеличивается количество мусорных пакетов.<br>
Количество уникальных адресов запоминается (просмотр - стрелка вниз в режиме просмотра адресов)<br>
После поиска можно переключиться в режим сканирования по найденным адресам или сканировать адрес конкретного пакета или группы адресов с различным LSB в адресе<br>
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-6.png">
<br>
Адреса, которые попались дважды и более раз отображаются списком:<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-7.png">
<br><br>
Изменение режима sniff/scan - стрелками на пункте Scan.<br><br>
Режим сканирования (scan) - просто чтение пакетов по заданным в настройках мак адресам и виду пакета - ESB/DPL.<br>
На начальном экране в режиме чтения можно загрузить файл настроек (по умолчанию загружается settings.txt из папки nrf24_scanner на SD карте).<br>
В файле настройке задаются адреса (максимум 6) в шестнадцатеричном виде (старший байт - первый), длина адреса вычисляется по P0.<br>
Остальные настройки можно поменять интерактивно<br>
Настройки сохраняются длительным нажатием на ОК.<br><br>
<b>Описание настроек:</b><br>
Ch - номер канала.<br>
Rate - скорость передачи данных<br>
Next Ch time - через сколько секунд будет увеличен номер канала<br>
Log - выбор режима авто сохраннения в файлы log-xx.txt. Yes - сохранять в новый файл при заполнении буфера в 99 записей, Append - добавлять в последний файл, Clear - только очистка буфера<br><br>
В пунктах Ch, Rate, Next при нажатии OK меняются параметры связи:<br>
ESB - Enhanced ShockBurst (включена автоотправка подтверждения получения пакета, работающий приемник тоже попытается это сделать, возможны коллизии)<br>
DPL - Динамический пакет<br>
CRC1/2 - Размер CRC в байтах<br>
Payload - размер пакета в байтах<br>
<br>
<b>Просмотр принятых пакетов</b><br><br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-2.png">
<br>
В пункте "Start scan/sniff" можно выбрать стрелками сканировать и смотреть или просто смотреть (view).<br>
Если в файле настройки было несколько адресов, то первая цифра - номер канала (pipe) от 0 до 5.<br>
Стрелки - перемещение по списку и горизонтальное скролирование<br>
Долгий OK - отправка пакета<br>
OK - вход в режим просмотра адресов и включения декодирования заголовка ESB пакета и CRC.<br>
При декодировании заголовка (PCF) - первые 2 цифры - длина пакета в hex или 33, если длина пакета фиксирована<br>
3-я цифра - PID (2bit) << 1 +  флаг NO_ACK<br>
Если включен режим декодирования CRC, то по всему пакету ищется подходящая CRC и подчеркивается в списке, а так же вместо ":" выводится "=" после номера записи в буфере<br>
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-3.png">
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-4.png">
<br>
<img src="https://raw.githubusercontent.com/vad7/nrf24scan/master/Screenshot-5.png">
<br>
<br>
_________________________________________________________________________________
<br>
<br>
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
Decode the Packet Control Field and check CRC (long press OK in the list and then press '<' / '>').<br>
ESB (Enhanced Shockburst) option must be turned off.
Press '>' to decode CRC.<br>
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

