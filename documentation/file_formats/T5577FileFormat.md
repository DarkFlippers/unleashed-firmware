# T5577 dump file format {#t5577_file_format}

## Example

```
Filetype: Flipper T5577 dump
Version: 1
# Block 0: ASK/Manchester, RF/64, 2 data blocks, password enabled
Page 0 blocks read: 0 1 2 7
Page 0 Block 0: 00 14 80 50
Page 0 Block 1: 12 34 56 78
Page 0 Block 2: 9A BC DE F0
Page 0 Block 3: 00 00 00 00
Page 0 Block 4: 00 00 00 00
Page 0 Block 5: 00 00 00 00
Page 0 Block 6: 00 00 00 00
Page 0 Block 7: 0C 0D 0E 0F
Page 1 blocks read: 0 1
Page 1 Block 0: E0 15 00 1A
Page 1 Block 1: 20 24 10 00
Page 1 Block 2: 00 00 00 00
Page 1 Block 3: 00 00 00 00
Password: 0C 0D 0E 0F
```

## Description

Filename extension: `.t5577`

The file stores the contents of a T5577 (ATA5577) transponder, block by block, as the chip
holds them. It describes a *chip*, where a `.rfid` file describes a *card*: a T5577 configured
as an EM4100 can be saved either way, and the two are not interchangeable. The `.rfid` file
says what the card is; this file says what is in the silicon, including the configuration that
decides how the chip talks and the password that protects it.

Page 0 has eight 32-bit blocks. Block 0 is the configuration - modulation, bit rate, how many
data blocks are broadcast, and whether a password is required - and block 7 is the password
store. Page 1 has four blocks: block 0 repeats the configuration, and blocks 1 to 3 carry the
traceability data the chip was made with. The page is writable, so its contents are not
proof of anything about the chip's origin.

### Blocks that were not read

Every block gets a key, whether or not it was read, so a block holding zero is written as
`00 00 00 00` - and so is a block nobody has seen. The `blocks read` keys are what tells
them apart: they list the blocks whose values came from the chip. Anything not listed is
padding, and a tool must not treat it as content - in particular it must not write it back
to a tag.

A file that lists no blocks at all is not a dump of a chip, and is rejected rather than read
as twelve blocks of padding.

A read can come back short: a chip that drifts out of the field mid-dump leaves a gap in the
middle, and a locked block can be read but says nothing about what a write would do. A
password-protected chip is the other way about - it refuses the block 0 read the detect itself
needs, so without the password there is no dump at all rather than a partial one.

### Password

`Password` is a password known to work on the chip. It is deliberately separate from
`Page 0 Block 7`, because the two are different kinds of fact and can legitimately disagree:

- `Page 0 Block 7` is what block 7 contained when it was read. On a chip with no password
  enabled, block 7 is ordinary storage and may hold anything.
- `Password` is a credential. It can be known from an authentication that succeeded even when
  block 7 itself was never read, and it can outlive block 7 - clearing a password only clears
  the password bit in block 0, so block 7 goes on holding the old value.

A restore uses `Password` to authenticate and `Page 0 Block 7` as data to write. The key is
omitted when no password is known.

### Version history

1. Initial version.

### Format fields

| Name                 | Description                                                      |
| -------------------- | ---------------------------------------------------------------- |
| Page 0 blocks read   | Blocks of page 0 whose values came from the chip, space-separated |
| Page 0 Block 0..7    | Block contents, 4 HEX bytes, most significant byte first         |
| Page 1 blocks read   | Blocks of page 1 whose values came from the chip, space-separated |
| Page 1 Block 0..3    | Block contents, 4 HEX bytes, most significant byte first         |
| Password             | A password known to work on this chip. Optional                  |

A comment line decoding block 0 may precede the blocks. It is written for the benefit of anyone
reading the file and carries no information that block 0 does not.

### Restoring

The order the blocks go back in matters, and it is not the order they appear in the file. It
is the order a Proxmark3 restore uses:

1. Page 0 blocks 1 to 7, in that order. Block 7 is the password store, so a tool addressing
   its writes to a password has to switch to the one block 7 now holds from that point on -
   the switch is at block 7, not at block 0. On a chip that is not protected to begin with,
   no frame carries a password at all until block 0 lands last and turns protection on, and
   the next tool to write it is the one that has to send the new password.
2. Page 1 blocks 1 to 3. Page 1 block 0 repeats the configuration and is never written.
   Blocks 1 and 2 of page 1 hold the traceability data, programmed and locked at manufacture:
   a genuine chip refuses to change them, and only some clones take the write. A tool that
   reads the blocks back afterwards should report a difference there as the block being
   unchanged rather than as a write that failed - it cannot tell a chip that refused the write
   from a write that was dropped, and on a genuine chip the first is the expected answer.
3. Page 0 block 0 last. It decides how the chip talks, so writing it first would leave the
   rest of the sequence addressed to a configuration the chip is no longer using.
4. Only blocks named by a `blocks read` key are written. The rest are padding.
5. A block 0 of zero is never written, even when the read list names it. Zero is not a
   configuration: a chip holding it answers nothing at all, and nothing on the air says
   that is what happened, so there is no way back in.

What the chip did with the writes cannot be read from the writes themselves - a write frame
has no answer - so a tool that wants to know whether a restore worked has to read the blocks
back afterwards and compare.
