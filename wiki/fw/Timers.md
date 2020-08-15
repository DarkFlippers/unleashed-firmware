## Tim1

~1 kHz, PWM

* Ch1 -- red led
* Ch2 -- green led
* Ch3 -- blue led

## Tim2

46/48 kHz, interrupt for IR. Ch4 -- IR TX

## Tim4

~1 kHz, PWM

* Ch1 -- backlight

## Tim5

Variable freq. PWM. Ch4 -- speaker.

## Tim8

* Ch2 -- iButton emulate

## Tim15

125 kHz. Square generator and interrupt/RFID pull.

* Ch1 -- RFID OUT (square generator)
* Ch2 -- RFID Pull

# Devices

## Speaker
Tim5 ch4

## Blue led
Tim1, Tim3, Tim8

## Red led
Tim1

## Green led
Tim15, Tim1, Tim8

## Backlight
Tim16, Tim4

## IR TX
Tim2 ch4

## RFID out
Tim15, Tim1

## RFID pull
Tim15, Tim1, Tim8

## iButton
Tim3, Tim8
