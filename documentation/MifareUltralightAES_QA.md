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
- **Flipper:** Emulate the captured dump.
- **PM3:** `hf mfu info` (identity/version/counters/config/signature) **and** an AES-authenticated
  read (all-zero key) against the emulation.
- **Expect:** every field matches the real card; PM3 authenticates against the emulated card and
  reads the (emulated) memory.

### P8 — Random ID reveal (PM3 sets `RID_ACT`)
- **PM3:** set `RID_ACT` (`0x29` byte 0 bit0 = OR `0x01`). Re-select — the card now presents a
  random anticollision UID.
- **Flipper:** Read.
- **Expect:** Flipper authenticates (UIDRetrKey) and **reveals the real static UID**
  `04 7B A2 C2 45 13 90` despite the random UID; info view shows `Random ID: on`.
- **Restore:** clear `RID_ACT`.

### P9 — Config-decoder cross-check (PM3 sets, Flipper decodes)
- **PM3:** toggle each of `AUTH0`, `PROT`, `CNT_RD_EN`, `RID_ACT`, `SEC_MSG_ACT` (one at a time), plus
  a **small, safe** `AUTH_LIM` value if you want (e.g. leave it — optional and risky).
- **Flipper:** Read + full info after each.
- **Expect:** the info-view decoder reflects each change exactly (this is the cleanest way to prove
  the config renderer). **Restore** each bit before moving on.

### P10 — Secure messaging (CMAC) → the critical gate
- **PM3:** set `SEC_MSG_ACT` (`0x29` byte 0 bit1 = OR `0x02`). Reversible with a plain write while
  the card stays open.
- Then follow **[`MifareUltralightAES_SecureMessaging_QA.md`](MifareUltralightAES_SecureMessaging_QA.md)**
  — with this kit you can run its **reader-side** rows (R1 full read over CMAC, R2 counters over
  CMAC, W1 write-back over CMAC). Watch for the `UL-AES: plain read failed post-auth, switching to
  secure messaging` marker and **zero** `MAC mismatch` warnings. The emulation-side CMAC rows (E/X)
  need a CMAC reader or 2nd Flipper — out of scope for this kit.
- **Restore:** clear `SEC_MSG_ACT`.

### P11 — Regression (non-UL-AES must be unaffected)
- **Flipper:** Read / emulate a **Ultralight-C**, an **NTAG** (e.g. NTAG215), and a **plain
  Ultralight**.
- **Expect:** unchanged behavior — the shared poller/listener changes and the new signature
  type-branch must not perturb these (their 32-byte signature path is untouched).

## Sign-off

| Field | Value |
|-------|-------|
| Firmware build | `git describe` / commit (expect `feat/nfc-ultralight-aes`, dirty=false) |
| OG card UID | `04 7B A2 C2 45 13 90` |
| PM3 build | |
| Tester / date | |
| Result | P1–P11 (note which CMAC rows ran) |
| Known gaps (this kit) | W2 clone-to-blank, Group X, emulation-side CMAC |

Once **P1–P9 + P11** pass and the SEC_MSG reader-side rows (P10 → R1/R2/W1) pass with correct
multi-exchange counter behavior, the feature is validated on real silicon to the extent this kit
allows, and PR #1058 can leave draft (noting the emulation-side CMAC gap for a follow-up with a
2nd Flipper).
