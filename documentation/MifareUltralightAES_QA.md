# MIFARE Ultralight AES (MF0AES20) — full-feature QA validation plan

End-to-end hardware validation for the whole UL-AES feature (identity, originality signature,
AES 3-pass auth + protected read, dictionary attack / manual unlock, key management, write-back,
emulation, one-way counters, config decode, Random ID, and secure messaging).

The **secure-messaging (CMAC)** slice has its own deep-dive — see
[`MifareUltralightAES_SecureMessaging_QA.md`](MifareUltralightAES_SecureMessaging_QA.md). It is the
one part with **no reference implementation** (PM3 only detects the `SEC_MSG_ACT` bit), so it is the
critical merge gate; this document covers everything else and hands off to it at Phase 10.

## Your equipment (this plan is scoped to it)

- **One genuine MF0AES20** ("OG card"). As shipped it is factory-default: DataProtKey **and**
  UIDRetrKey both all-zero, `SEC_MSG` off, `RID` off, fully open (`AUTH0` protects nothing),
  `AUTH_LIM` unlimited, config unlocked (`CFGLCK` = 0), key pages unlocked. Confirmed via
  `hf mfu info`.
- **Proxmark3 (Iceman)** — three roles: (a) capture ground-truth dumps, (b) personalize the card
  into each test configuration, (c) act as the external reader for emulation tests. It **cannot**
  speak secure messaging.
- **One Flipper** running this firmware (device under test). Enable **Debug** logging
  (Settings → System → Log Level → Debug) so the per-feature markers are visible.

### What this kit can and cannot cover

| Area | Covered with this kit? |
|------|------------------------|
| Identity / read / config decode / counters | ✅ Flipper vs PM3 ground truth |
| Originality signature (capture + emulate) | ✅ Flipper capture, PM3 verifies emulation |
| AES auth + protected read, dict/unlock | ✅ after PM3 sets a protected region |
| Write-back (Keep/Copy Key) to the OG card | ✅ PM3 re-dump verifies |
| Emulation → external reader | ✅ PM3 is the reader |
| Random ID reveal | ✅ after PM3 sets `RID_ACT` |
| **Reader-side** secure messaging (read/counter/write over CMAC) | ✅ after PM3 sets `SEC_MSG_ACT` |
| **Clone to a blank** (W2) | ❌ needs a 2nd blank UL-AES card |
| **Flipper↔Flipper** cross-validation (Group X) | ❌ needs a 2nd Flipper |
| **Emulation-side** secure messaging (reader reads an emulated CMAC card) | ❌ needs a CMAC-capable reader or a 2nd Flipper — PM3 can't |

The two ❌ CMAC items are the only feature paths this kit leaves unvalidated. Note them at sign-off.

## Memory / config map (authoritative — from this firmware's own decoder + datasheet)

| Item | Location |
|------|----------|
| DataProtKey | pages `0x30`–`0x33` |
| UIDRetrKey | pages `0x34`–`0x37` |
| **CFG** page | `0x29`: byte 0 bit0 = `RID_ACT`, bit1 = `SEC_MSG_ACT`; **byte 3 = `AUTH0`** |
| **ACCESS** page | `0x2A`: byte 0 bit2 = `CNT_RD_EN`, bit3 = `CNT_INC_EN`, bit6 = `CFGLCK`, bit7 = `PROT`; **bytes 2–3 = `AUTH_LIM`** |
| **LOCK_KEYS** page | `0x2D`: byte 0 = per-key lock bits |

> ⚠️ PM3's `hf mfu info` config decode uses a legacy Ultralight template for MF0AES20 (it labels
> pages `0x34`–`0x37`, actually the UIDRetrKey, as `cfg0/cfg1/PWD/PACK`). **Trust this table, not
> PM3's labels,** for where the config bits live.

## Safety & reversibility rules (read before any PM3 write)

1. **Never** write `AUTH_LIM` (`0x2A` bytes 2–3), `CFGLCK` (`0x2A` bit6), `LOCK_KEYS` (`0x2D`), or
   the static lock bytes. A wrong value there can permanently lock or brick the card.
