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
store. Page 1 has four blocks carrying traceability data; it is writable, so its contents are
not proof of anything about the chip's origin.

### Blocks that were not read

Every block is always written, so a block holding zero is written as `00 00 00 00` and means
exactly that. Blocks that were *never read* are written as zero too, which is why the
`blocks read` keys exist: they list the blocks whose values came from the chip. Anything not
listed is padding, and a tool must not treat it as content - in particular it must not write
it back to a tag.

A read is partial more often than not. A password-protected chip answers nothing without its
password, a locked block can be read but says nothing about what a write would do, and a chip
that drifts out of the field mid-dump leaves a gap in the middle.

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

The order the blocks go back in matters, and it is not the order they appear in the file:

1. Data blocks first, then block 0 last. Block 0 decides how the chip talks, so writing it
   first means the rest of the sequence is verified against a configuration that may not match
   what the chip is still using.
2. When the dump carries a password and the chip is to be protected again, block 7 goes in
   *before* block 0. Writing block 0 first clears the password bit, and the block 7 frame that
   followed would still be carrying a password the chip had just stopped expecting - which it
   reads off the wrong bit positions.
3. Only blocks named by a `blocks read` key are written. The rest are padding.
