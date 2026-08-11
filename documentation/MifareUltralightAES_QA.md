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

### P4 — Dictionary attack, manual unlock, wrong key
- **Dict:** Read → Unlock → Run Dictionary Attack → recovers the all-zero key.
- **Manual:** Read → Unlock → Enter Key Manually → `00…00` → same result.
- **Wrong key:** enter a deliberately wrong key → **clean failure** (falls back to open pages), no
  hang, no infinite re-select, and — since `AUTH_LIM` is unlimited — no lock-out risk.

### P5 — Key management (UI, no card)
- **Flipper:** Extra Actions → **MIFARE Ultralight AES Keys** → add a site key.
- **Expect:** it persists and is offered on subsequent Unlock/dict attempts.

### P6 — Write-back to the OG card (PM3-verified)
- **Keep Key:** from a saved dump, change one user data page, **Write (Keep Key)** to the OG card.
  PM3 re-dump: that data page changed, **DataProtKey unchanged**.
- **Copy Key:** **Write (Copy Key)** a dump whose DataProtKey differs → PM3 confirms pages `0x30`–
  `0x33` written and the new key authenticates. (Config/lock pages must stay untouched either way.)
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
