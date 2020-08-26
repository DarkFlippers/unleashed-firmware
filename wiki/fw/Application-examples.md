One of the most important component of Flipper Core is [FURI](FURI) (Flipper Universal Registry Implementation). It helps control the applications flow, make dynamic linking and interaction between applications.

In fact, FURI is just wrapper around RTOS thread management and mutexes, and callback management.

In this article we create few application, interact between apps, use OS functions and interact with HAL.

# General agreements

Flipper application is just a function:

```C
void application_name(void* p) {
    // Setup

    while(1) {
        // Loop
    }
}
```

1. `void* p` is arbitrary pointer that may be used for pass parameters to application at launch (like argc/argv in POSIX).
2. Application must never attempt to return or exit from their implementing function.
3. Avoid long cycles without any "waits" or "blocking" like `delay` or `xQueueReceive`, otherwise your app will blocking overall Flipper work.
4. Do not create static variables inside function or global variables. Use only local variables. We plan to add virual in-RAM filesystem to save any persistent data.  

# Application examples

* **[Blink](Blink-app)** show how to create app and control GPIO
* **[UART write](UART-write)** operate with FURI pipe and print some messages
