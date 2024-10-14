let tests = require("tests");
let event_loop = require("event_loop");

let ext = {
    i: 0,
    received: false,
};

let queue = event_loop.queue(16);

event_loop.subscribe(queue.input, function (_, item, tests, ext) {
    tests.assert_eq(123, item);
    ext.received = true;
}, tests, ext);

event_loop.subscribe(event_loop.timer("periodic", 1), function (_, _item, queue, counter, ext) {
    ext.i++;
    queue.send(123);
    if (counter === 10)
        event_loop.stop();
    return [queue, counter + 1, ext];
}, queue, 1, ext);

event_loop.subscribe(event_loop.timer("oneshot", 1000), function (_, _item, tests) {
    tests.fail("event loop was not stopped");
}, tests);

event_loop.run();
tests.assert_eq(10, ext.i);
tests.assert_eq(true, ext.received);
