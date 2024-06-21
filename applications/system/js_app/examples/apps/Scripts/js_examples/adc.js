// This is an example of how to use the analog pins (ADC) on the Flipper Zero. 
// The example uses a reference voltage of 2048mV (2.048V), but you can also use 2500mV (2.5V). 
// The example reads the values of the analog pins A7, A6, and A4 and prints them to the console. 
// The example also checks if the value of A7 is twice the value of A6 and breaks the loop if it is. 
// The example uses the analog pins A7, A6, and A4, but you can also use PC3, PC1, and PC0. 

let gpio = require("gpio");

// initialize pins A7, A6, A4 as analog (you can also use PC3, PC1, PC0)
gpio.init("PA7", "analog", "no"); // pin, mode, pull
gpio.init("PA6", "analog", "no"); // pin, mode, pull
gpio.init("PA4", "analog", "no"); // pin, mode, pull

gpio.startAnalog(2048); // vRef = 2.048V (you can also use 2500 for a 2.5V reference voltage)

while (true) {
    let pa7_value = gpio.readAnalog("PA7");
    let pa6_value = gpio.readAnalog("PA6");
    let pa4_value = gpio.readAnalog("PA4");
    print("A7: " + to_string(pa7_value) + " A6: " + to_string(pa6_value) + " A4: " + to_string(pa4_value));
    delay(100);
    if (pa7_value === pa6_value * 2) {
        break;
    }
}
print("A7 is twice A6!");

gpio.stopAnalog();

// possible analog pins https://docs.flipper.net/gpio-and-modules#miFsS
// "PA7" aka 2
// "PA6" aka 3
// "PA4" aka 4
// "PC3" aka 7
// "PC1" aka 15
// "PC0" aka 16

// possible modes
// "analog"

// possible pull
// "no"
