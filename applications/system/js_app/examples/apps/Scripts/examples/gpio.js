let gpio = require("gpio");

// initialize pins
gpio.init("PC3", "outputPushPull", "up"); // pin, mode, pull
print("PC3 is initialized as outputPushPull with pull-up");

gpio.init("PC1", "input", "down"); // pin, mode, pull
print("PC1 is initialized as input with pull-down");

// let led on PC3 blink
gpio.write("PC3", true); // high
delay(1000);
gpio.write("PC3", false); // low
delay(1000);
gpio.write("PC3", true);  // high
delay(1000);
gpio.write("PC3", false); // low

// read value from PC1 and write it to PC3
while (true) {
    let value = gpio.read("PC1");
    gpio.write("PC3", value);

    value ? print("PC1 is high") : print("PC1 is low");

    delay(100);
}


// possible pins https://docs.flipper.net/gpio-and-modules#miFsS
// "PA7" aka 2
// "PA6" aka 3
// "PA4" aka 4
// "PB3" aka 5
// "PB2" aka 6
// "PC3" aka 7
// "PA14" aka 10
// "PA13" aka 12
// "PB6" aka 13
// "PB7" aka 14
// "PC1" aka 15
// "PC0" aka 16
// "PB14" aka 17

// possible modes
// "input"
// "outputPushPull"
// "outputOpenDrain"
// "altFunctionPushPull"
// "altFunctionOpenDrain"
// "analog"
// "interruptRise"
// "interruptFall"
// "interruptRiseFall"
// "eventRise"
// "eventFall"
// "eventRiseFall"

// possible pull
// "no"
// "up"
// "down"