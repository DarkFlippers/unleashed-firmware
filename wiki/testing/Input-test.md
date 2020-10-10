1. `docker-compose exec dev make -C target_f2 example_input_dump`
2. Flash
3. For x in ```
[
    (Up, "00"),
    (Down, "01"),
    (Right, "02"),
    (Left, "03"),
    (Ok, "04"),
    (Back, "05"),
]
```
    * Press ${x[0]}
    * wait 0.05
    * Expect: Uart: "event: ${x[1]} pressed"
    * wait 0.05
    * Release ${x[0]}
    * wait 0.05
    * Expect: Uart: "event: ${x[1]} released"
    * wait 0.05

TODO: add debouncing check (multiple press and check there is no multiple events)
