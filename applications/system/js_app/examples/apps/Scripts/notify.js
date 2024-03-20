let notify = require("notification");
notify.error();
delay(1000);
notify.success();
delay(1000);
for (let i = 0; i < 10; i++) {
    notify.blink("red", "short");
    delay(500);
}