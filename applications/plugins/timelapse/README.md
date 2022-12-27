
# zeitraffer

[![Build FAP](https://github.com/theageoflove/flipperzero-zeitraffer/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/theageoflove/flipperzero-zeitraffer/actions/workflows/build.yml)

english version [below](#eng)

![zeitraffer for flipper zero](https://theageoflove.ru/uploads/2022/11/photo_2022-11-10_15-54-25.jpg)
Видео работы: https://youtu.be/VPSpRLJXYAc

Готовый фап под последнюю релизную прошивку [можно скачать здесь](https://nightly.link/theageoflove/flipperzero-zeitraffer/workflows/build/main/zeitraffer.fap.zip).

Я ненастоящий сварщик, не обессудьте. Делал для своей Sony DSLR A100, подходит для любых камер, поддерживающих проводной пульт с тремя контактами.

Основано на хелловорлде https://github.com/zmactep/flipperzero-hello-world

### Управление: 

 - **вверх-вниз** - время.
 - **влево-вправо** - количество кадров
 
 0 кадров - бесконечный режим, -1 кадров - BULB
 - **зажатие стрелок** - ±10 кадров/секунд
 - **ОК** - пуск/пауза
 - Длинное нажатие **ОК** - включить/выключить подсветку
 - **назад** - сброс
 - длинное нажатие **назад** - выход

При работающем таймере блокируются все кнопки кроме ОК.

При запуске даётся три секунды на отскочить.

## Чо надо
 - две оптопары типа EL817C
 - кусок гребёнки на три пина
 - немного провода
 - термоусадка
 - разъём пульта от камеры. Где взять или из чего сделать - думайте

## Как собрать
Берём оптопары, соединяем по схеме. 
![](https://theageoflove.ru/uploads/2022/11/camera_cable.jpg)
Где какой пин у камеры, можно узнать например тут: https://www.doc-diy.net/photo/remote_pinout/

# <a name="eng"></a>English
Simple timelapse app for Flipper Zero.

[Get latest release](https://nightly.link/theageoflove/flipperzero-zeitraffer/workflows/build/main/zeitraffer.fap.zip)

based on https://github.com/zmactep/flipperzero-hello-world

### Control:
 - Up and down - time. 
 - Left and right - number of frames 
 - Long press arrows - ±10 frames/seconds 
 - OK - start/pause 
 - Long press OK - turn on/off the backlight 
 - Back - reset 
 - Long press back - exit

When the timer is running, all buttons are blocked except OK.

## What you need:
  - two EL817C optocouplers
  - pin header connector 1x3 2,54mm male
  - some wire
  - heat shrink
  - camera remote connector
## How to assemble
Take optocouplers, connect according to the scheme.
![](https://theageoflove.ru/uploads/2022/11/camera_cable_en.jpg)
Camera pinout can be found here: https://www.doc-diy.net/photo/remote_pinout/
