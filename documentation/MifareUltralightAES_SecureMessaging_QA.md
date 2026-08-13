# MIFARE Ultralight AES — Secure Messaging (CMAC) QA validation plan

Hardware validation plan for the **secure-messaging (CMAC) capability** of the MIFARE
Ultralight AES (MF0AES20) support. This is the one part of the UL-AES feature that has **no
reference implementation to cross-check against** (Proxmark3 only detects the `SEC_MSG_ACT`
bit — it does not implement the secure-messaging protocol), so its framing was derived from
the datasheet alone and must be confirmed on real silicon before merge.

## Scope

In scope — secure messaging only:

- **Reader, read** over secure messaging: adaptive detection (a plain `READ` NAKs right after
  auth ⇒ re-auth and switch to CMAC), MAC-wrapped `READ`, MAC-wrapped `READ_CNT`.
- **Reader, write-back** over secure messaging: adaptive detection on the first page write,
  MAC-wrapped `WRITE`, standalone-MAC ACK.
- **Emulation** of a secure-messaging card: listener verifies the command MAC, MAC-wraps every
  response, replaces the `WRITE` ACK with a standalone MAC, drops the session on a bad MAC.

Relevant commits: `3af5af8a9`, `5f72e5e31`, `8251a8222`, `db0a51cac`, `c06ef72bd`.
Key code: `mf_ultralight_aes_crypto.c` (`mf_ultralight_aes_cmac8_ctr`),
`mf_ultralight_poller_i.c` (`mf_ultralight_poller_txrx_aes_cmac`),
`mf_ultralight_poller.c` (`mf_ultralight_poller_write_page_auto`, counter handler),
`mf_ultralight_listener.c` (`send_response`, `verify_strip_cmd`, run-loop dispatch).

**Out of scope** (covered by the general UL-AES validation, not repeated here): AES 3-pass auth,
dictionary attack, manual unlock, read/write/emulation on **non**-secure-messaging cards, config
render, Random ID, key management.

## Why the first test must go past the first frame

The two highest-risk, HW-unverified choices are both invisible on a single exchange:

1. **Counter progression.** The command counter must advance **+2 per exchange** (command uses
   `CmdCtr`, response uses `CmdCtr+1`), both sides starting at 0. A wrong step size still lets
   the **first** `C=0` read succeed, then fails every later frame. So a passing test must span
   **at least two** MAC'd exchanges (a multi-`READ` full read already does — 60 pages ≈ 15
   `READ`s) and, ideally, a **secure write** (the ACK-replacement MAC uses `CmdCtr+1` over an
   empty body — a distinct code path).
2. **MAC byte order / SV2 layout.** A mismatch here fails the very first MAC. Any successful
   authenticated read is already strong evidence this is right.

**Start with X1 and W1 below.**

## Equipment

- One genuine **MF0AES20** (MIFARE Ultralight AES) card with a **known DataProtKey**.
- A **Proxmark3** — used only to (a) set `SEC_MSG_ACT` on the card and (b) read back raw memory
  to confirm expected contents. PM3 **cannot** act as a secure-messaging reader.
- One **blank/factory** UL-AES card (for the clone-to-blank regression, W2).
- For emulation tests, **one of**:
  - a **second Flipper** running this firmware (preferred — enables the self-contained
    cross-validation, group X, with no special reader), or
  - a real access reader / phone app that speaks UL-AES secure messaging.
- Debug logging enabled on the device under test (see below).

## Setup — turn a card into a secure-messaging card

While `SEC_MSG_ACT` is still **off**, secure messaging is not yet required, so an authenticated
**plain** write can set it. With Proxmark3:

