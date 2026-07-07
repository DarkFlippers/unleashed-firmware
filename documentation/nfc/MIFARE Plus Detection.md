# MIFARE Plus type detection — NFC TagInfo by NXP v6.2.0

Reference for how the app classifies a MIFARE Plus card into **variant × security level × memory size**.
Derived from static analysis (jadx) of `com.nxp.taginfolite` v6.2.0. Code is DexGuard-obfuscated
(control-flow + string encryption, framework-decoy class names); method bodies recovered with
`jadx --show-bad-code --comments-level debug`. **All byte values are exact**; enum semantics are from
decrypted labels cross-checked against NXP's public MIFARE Plus command set.

## TL;DR

The final label is assembled from four independently-probed dimensions:

- **Family** — always `MIFARE Plus` here (vs Classic / Pro).
- **Sub-type** — `S | X | SE | EV1 | EV2` (and `Engineering`).
- **Security level** — `SL0 | SL1 | SL3` (no SL2 for EV parts).
- **Memory size** — `2K | 4K` (also 1K/8K buckets).

Routing is **by SAK first** (which transport), then per-lane probes:
- **SL1 / Classic-compatible** cards are driven over `MifareClassic`; sub-type comes from **ATS historical-byte fingerprinting**.
- **SL3 / EV** cards are ISO-14443-4 and driven over `IsoDep`; EV level comes from **GET_VERSION** via the bundled TapLinx SDK.
- **EV1/EV2** are gated by the **Read-Signature (`3C`)** originality command (only EV parts answer it).

## Analyzer classes (decoy name → real role)

| Decoy class (package `o`) | Real role | Logcat tag | Transport |
|---|---|---|---|
| `AppCompatMultiAutoCompleteTextView` | MIFARE Classic **+ Plus SL1** analyzer | `TagInfo_MF` | `MifareClassic` |
| `AppCompatRadioButton` | Dedicated MIFARE **Plus (MFP)** analyzer + fingerprint tables | `TagInfo_MFP` | `IsoDep` |
| `com.nxp.nfclib.plus.PlusFactory` | TapLinx SDK — GET_VERSION predicate; builds `IPlusEV1/EV2·SL0/1/3` | — | SDK |
| `com.nxp.nfclib.CardType` / `VirtualCardTypeIdentifier` | SDK card-type enums (string-encrypted) → PLUS_EV1 / PLUS_EV2 | — | SDK |

## Decision procedure

```
NFC tag presented
│
├─ Step 1: SAK gate — NfcA.getSak()
│     SAK ∈ {01,08,09,18,19,28,38,88,98,B8} ──▶ CLASSIC lane (MifareClassic, SL1)
│     SAK == 20 (ISO-14443-4)               ──▶ ISO-DEP lane (IsoDep, SL3/EV)
│
├─ CLASSIC lane  (TagInfo_MF)
│   ├─ Step 2: RATS (E0 80) → capture ATS → C2 (DESELECT)
│   │          match ATS historical bytes vs table ──▶ S | X | SE | Engineering
│   ├─ Step 3: Read-Signature (3C 00); status 90 ⇒ EV generation ──▶ EV1
│   ├─ Step 4: security level = SL1 (answers Classic block auth)
│   └─ Step 5: size = First-Auth probe to increasing key-block addrs (2K/4K/8K)
│
└─ ISO-DEP lane  (TagInfo_MFP)
    ├─ Step 4: SELECT VCA applet (00 A4 04 00 10 A0 00 00 03 96 56 43 41 …) ⇒ SL3
    ├─ Step 3: GET_VERSION (60…AF) → CardType ──▶ EV1 | EV2  (+ storage size)
    └─ size = GET_VERSION storage byte (22=2K, 42=4K)
```

## Step 1 — SAK gate (`notify(NfcA)`)

Returns `true` only for the Classic/Plus SAK set. A second predicate isolates the SmartMX/Pro subset `{28,38,B8}`.

