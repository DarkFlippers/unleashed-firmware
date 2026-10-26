# NERO Intro II (experimental receive support)

Validated with one NERO INTRO II 8501-1M remote and three additional
owner-provided remotes using the same observed format at 434.42 MHz, OOK.
Their PCB revisions have not been inspected. AM270 and AM650 recordings were
independently checked. This is a different line format from `Nero Radio`.

The observed chip duration is about 125 us. A synchronization section is
followed by 15 symbols, each containing a 63-chip codeword and a nominal `01`
separator. Codewords are cyclic phases of a degree-6 maximal-length sequence
or its complement. The recurrence is `b[n] = b[n-5] XOR b[n-6]`.

The decoder labels a symbol with its six-bit initial state and an inversion
bit. These are canonical line-code labels; the manufacturer's application
mapping has not been established. Fifteen labels make 105 canonical bits.
`Key` stores the first 64 and `Data` the remaining 41, big-endian in the usual
eight-byte Flipper fields. `Bit` must be exactly 105, and unused `Data` bits
must be zero. This preserves every recovered symbol without inventing a
serial number, button, counter or authentication value.

Across four remotes, the observed alphabet contains exactly 64 labels:
even rotations of codeword(1) and their complements. An empirical compact
label is rotation/2 with inversion as bit 5, giving 15 six-bit labels (90
bits). Four positions are constant across the samples. This is a research
representation, not a recovered application format or security bit count.
The saved 105-bit representation is retained for compatibility; it is a
lossless, redundant representation of these symbols. No fixed remote-specific
field was identified in the compact representation. For the original remote,
all 13 interior six-bit symbols have full binary difference rank across the
available distinct messages, excluding simple per-symbol linear constants.

The decoder supports bounded chip insertion/deletion corrections and rejects
ambiguous best decodes. Its workspace is allocated with the decoder rather
than placed on the receive worker's stack. It resumes synchronization after
each frame and after long gaps, so normal Sub-GHz Read and CLI RX can process
multiple activations. For unattended UI reception, enable the existing
`Delete old signals when memory is full` option. UI history is finite; CLI RX
does not use the UI history limit.

The constant tail of a long idle is retained in the sync shift register: the
first synchronization chip may merge with that idle. Discarding the entire
idle lost repeated frames in the additional remotes' captures. The protocol
wrapper also retains this sync tail across the receiver callback, which resets
all decoder instances; otherwise two valid frames in the four-remote fixture
were still lost on the device despite passing standalone core tests.

On-device tests confirmed two successive activations in one normal Read
session using the internal CC1101, AM650, 434.42 MHz. Other tested radio/preset
combinations missed activations. Long-duration reliability is not established.
Decoded frames identify this line format, not an authenticated remote identity
or a particular receiver; one button activation can produce several distinct
frames. Persistent logging and notification integration are outside this patch.

Save/load support is provided. No encoder is registered: authentication and
generation of the next valid command are not understood. A changing payload
or successful line-code decode must not be presented as a recovered remote
identity, counter or cryptographic key. Old RAW replays did not open the test
barrier, while the original remote did. RF-link quality and receiver replay
rules have not been isolated, so this alone is not a cryptographic conclusion.

Validation used independent private captures, concatenated multi-activation
streams, a wrong-modulation negative control, the existing mixed-protocol
random fixture, and existing Nero Radio/Sketch fixtures. Forty-one unambiguous
frames across nine recordings match the independent reference decoder.
Private access captures are intentionally outside this repository. Longer
live reception tests and additional hardware revisions are still needed.
An on-device offline integration test of the combined four-remote recording
recovers all 21 unambiguous frames, in the same order and with identical
symbol values as the independent reference.

Public host-side regression tests use synthetic payloads, including consecutive
frames with merged idle/sync chips, chip insertion/deletion/substitution, an
invalid sync marker, truncation and unrelated alternating pulses. Run from the
repository root:

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Ilib/subghz/protocols scripts/tests/subghz_nero_intro_ii_test.c \
  lib/subghz/protocols/nero_intro_ii_code.c -o /tmp/nero_intro_ii_test
/tmp/nero_intro_ii_test
```

These exercise the standalone line-code core, not the firmware wrapper's
save/load handling or receiver callback resets. Synthetic synchronization
padding is used after the marker; this is not a transmitter implementation.
