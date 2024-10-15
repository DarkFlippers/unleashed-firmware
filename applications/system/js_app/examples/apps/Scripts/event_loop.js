let eventLoop = require("event_loop");

// print a string after 1337 milliseconds
eventLoop.subscribe(eventLoop.timer("oneshot", 1337), function (_subscription, _item) {
    print("Hi after 1337 ms");
});

// count up to 5 with a delay of 100ms between increments
eventLoop.subscribe(eventLoop.timer("periodic", 100), function (subscription, _item, counter) {
    print("Counter two:", counter);
    if (counter === 5)
        subscription.cancel();
    return [counter + 1];
}, 0);

// count up to 15 with a delay of 100ms between increments
// and stop the program when the count reaches 15
eventLoop.subscribe(eventLoop.timer("periodic", 100), function (subscription, _item, event_loop, counter) {
    print("Counter one:", counter);
    if (counter === 15)
        event_loop.stop();
    return [event_loop, counter + 1];
}, eventLoop, 0);

eventLoop.run();