| SAK | Interpretation | Lane |
|-----|----------------|------|
| `08` | Classic 1K / **Plus 2K SL1** | Classic |
| `18` | Classic 4K / **Plus 4K SL1** | Classic |
| `09` | MIFARE Mini | Classic |
| `19` | Classic 2K / Plus 2K variant | Classic |
| `28` | SmartMX w/ Classic 1K | Pro/SmartMX |
| `38` | SmartMX w/ Classic 4K | Pro/SmartMX |
| `88` | NXP Classic 1K (0x80 bit) | Classic |
| `98` `B8` | SmartMX / Pro variants | Pro/SmartMX |
| `01` | Legacy / TNP marker | Classic |
| `20` | ISO-14443-4 → Plus **SL3** / EV, DESFire | ISO-DEP |

## Step 2 — Sub-type S/X/SE via ATS fingerprint

`cancelAll(MifareClassic)` sends **RATS `E0 80`**, keeps the **ATS**, then **`C2`** (DESELECT).
ATS historical bytes are matched **byte-exact** against this table (`AppCompatRadioButton`):

| ATS historical bytes | Sub-type | Product logged | SDK enum |
|----------------------|----------|----------------|----------|
| `C1 05 2F 2F 00 35 C7` | **Plus S** | `MF1SPLUS` | `PLUS_S` |
| `C1 05 2F 2F 01 BC D6` | **Plus X** | `MF1PLUS` | `PLUS_X` |
| `C1 05 21 30 00 77 C1` | **Plus SE** | `MF1SEP10` | `PLUS_SE` |
| `C1 05 20 30 00 AB 9B` | **Plus SE** (alt) | `MF1SEP10` | `PLUS_SE` |
| `4D 46 50 5F 45 4E 47` (`"MFP_ENG"`) | Engineering | — | `ENGINEERING` |
| (no match) | **Plus X** (default) | `MF1PLUS` | `PLUS_X` |

- S-vs-X discriminator is byte[4]: `00`=S, `01`=X (on the `C1 05 2F 2F …` family).
- Cross-check: `C1 05 2F 2F 01 BC D6` is the documented real ATS of MIFARE Plus X.

## Step 3 — EV1/EV2 via Read-Signature + GET_VERSION

- `subscribe()` sends **Read-Signature `3C 00`** (ECC originality). Status `90` ⇒ EV part (Plus S/X do not answer it).
- EV1 vs EV2: `NxpNfcLib.getCardType()` → `CardType` → mapped to `PLUS_EV1`/`PLUS_EV2` in `AppCompatRadioButton.asInterface()`.
- SDK predicate `PlusFactory.INotificationSideChannel(byte[])` parses GET_VERSION: vendor byte `04` (NXP), storage byte `22`=2K / `42`=4K.

## Step 4 — Security level

| Level | Signal | Where |
|-------|--------|-------|
| **SL1** | Answers ISO-14443-3 Classic block auth over `MifareClassic` | Classic lane |
| **SL3** | SELECT VCA applet `A0 00 00 03 96 56 43 41 …` succeeds over ISO-DEP | `cancel(IsoDep)` |
| **SL0** | Personalization state (perso commands open) | MFP enum |

- VCA AID payload `96 56 43 41` = ASCII **"VCA"** (Virtual Card Architecture).
- Full SELECT APDU: `00 A4 04 00 10 A0 00 00 03 96 56 43 41 FF FF FF FF FF FF FF FF 00`.

## Step 5 — Memory size & part number

**SL1 lane** — `cancel(MifareClassic)` sends Plus AES **First-Auth** frames to increasing key-block addresses;
highest that authenticates bounds sector count → size. Payload `70 XX 40 01 00` wrapped in an ISO-DEP I-block:

| Payload | Key block | Implies |
|---------|-----------|---------|
| `70 10 40 01 00` | `0x4010` | base sectors |
| `70 20 40 01 00` | `0x4020` | 2K reachable |
| `70 40 40 01 00` | `0x4040` | 4K reachable |
| `70 64 40 01 00` | `0x4064` | extended / 8K |