2. Every personalization in this plan is **reversible** *because the card stays open* (`AUTH0` off
   for reads / plain writes allowed). Once you set `AUTH0`/`PROT`, plain writes to config need auth;
   always **restore `AUTH0`/`PROT` to open first**, then any other change.
3. **Read-modify-write only.** Before setting a bit in a page, read the page's current 4 bytes,
   change *only* the target bit, keep the other three bytes. Page `0x29` byte 3 is `AUTH0` — never
   clobber it.
4. Keep the **Phase 0** pristine dump as ground truth for every diff.
5. PM3 command syntax for MF0AES20 is recent/build-dependent — confirm with `hf mfu --help` /
   `hf mfu wrbl -h`. On the open card, `hf mfu rdbl -b <blk>` / `hf mfu wrbl -b <blk> -d <8 hex>`
   need no key. **Paste the current page bytes and the exact `-d` value will be computed for you**
   so `AUTH0` is never clobbered.

## Phase 0 — Baseline capture (PM3, no change)

1. `hf mfu info` → record UID, version, all three counters, config, and the `Known UL-AES keys`
   (expect Data + UID key = all-zero, `( ok )`).
2. Dump raw memory (with the all-zero key) and save it — this is the ground truth every later phase
   diffs against.
3. Note the raw bytes of pages `0x29`, `0x2A`, `0x2D` — you'll read-modify-write these later.

## Test matrix

Record Pass/Fail + notes per row. Restore the card to open/factory (Phase 0 state) between phases
that personalize it.

### P1 — Identity & read (open card, no change)
- **Flipper:** NFC → Read.
- **Expect:** type `Mifare Ultralight AES`, UID `04 7B A2 C2 45 13 90`, ATQA `0044`, SAK `00`,
  `60/60` pages, counters `0/0/0`. Full info → config decode matches PM3 (Auth off, unlimited, user
  cfg open, `RID` off, `SEC_MSG` off, key lock N).

### P2 — Originality signature (capture + PM3-verified emulation, no change)
- **Flipper:** Read → **Save** → inspect the `.nfc` for an **`AES signature:`** line (= the 48 bytes
  PM3 showed). Then **Emulate** that dump.
- **PM3:** `hf mfu info` against the emulation.
- **Expect:** the saved `.nfc` carries the 48-byte signature; PM3 prints the **Tag Signature** block
  and **`Signature verification: successful`** (not the old READ-SIGNATURE timeout).

### P3 — AES auth + protected read (PM3 creates a protected region)
- **PM3 (record `0x29`/`0x2A` first):** set `AUTH0` (`0x29` byte 3) to the first data page you want
  protected (e.g. `0x10`) **and** `PROT` (`0x2A` byte 0 bit7 = OR `0x80`) so reads of the protected
  region require auth. Leave `AUTH_LIM`/`CFGLCK` alone.
- **Flipper:** Read → open pages read, protected pages need auth → **Unlock → Run Dictionary
  Attack** (finds all-zero) *or* **Enter Key Manually** `00…00` → protected pages now read.
- **Expect:** all pages match the Phase 0 ground truth; recovered key shown + saved with the dump.
- **Restore:** `PROT` = 0, `AUTH0` back to its open value.

### P4 — Dictionary attack, manual unlock, wrong key (+ non-standard-key give-up)

> **UL-AES has no one-shot "enter password" box.** Manual key entry = adding the key to the **User
> dictionary** (Extra Actions → **MIFARE Ultralight AES Keys → Add**). On an incomplete read the AES
> dictionary attack auto-launches and tries the **User** dict, then the **System** dict.

