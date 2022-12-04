[Original link](https://github.com/Mywk/FlipperTemperatureSensor)

# Flipper Temperature Sensor

## Supported sensors

> HTU2xD, SHT2x, SI702x, SI700x, SI701x, AM2320

## What is this?

A small app for the [Flipper Zero](https://flipperzero.one) that reads the [I2C](https://en.wikipedia.org/wiki/I%C2%B2C) signal from a few temperature sensors and displays the current temperature and humidity.

I'm using a [Sparkfun HTU21D sensor](https://learn.sparkfun.com/tutorials/htu21d-humidity-sensor-hookup-guide), also tested with a clone and with the Si7021 variant.

![Flipper Temperature Sensor](images/Flipper.png)

![App](images/App.png)

<br/>

# How to Connect the HTU21D sensor
![Connection](images/Connection.png)


# How to install

If you have the FAP loader, just copy the fap file from the Releases into your Flipper apps folder and you should be able to launch it from the menu.

If you don't have the FAP loader you will have to bake this application together with your firmware (aka compile it all together).

# FAQ

## The app says the sensor is not found!

1- Are the four connectors correctly soldered?

2- Are the SCL and SDA connections correct? Re-check the "How to Connect the sensor" above.

3- For the HTU21D, on the sensor board, there should be three contacts in the center, for it to work correctly they must be soldered together (basically drop a blob of solder to connect the three of them). Without the solder it looks like this:

![Sensor](images/Sensor.png)

## Which Flipper versions was this app tested on?

Version 1.2
- RM11221439-0.71.2
- unlshd-015
- 0.71.1 

Version 1.1
- RM10302252-0.70.1
- unlshd-012
- 0.68.2-1007-RM
- 0.68.1
- 0.67.2

## I can't build the app together with the firmware?

In the *application.fam*, don't forget to change the apptype, it should not be EXTERNAL but APP.

# How to compile

Place the temperature_sensor folder in the applications_user folder and compile using FBT.

Please refer to the [Flipper Build Tool documentation](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/documentation/fbt.md).