**EV lane** — storage size taken directly from GET_VERSION storage byte.

### Plus product codes emitted

| Code | Meaning |
|------|---------|
| `MF1SPLUS60` / `MF1SPLUS80` | Plus **S** 2K / 4K |
| `MF1PLUS60` / `MF1PLUS80` | Plus **X** 2K / 4K |
| `MF1SEP10xx` | Plus **SE** |
| `MF1P2101` | Plus **EV1** 2K |

`getItem()` also resolves Classic part numbers from the GET_VERSION storage byte via a large `switch`
(e.g. `0x44`→MF1ICS50xx, `0xC2`→MF1S50xx Classic EV1, `0xC4`→TNP3100 series).

## Full taxonomy (enums)

- **Family**: `PLUS` (→ "MIFARE Plus"), `CLASSIC`, `PRO`, `TNP`, `TNPP`, `TNP_P`, `TNP_I2C`, `UNKNOWN`.
- **Plus sub-type**: `PLUS_S`, `PLUS_X`, `PLUS_SE`, `PLUS_EV1`, `PLUS_EV2`, `PLUS_S_OR_SE`, `ENGINEERING`, `UNKNOWN`.
- **Security level**: `SL0`, `SL1`, `SL3`, `UNKNOWN`.
- **Memory size**: `OneK`, `TwoK`, `FourK`, `EightK`, `B256`, `B512`, `UNKNOWN`.

## Command reference

| Bytes | Purpose | Source method |
|-------|---------|---------------|
| `E0 80` | RATS — request ATS | `cancelAll(MifareClassic)` |
| `C2` | ISO-DEP S(DESELECT) | `cancelAll(MifareClassic)` |
| `3C 00` | Read Signature (ECC originality) — EV test | `subscribe()` |
| `70 XX 40 01 00` | Plus AES First-Auth, key-block XX — size probe | `cancel(MifareClassic)` |
| `00 A4 04 00 10 A0 00 00 03 96 56 43 41 FF FF FF FF FF FF FF FF 00` | SELECT VCA applet — SL3 test | `cancel(IsoDep)` |
| `60 … AF` | GET_VERSION (chained) — EV level + storage size | `PlusFactory` / `NxpNfcLib` |

## Source references

- `o/AppCompatMultiAutoCompleteTextView.java` — `notify(NfcA)` (SAK gate), `cancel`/`cancelAll(MifareClassic)` (size probe, RATS/ATS), `subscribe()` (Read-Signature EV test), `getItem()` (part-number resolver).
- `o/AppCompatRadioButton.java` — `:106–192` ATS fingerprint constants; `:251–517` taxonomy enums; `:668` CardType→EV mapping; `:731` VCA SELECT.
- `com/nxp/nfclib/plus/PlusFactory.java:195` — GET_VERSION predicate; interfaces `IPlusEV1/EV2·SL0/1/3`.

## Constant values (jadx substituted misleading named constants)

DexGuard/jadx replaced raw command bytes with coincidentally-matching named constants; resolved values:

| Named constant (jadx) | Value |
|---|---|
| `READ_SIGNATURE_COMMAND` | `0x3C` |
| `MKNO_AES` | `0x80` |
| `MKNO_3K3DES` | `0x40` |
| `GET_KEY_VERSION` | `0x64` |
| `CHANGE_FILE_SETTINGS` | `0x5F` (`_`) |
| `SELECT_PICC` | `0xA4` |
| `CMD_COMMIT_TRANSACTION` | `0xC7` |
| `UPDATE_BINARY_COMMAND` | `0xD6` |
| `READ_RECORDS_COMMAND` | `0xAB` |
| `LRP_AUTH_NONFIRST` | `0x77` |
| `TRAILER_IMPLICIT` (BC PSSSigner) | `0xBC` |
