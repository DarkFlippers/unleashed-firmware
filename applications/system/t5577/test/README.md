# T5577 host tests

```
python applications/system/t5577/test/run.py
```

These run on the PC, against host clang (`C:\Program Files\LLVM\bin\clang.exe`). They are not
part of the firmware build: `application.fam` excludes this directory from the app's sources,
because the default glob is `*.c*` and it recurses.

They are host tests rather than `unit_tests` cases for two reasons, and the second is the one
that would still stand if the first were fixed:

- `unit_tests` cannot link in this tree — a pre-existing DWARF error, unrelated to this app.
- `unit_tests` is its own app set in `fbt_options.py`, replacing `main_apps` and `system_apps`
  rather than joining them. Running a case there means building a second firmware and flashing
  a Flipper. The units here are arithmetic over a synthetic ring buffer, and they are worth
  iterating on in seconds.

## The suites

**`t5577_sniff_test.c`** — 21 cases over the downlink sniffer's framing and decode. It
`#include`s `t5577_sniff.c` directly and builds synthetic rings from this firmware's own write
timings, so the layout arithmetic is checked against sequences whose answer is known in advance.
That matters because the alternative way to check it is to find a cloner and hope.

Cases worth knowing about:

- Both symbol modes. Leading-zero periods are 288 and 464 us, *both* under the 480 us fixed
  split, which is why a burst has to be allowed to split itself rather than measured against a
  constant.
- A missed gap mid-frame, which is what dragged an earlier midpoint-of-extremes estimate off by
  120 us and decoded a whole frame as zeroes.
- A long leading reference symbol, which poisoned the class estimate until the preamble was
  stripped before measuring.
- The blue cloner's real routine: nine passworded writes among junk bursts.

**`t5577_restore_test.c`** — the `MU_TEST` body for `t5577_restore_password()`, run on the host.
`run.py` cuts it out of `applications/debug/unit_tests/tests/lfrfid/lfrfid_protocols.c` on every
run rather than keeping a copy, so there is nothing to hold in step: edit that test and this
suite runs the edit. The copy this replaces had already fallen two cases behind it.

This one decides whether a password frame is sent at all, so it was mutation-tested: 16
mutations, 12 killed by the suite as it stood, and the two real gaps the survivors exposed are
now covered.

## `stub/`

Just enough of `furi.h`, `furi_hal.h`, the LL timer header and a couple of others to compile the
units away from the firmware. Nothing in it mocks behaviour — the stubs exist so the real source
can be included verbatim rather than adapted.