1. Authenticate and dump the card; record the config pages (`0x29`–`0x2D`) and all data.
2. Set the `SEC_MSG_ACT` bit in the configuration (per datasheet §8.6 / Table "Random ID
   RID_ACT and Secure Messaging SEC_MSG_ACT"). Do **not** touch `AUTH0`/`LOCK`/`AUTH_LIM` —
   a wrong value there can brick or lock the card.
3. Re-select the card. From now on every authenticated session requires CMAC.
4. Confirm on the Flipper: read the card and open the full info view — the config decoder should
   show **Secure messaging: on**.

Keep the recorded plain dump — it is the ground truth to diff every read/write result against.

## Observing which path executed

Enable debug logs (CLI `log` at `debug` level, or Settings → System → Log level → Debug) and
watch for the markers added for this feature:

- Reader switched to CMAC read: full read of a `SEC_MSG` card **succeeds** (a plain read would
  NAK), and the counters appear — impossible over plain commands.
- `UL-AES response MAC mismatch (ctr N)` — reader rejected a card response MAC (should **not**
  appear in a healthy run; expected only in the tamper test).
- `UL-AES command MAC mismatch, dropping session` — emulator rejected a reader command MAC
  (expected only in E3).
- `UL-AES secure-messaging write: re-auth failed, page N locked` / `... card gone during
  re-activation` — write-back adaptive fallback diagnostics.

A **clean** end-to-end run should show none of the mismatch/failure warnings.

## Test matrix

Record `Pass`/`Fail` + notes for each. A **Fail** on any MAC/counter test most likely points at
byte order, the `+2` counter step, or the SV2 session-key layout — note exactly which exchange
number failed.

### Group X — Flipper ↔ Flipper cross-validation (no external reader needed) — DO FIRST

| ID | Scenario | Steps | Expected |
|----|----------|-------|----------|
| X1 | Emulate → read a secure-messaging card | Flipper A emulates the captured `SEC_MSG` dump; Flipper B reads it | B performs AES 3-pass auth, detects CMAC, reads **all** pages; data matches the ground-truth dump; **no** MAC-mismatch warnings on either device (validates auth + multi-exchange `READ` counter progression end-to-end) |
| X2 | Emulate → write-back | With A still emulating, B writes the dump back to A (Write, Keep Key) | B detects CMAC on the first page write, writes every data page MAC-wrapped; A's emulated memory matches after; no warnings (validates the `WRITE` command MAC + standalone-MAC ACK + write counter progression) |
| X3 | Emulate → read counters | B reads A; open full info | The 3 one-way counters read over secure messaging show A's values |

### Group R — Reader reads a real secure-messaging card

| ID | Scenario | Steps | Expected |
|----|----------|-------|----------|
| R1 | Full read over CMAC | Read the real `SEC_MSG` card (key in dict or via Unlock) | Detection fires (plain `READ` NAK → re-auth → CMAC); full memory read; all pages match the PM3 ground truth |
| R2 | Counter read over CMAC | After R1, view counters | All 3 counters match PM3; no MAC-mismatch warning |
| R3 | Wrong key | Read with a wrong key present | Clean failure / falls back to open pages; no hang, no infinite re-select, no unexpected `AUTH_LIM` burn |
| R4 | Key pages never leak | Inspect the saved dump's pages `0x30`–`0x37` | Read back as zero (masked), never the real key bytes |

### Group W — Reader writes back to a real secure-messaging card

| ID | Scenario | Steps | Expected |
|----|----------|-------|----------|
| W1 | Write-back to a SEC_MSG card | Write a saved dump to the real `SEC_MSG` card (Keep Key) | First page write NAKs → re-auth → all data pages written MAC-wrapped; re-read (PM3 or Flipper) matches; no warnings (validates write counter progression across many pages) |
| W2 | Clone to a blank (regression) | Write the same dump to a **blank** UL-AES card | Plain writes succeed with **no** false CMAC switch (blank has `SEC_MSG_ACT` off); result verified by re-read |
| W3 | Copy-key write-back | Write a dump with "Copy Key" to a SEC_MSG card | Data pages **and** DataProtKey (`0x30`–`0x33`) written over CMAC; subsequent auth with the copied key succeeds |

### Group E — Emulate to an external reader (only if no 2nd Flipper; else X covers it)

| ID | Scenario | Steps | Expected |
|----|----------|-------|----------|
| E1 | Real reader reads emulated card | Emulate the captured `SEC_MSG` dump; present to a UL-AES SEC_MSG reader | Reader authenticates and reads the AUTH0/PROT-gated memory correctly |
| E2 | Real reader writes emulated card | Reader performs a MAC-protected `WRITE` | Emulator accepts it and returns the standalone-MAC ACK; emulated memory updates |
| E3 | Bad command MAC | Inject/replay a frame with a corrupted command MAC (PM3 raw, or a relay) | Emulator NAKs, drops the session (log: `command MAC mismatch, dropping session`), **does not crash**; a fresh AUTHENTICATE recovers |

### Group N — Regression (secure messaging must not disturb the plain paths)

| ID | Scenario | Expected |
|----|----------|----------|
| N1 | Plain (non-SEC_MSG) UL-AES read / write / emulate | Unchanged — no spurious CMAC switch, no extra re-auth |
| N2 | Ultralight-C, NTAG, plain Ultralight read / write / emulate | Unchanged (shared poller/listener dispatch and the refactored `cmac8`/transceiver must not affect non-AES types) |

## Sign-off

| Field | Value |
|-------|-------|
| Firmware build | `git describe` / commit |
| Card(s) used | MF0AES20 UID(s) |
| Reader used for E-group | |
| Tester / date | |
| Result | X1–X3, R1–R4, W1–W3, (E1–E3), N1–N2 |

Once X1 + W1 (or R1 + W1) pass with correct multi-exchange counter behaviour, the CMAC framing
is confirmed against real silicon and the feature is clear for a PR.

## HW validation result (2026-08-13)

**Reader-side R1 + R2 + R4 + W1 PASS on a real MF0AES20** (1 Flipper + PM3 kit). Full CMAC read of all
60 pages with correct **+2 command-counter progression across ~60 exchanges** and zero MAC mismatches
(data matched the PM3 ground truth); the 3 counters read over CMAC; key pages `0x30`–`0x37` masked to
zero; CMAC write-back of data pages `0x04`–`0x27` MAC-wrapped with +2 write-counter progression, PM3
re-read confirming the overwrite. So the **SV2 session key, CMAC byte order, and counter step are all
confirmed correct** on silicon.

**Bug found + fixed by this testing (commit `ce541e129`):** the CMAC path never engaged because a real
card **mutes** (frame-wait `Timeout`) the first plain command in a secure session instead of sending a
`Protocol` NAK, and the read/write detection gates only accepted `Protocol`. Fixed by also accepting
`Timeout`. This is precisely the R1/W1 "must go past the first frame" gate doing its job — the failure
was invisible until a multi-frame exchange ran on real hardware.

Out of scope for this kit (need a 2nd Flipper / CMAC reader): **X-group**, **E-group**, **W2/W3**.
