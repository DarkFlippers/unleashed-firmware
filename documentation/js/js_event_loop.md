# js_event_loop {#js_event_loop}

# Event Loop module
```js
let eventLoop = require("event_loop");
```

The event loop is central to event-based programming in many frameworks, and our
JS subsystem is no exception. It is a good idea to familiarize yourself with the
event loop first before using any of the advanced modules (e.g. GPIO and GUI).

## Conceptualizing the event loop
If you ever wrote JavaScript before, you have definitely seen callbacks. It's
when a function accepts another function (usually an anonymous one) as one of
the arguments, which it will call later on, e.g. when an event happens or when
data becomes ready:
```js
setTimeout(function() { console.log("Hello, World!") }, 1000);
```

Many JavaScript engines employ a queue that the runtime fetches events from as
they occur, subsequently calling the corresponding callbacks. This is done in a
long-running loop, hence the name "event loop". Here's the pseudocode for a
typical event loop:
```js
while(loop_is_running()) {
    if(event_available_in_queue()) {
        let event = fetch_event_from_queue();
        let callback = get_callback_associated_with(event);
        if(callback)
            callback(get_extra_data_for(event));
    } else {
        // avoid wasting CPU time
        sleep_until_any_event_becomes_available();
    }
}
```

Most JS runtimes enclose the event loop within themselves, so that most JS
programmers does not even need to be aware of its existence. This is not the
case with our JS subsystem.

# Example
This is how one would write something similar to the `setTimeout` example above:
```js
// import module
let eventLoop = require("event_loop");

// create an event source that will fire once 1 second after it has been created
let timer = eventLoop.timer("oneshot", 1000);

// subscribe a callback to the event source
eventLoop.subscribe(timer, function(_subscription, _item, eventLoop) {
    print("Hello, World!");
    eventLoop.stop();
}, eventLoop); // notice this extra argument. we'll come back to this later

// run the loop until it is stopped
eventLoop.run();

// the previous line will only finish executing once `.stop()` is called, hence
// the following line will execute only after "Hello, World!" is printed
print("Stopped");
```

I promised you that we'll come back to the extra argument after the callback
function. Our JavaScript engine does not support closures (anonymous functions
that access values outside of their arguments), so we ask `subscribe` to pass an
outside value (namely, `eventLoop`) as an argument to the callback so that we
can access it. We can modify this extra state:
```js
// this timer will fire every second
let timer = eventLoop.timer("periodic", 1000);
eventLoop.subscribe(timer, function(_subscription, _item, counter, eventLoop) {
    print("Counter is at:", counter);
    if(counter === 10)
        eventLoop.stop();
    // modify the extra arguments that will be passed to us the next time
    return [counter + 1, eventLoop];
}, 0, eventLoop);
```

Because we have two extra arguments, if we return anything other than an array
of length 2, the arguments will be kept as-is for the next call.

The first two arguments that get passed to our callback are:
  - The subscription manager that lets us `.cancel()` our subscription
  - The event item, used for events that have extra data. Timer events do not,
    they just produce `undefined`.

# API reference
## `run`
Runs the event loop until it is stopped with `stop`.

## `subscribe`
Subscribes a function to an event.

### Parameters
  - `contract`: an event source identifier
  - `callback`: the function to call when the event happens
  - extra arguments: will be passed as extra arguments to the callback

The callback will be called with at least two arguments, plus however many were
passed as extra arguments to `subscribe`. The first argument is the subscription
manager (the same one that `subscribe` itself returns). The second argument is
the event item for events that produce extra data; the ones that don't set this
to `undefined`. The callback may return an array of the same length as the count
of the extra arguments to modify them for the next time that the event handler
is called. Any other returns values are discarded.

### Returns
A `SubscriptionManager` object:
  - `SubscriptionManager.cancel()`: unsubscribes the callback from the event

### Warning
Each event source may only have one callback associated with it.

## `stop`
Stops the event loop.

## `timer`
Produces an event source that fires with a constant interval either once or
indefinitely.

### Parameters
  - `mode`: either `"oneshot"` or `"periodic"`
  - `interval`: the timeout (for `"oneshot"`) timers or the period (for
    `"periodic"` timers)

### Returns
A `Contract` object, as expected by `subscribe`'s first parameter.

## `queue`
Produces a queue that can be used to exchange messages.

### Parameters
  - `length`: the maximum number of items that the queue may contain

### Returns
A `Queue` object:
  - `Queue.send(message)`:
    - `message`: a value of any type that will be placed at the end of the queue
  - `input`: a `Contract` (event source) that pops items from the front of the
    queue