- **Dict:** with the card protected (P3's `AUTH0=0x10`), Read → the AES dict attack auto-runs →
  recovers the all-zero key from the System dict → full `60/60` read.
- **Manual (correct key):** AES Keys → **Add** the key → Read → the header shows **"MFUL AES User
  Dictionary"** and succeeds. Proves the manual-entry path feeds the poller.
- **Wrong key → clean fall-over:** put *only* a bogus key in the User dict → Read → User dict fails
  cleanly (no hang, no infinite re-select), auto-advances to the System dict, succeeds with all-zero.
  `AUTH_LIM` unlimited → no lock-out.

#### P4c — non-standard DataProtKey → terminal give-up (all-plain, fail-safe)
The rows above never show the *all-keys-exhausted* state, because the System dict's all-zero key
always wins on a factory card. Force it by giving the card a key that is in no dictionary.
**Safety design:** protect from `0x2A` (not `0x10`) so the config page `0x29` stays *below* `AUTH0`
and remains plain-writable — then *every* write is plain and the throwaway key never has to be
authenticated against (sidesteps the byte-order / partial-key traps entirely).

Throwaway key `A0A1A2A3A4A5A6A7A8A9AAABACADAEAF`. Blocks: `0x29 = 41`, `0x30–0x33 = 48–51`.

1. **Set the key** (card open → plain writes): `wrbl -b 48 -d A0A1A2A3`, `-b 49 -d A4A5A6A7`,
   `-b 50 -d A8A9AAAB`, `-b 51 -d ACADAEAF`.
2. **Verify it changed:** `hf mfu info` → `Known UL-AES keys` no longer lists the all-zero **Data
   key** as `( ok )` (only the UID key stays `( ok )`). This is the fail-safe checkpoint — the card
   is still open, so if a write didn't take you've lost nothing.
3. **Protect:** `wrbl -b 41 -d 0000002A`.
4. **Flipper Read →** the AES dict attack exhausts **User + System** dicts, **none authenticate** →
   **semi-success**, partial pages (~`42/60`), no key recovered, **no hang, no lock-out**.
5. **Restore (reopen first, then zero the key — all plain):** `wrbl -b 41 -d 0000003C`, then
   `-b 48 -d 00000000` … `-b 51 -d 00000000`.
6. **Confirm:** `hf mfu info` → all-zero Data key `( ok )`, `AUTH0` open.

> **Hardware-learned safety notes.** Key pages are plain-writable **only while the card is open**
> (`AUTH0 > 0x3B`); a plain key-page write on a still-protected card **times out** — the card mutes
> it, nothing is written, no harm. If that happens, reopen with an *authenticated* write
> (`wrbl -b 41 -d 0000003C -k <AES key>`) and retry. Never touch `AUTH_LIM` / `CFGLCK` / `LOCK`.

#### DataProtKey byte order (validated gotcha)
Raw `wrbl` writes bytes in **tag-memory order**; the AES engine — and **both** the Flipper's manual
entry *and* PM3 `hf mfu aesauth --key` — use the **reverse**. A card whose pages `0x30–0x33` read
`A0A1A2A3 A4A5A6A7 A8A9AAAB ACADAEAF` therefore authenticates with key
**`AFAEADACABAAA9A8A7A6A5A4A3A2A1A0`**. Cross-validated on hardware: that reversed value unlocks on
the Flipper *and* returns `Authentication with DataProtKey … ( ok )` on PM3. This is the datasheet
§8.6.3 / PM3 `SwapEndian16` convention; our dumps store the key reversed so they write back to tag
memory verbatim (`mf_ultralight.c:802`, `mf_ultralight_poller.c:820`). Enter a manually-recovered key
in **AES order** (reversed from what raw `wrbl` shows), not tag-memory order.

### P5 — Key management (UI, no card)
- **Flipper:** Extra Actions → **MIFARE Ultralight AES Keys** → add a site key.
- **Expect:** it persists and is offered on subsequent Unlock/dict attempts.

### P6 — Write-back to the OG card (PM3-verified)

> **Write behavior (from the poller).** Offered only on a fully-read UL-AES dump, as two items
> **Write (Keep Key)** / **Write (Copy Key)**. The write **authenticates the target first** with a
> dictionary key (so the target's *current* key must be in a dict — all-zero is), then writes.
> **Scope is safe:** data pages `0x04–0x27` always; **config/lock `0x28–0x2F` are never written**;
> Keep Key stops before the key; Copy Key additionally writes **DataProtKey `0x30–0x33`** (UIDRetrKey
> `0x34–0x37` never written). Match gate is by **type, not UID**. Key pages are written **verbatim**
> from the dump (tag-memory/reversed order) → PM3 verifies with the **reversed** key. Enable Debug
> logging to see `UL write: key kept…` / `key overwrite with source`.

- **Keep Key (no file edit needed):** Read + Save the factory card → dump A (all-zero). PM3 drops a
  marker on a data page (`wrbl -b 10 -d DEADBEEF`, card open). Flipper: load A → **Write (Keep Key)**.
  PM3 re-dump: **page 10 back to `00000000`** (data pages *are* written), **pages 48–51 still
  all-zero**, `0x29/0x2A/0x2D` unchanged, `hf mfu info` → all-zero Data key `( ok )`.
- **Copy Key (one `.nfc` edit — byte-order proof):** the `.nfc` is plain-text; the DataProtKey lives
  in the **page lines** `Page 48–51` (the `DataProtKey:` on-screen is a render, not a save field).
  Edit them to a non-zero key in tag-memory order (e.g. `00 11 22 33 / 44 55 66 77 / 88 99 AA BB /
  CC DD EE FF`), load → **Write (Copy Key)** (auths with the card's current all-zero key, then writes
  data + new key). PM3: `dump` shows pages 48–51 = that value verbatim; the all-zero Data key is no
  longer `( ok )`; **`hf mfu aesauth --key FFEEDDCCBBAA99887766554433221100` → `( ok )`** — the
  reversed key authenticates, proving the write path's byte order end-to-end. Config/lock untouched.
- **Restore:** card stays open (Copy Key never touches `AUTH0`), so PM3 plain-writes pages 48–51 back
  to `00000000`, then `hf mfu info` → all-zero Data key `( ok )`.
- *(W2 clone-to-blank needs a 2nd blank UL-AES card — skip with a single OG card.)*

### P7 — Emulation to an external reader (PM3 is the reader)
The listener-side counterpart to P3 — the Flipper emulates, PM3 reads. Reuse P6's dumps (A =
factory/all-zero key, B = key `00112233…FF`).

- **P7a — auth + field fidelity (dump A):** Emulate A → PM3 `hf mfu info` (identity/version/counters/
  config/**signature verifies**) matches the real card, **and** `hf mfu aesauth --key 000…0` →
  `( ok )`. That `( ok )` proves the listener completes the 3-pass AES challenge-response as a card.
- **P7b — listener byte order + wrong-key (dump B):** Emulate B → `hf mfu aesauth --key
  FFEEDDCCBBAA99887766554433221100` (the **reversed** key) → `( ok )`; `--key 000…0` (wrong) →
  **fails**. Confirms the listener extracts the AES key as `reverse(pages 0x30–0x33)` and rejects bad
  keys.
- **P7c — AUTH0 enforcement (optional):** edit a copy of A, `Page 41` → `00 00 00 10`, emulate; PM3
  `hf mfu dump` without a key vs `-k 000…0` reads the `0x10+` region differently. Caveat: the listener
  **rolls over** restricted reads (returns early-page data) rather than NAKing, so the unauth signal
  is *rolled data*, not a clean error — the definitive proof stays the `aesauth` result above.
- **Result (HW):** P7a/b/c all pass — emulation is indistinguishable from the real card and the
  listener AES auth path (auth grant, byte order, wrong-key reject, AUTH0 gate) is validated.

### P8 — Random ID detection & display (PM3 sets `RID_ACT`) — PASSED
- **PM3:** set `RID_ACT` (`0x29` byte 0 bit0 = OR `0x01`, keeping AUTH0): `hf mfu wrbl -b 41 -d
  0100003C`. Re-select — the card now presents a **random 4-byte** anticollision UID (ISO14443-3
  single-size, first byte `0x08`); the real 7-byte UID is hidden until an authenticated read.
- **Flipper:** Read → shows the random UID + `Random ID: on`.
- **HW result (PASS):** detected as UL-AES, the random UID is read, and the info view decodes
  `Random ID: on`.
- **UID length note:** the Flipper's **4-byte** display is the literal random anticollision UID
  (`08 …`); PM3's **7-byte with `00 00 00`** trailing is the same value zero-padded to the normal
  Ultralight UID width. Neither is the real UID.
- **Restore:** `hf mfu wrbl -b 41 -d 0000003C`.
- **Scope:** retrieving/showing the *real* static UID behind a random one is a **separate planned
  capability, tracked as P12** (below) — out of P8's scope.

### P9 — Config-decoder cross-check (PM3 sets, Flipper decodes) — PASSED
One bit at a time (plain, reversible writes on the open card): PM3 write → Flipper Read + full info →
confirm the one decoded line → restore → next. Baseline `0x29 = 00 00 00 3C`, `0x2A = 8C 05 00 00`
(keep `0x2A` byte 1 = `05` VCTID, bytes 2-3 = `00`).

| Bit | PM3 write | Info shows | Restore |
|-----|-----------|------------|---------|
| SEC_MSG_ACT | `wrbl -b 41 -d 0200003C` | `Secure msg: on` | `wrbl -b 41 -d 0000003C` |
| RID_ACT (P8) | `wrbl -b 41 -d 0100003C` | `Random ID: on` | `wrbl -b 41 -d 0000003C` |
| CNT_RD_EN off | `wrbl -b 42 -d 88050000` | `Counter 2: rd auth / inc open` | `wrbl -b 42 -d 8C050000` |
| CNT_INC_EN off | `wrbl -b 42 -d 84050000` | `Counter 2: rd open / inc auth` | `wrbl -b 42 -d 8C050000` |
| both counters off | `wrbl -b 42 -d 80050000` | `Counter 2: rd auth / inc auth` | `wrbl -b 42 -d 8C050000` |
| AUTH0 (≈P3) | `wrbl -b 41 -d 0000002A` | `Auth from: page 0x2A (r+w)` | `wrbl -b 41 -d 0000003C` |

- **⚠️ Never test CFGLCK** (`0x2A` bit 6 = `0x40`, **permanent** config lock) or **AUTH_LIM** (`0x2A`
  bytes 2-3, arms a failed-auth **lock-out**) — every `0x2A` write above is `0x8*`/`0x0C`, no bit 6.
- **HW result (PASS):** each decoded line flips exactly as expected.

### P10 — Secure messaging (CMAC) → the critical gate
The one part with **no reference implementation** (PM3 detects the `SEC_MSG_ACT` bit but **cannot
speak CMAC**). Full row detail: **[`MifareUltralightAES_SecureMessaging_QA.md`](MifareUltralightAES_SecureMessaging_QA.md)**.
With this kit (1 Flipper + PM3 + 1 card) run the **reader-side** rows **R1/R2/W1/R4**; the X-group
(2nd Flipper), E-group (CMAC reader) and W2 (blank card) are out of scope.

> **⚠️ CRITICAL recoverability rule.** Once `SEC_MSG_ACT` is on, any config write in a *protected*
> region needs a **CMAC-authenticated write** — which **PM3 can't do** and the Flipper's Write
> **skips config pages**. So **never set `AUTH0 ≤ 0x29`** while `SEC_MSG_ACT` is on, or you may be
> unable to clear it. Use **`AUTH0 = 0x2A`**: it keeps `0x29` *below* AUTH0 (PM3 can plain-restore it)
> yet still forces the Flipper to authenticate — which is what triggers the CMAC session.

**Setup:** `hf mfu wrbl -b 41 -d 0200002A` (SEC_MSG_ACT + AUTH0=0x2A). Confirm `hf mfu rdbl -b 41` →
`02 00 00 2A`. Enable Debug logging.

**Why this triggers CMAC even though data pages stay open:** the Flipper enters an authenticated
session via the **dictionary attack** (an incomplete plain read triggers it) for reads, and
**unconditionally** for writes. On a SEC_MSG card, once authenticated *every* command must be CMAC'd,
so the first post-auth plain `READ`/`WRITE` NAKs at `pages_read == 0` (`mf_ultralight_poller.c:706-718`)
→ the poller switches to CMAC.

- **R1 — full CMAC read:** Read → incomplete → **Unlock with Dictionary** → log shows `UL-AES: plain
  read failed post-auth, switching to secure messaging` → all 60 pages read over CMAC (~15 exchanges
  → validates the **+2 counter step** + MAC byte order), **no `MAC mismatch`**; data matches PM3.
- **R2 — counters over CMAC:** after R1, full info shows counters `0/0/0` (match PM3).
- **W1 — CMAC write-back:** Write (Keep Key) a saved dump → the write auths → log shows the CMAC
  switch on the first page write → data pages written MAC-wrapped, no `MAC mismatch`; verify via PM3
  re-read (data pages are below AUTH0, so PM3 reads them plain).
- **R4 — key pages never leak:** the saved dump's pages `0x30-0x37` read back as zero.

**Restore:** `hf mfu wrbl -b 41 -d 0000003C` (plain — `0x29` is below `AUTH0=0x2A`), then `hf mfu
info` → `Secure msg` off, AUTH0 open, all-zero Data key `( ok )`.

### P11 — Regression (non-UL-AES must be unaffected)
- **Flipper:** Read / emulate a **Ultralight-C**, an **NTAG** (e.g. NTAG215), and a **plain
  Ultralight**.
- **Expect:** unchanged behavior — the shared poller/listener changes and the new signature
  type-branch must not perturb these (their 32-byte signature path is untouched).

### P12 — Random ID real-UID retrieval (FUTURE — implement, then test in isolation)
Planned capability, separate from P8 (which only *detects/displays* RID). Today the real static UID
behind a random one is **not retrieved on an open RID card**: the reveal (`mf_ultralight_poller.c:837`)
only fires after a successful auth, but a plain UL-AES read **skips auth** (`mf_ultralight.c:140`) and
**Unlock** is offered **only on an incomplete read** (`mf_ultralight.c:233,259`) — so an `AUTH0`-off RID
card never authenticates and the real UID is never recovered.
- **Implement:** on RID detection (anticollision `UID0 == 0x08`, or the `RID_ACT` bit after a read),
  auto-attempt an authentication to retrieve the real UID using the **UIDRetrKey**
  (`MfUltralightAesKeyTypeUid`, type `0x01`; dict / all-zero) — not only the DataProtKey the read/unlock
  path uses today (`mf_ultralight.c:138`) — then **display the recovered real UID in the UL-AES info
  view** ("on top of the AES info"), alongside `Random ID: on`. Optionally also offer **Unlock** when
  the presented UID is random even though data read completed.
- **Isolation test:** set `RID_ACT` **only** (open card, `hf mfu wrbl -b 41 -d 0100003C`) → plain Read
  → expect the real UID **auto-retrieved and shown in info** with no manual unlock; cross-check vs PM3's
  known static UID `04 7B A2 C2 45 13 90`. Then set a **non-zero UIDRetrKey ≠ DataProtKey** (pages
  `0x34–0x37`, plain write while open; same tag-memory/reversed byte order as DataProtKey) and confirm
  the reveal uses the **UID** key specifically (fails if only the Data key is tried). Restore both.
- **Interim mechanism check (current build):** force an incomplete read so Unlock appears — set RID
  **and** `AUTH0=0x2A` (keeps `0x29` plain-writable): `hf mfu wrbl -b 41 -d 0100002A` → Read → **Unlock
  with Dictionary** → see whether `04 7B A2 C2 45 13 90` appears (answers: does DataProtKey auth reveal
  it, or is UIDRetrKey required?). Restore plain: `hf mfu wrbl -b 41 -d 0000003C`.

## Sign-off

| Field | Value |
|-------|-------|
| Firmware build | `git describe` / commit (expect `feat/nfc-ultralight-aes`, dirty=false) |
| OG card UID | `04 7B A2 C2 45 13 90` |
| PM3 build | |
| Tester / date | |
| Result | P1–P11 (note which CMAC rows ran) |
| Known gaps (this kit) | W2 clone-to-blank, Group X, emulation-side CMAC |
| Planned follow-up | P12 real-UID retrieval on RID cards (implement + isolation-test separately) |

Once **P1–P9 + P11** pass and the SEC_MSG reader-side rows (P10 → R1/R2/W1) pass with correct
multi-exchange counter behavior, the feature is validated on real silicon to the extent this kit
allows, and PR #1058 can leave draft (noting the emulation-side CMAC gap for a follow-up with a
2nd Flipper).
