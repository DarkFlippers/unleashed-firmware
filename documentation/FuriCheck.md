# Run time checks and forced system crash {#furi_check}

The best way to protect system integrity is to reduce amount cases that we must handle and crash the system as early as possible.
For that purpose, we have a bunch of helpers located in Furi Core `check.h`.

## Couple notes before start

- Definition of Crash — log event, save crash information in RTC and reboot the system.
- Definition of Halt — log event, stall the system.
- Debug and production builds behave differently: debug build will never reset system in order to preserve state for debugging.
- If you have debugger connected we will stop before reboot automatically.
- All helpers accept optional MESSAGE_CSTR: it can be in RAM or Flash memory, but only messages from Flash will be shown after system reboot.
- MESSAGE_CSTR can be NULL, but macros magic already doing it for you, so just don't.

## `furi_assert(CONDITION)` or `furi_assert(CONDITION, MESSAGE_CSTR)`

Assert condition in development environment and crash the system if CONDITION is false.

- Should be used at development stage in apps and services.
- Keep in mind that release never contains this check.
- Keep in mind that libraries never contain this check by default, use `LIB_DEBUG=1` if you need it.
- Avoid putting function calls into CONDITION, since it may be omitted in some builds.

## `furi_check(CONDITION)` or `furi_check(CONDITION, MESSAGE_CSTR)`

Always assert condition and crash the system if CONDITION is false.

- Use it if you always need to check conditions

## `furi_crash()` or `furi_crash(MESSAGE_CSTR)`

Crash the system.

- Use it to crash the system. For example, if an abnormal condition is detected.

## `furi_halt()` or `furi_halt(MESSAGE_CSTR)`

Halt the system.

- We use it internally to shutdown Flipper if poweroff is not possible.
