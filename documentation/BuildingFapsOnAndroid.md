# Building FAPs on Android with Termux

> Contributed by **[@CamsShaft](https://github.com/CamsShaft)**, adapted from
> [BUILD-FLIPPER-FAPS-IN-TERMUX](https://github.com/CamsShaft/BUILD-FLIPPER-FAPS-IN-TERMUX) (MIT).

Set up `ufbt` (micro Flipper Build Tool) inside [Termux](https://termux.dev/) on a stock Android phone and
compile Flipper Zero applications (`.fap`) on-device. No PC, no proot, no virtual machine.

**Scope:** this builds **applications**, not the firmware. Building the firmware itself with `fbt` is not
covered here — see [HowToBuild.md](HowToBuild.md).

Originally worked out on a Samsung Galaxy S22 (SM-S901W, aarch64). The paths below assume an **aarch64**
device and the default Termux prefix (`$PREFIX` = `/data/data/com.termux/files/usr`). Nothing here is
guaranteed to match your device exactly, but the failure modes are documented in
[Troubleshooting](#troubleshooting).

## Why this needs extra work

ufbt downloads a prebuilt `arm-none-eabi-gcc` cross-toolchain built for ordinary Linux against glibc.
Termux runs on Android's Bionic libc with a non-standard filesystem layout, and every ELF binary in that
toolchain asks for `/lib/ld-linux-aarch64.so.1` as its dynamic linker — which does not exist on Android.

The fix is to install Termux's glibc compatibility layer, then run each toolchain binary through glibc's
own linker via a small wrapper script.

> **Do not run `patchelf` on the toolchain binaries.** It corrupts their ELF segment layout and you get
> segfaults out of `cc1`. The wrappers below leave the binaries byte-for-byte untouched.

## Prerequisites

- An aarch64 Android device
- [Termux](https://f-droid.org/en/packages/com.termux/) installed **from F-Droid** — the Google Play build
  is obsolete and will not work
- An internet connection and a couple of GB free for the toolchain, SDK and glibc

## 1. Base packages

```sh
pkg update && pkg upgrade -y
pkg install python python-pip git file -y
```

`file` is not optional — the wrapper loop in step 6 uses it to tell ELF binaries apart from everything else.

## 2. glibc compatibility layer

```sh
pkg install glibc-repo -y
pkg install glibc-runner -y
```

These have to be two separate commands: `glibc-repo` is what adds the glibc repository that
`glibc-runner` is served from. Together they install a full glibc under `$PREFIX/glibc/`, including the
dynamic linker we need. Verify it:

```sh
ls $PREFIX/glibc/lib/ld-linux-aarch64.so.1
```

> **Never set `LD_PRELOAD` to glibc's `libc.so.6` globally.** It poisons Termux's own Bionic binaries,
> Python included, and the breakage is not obvious. The wrappers in step 6 remove any need for it.

## 3. Install ufbt and the Python build dependencies

```sh
pip install ufbt colorlog heatshrink2 pyelftools scons pillow
```

| Package | Needed for |
|---|---|
| `colorlog` | build script logging |
| `heatshrink2` | icon and asset compression |
| `pyelftools` | the `FASTFAP` post-processing step |
| `scons` | the build system — needed as a standalone install, the toolchain's bundled copy is not usable here |
| `pillow` | icon conversion; without it the only error you get is a bare `convert: No such file or directory` |

If the `pillow` wheel fails to build, use the Termux package instead: `pkg install python-pillow`.

## 4. Point ufbt at the Unleashed SDK

**ufbt defaults to the official firmware SDK.** To build against Unleashed, pass this repository's update
index:

```sh
ufbt update --index-url=https://up.unleashedflip.com/directory.json --channel=dev
```

Channels are `dev` and `release`. The index URL is also listed in the [ReadMe](../ReadMe.md#-links).
**Every later `ufbt update` needs the same flags**, otherwise you silently fall back to the official SDK
and your app is built against the wrong API.

If `~/.ufbt/toolchain/` is still empty after that, any ufbt invocation bootstraps the toolchain:

```sh
ufbt -h
```

An error mentioning `python3` at this point is expected — step 5 fixes it. Check that the toolchain landed:

```sh
ls ~/.ufbt/toolchain/aarch64-linux/bin/arm-none-eabi-gcc
ls ~/.ufbt/toolchain/aarch64-linux/arm-none-eabi/bin/as
ls ~/.ufbt/toolchain/aarch64-linux/libexec/gcc/arm-none-eabi/*/cc1
```

## 5. Repoint the toolchain's Python

The toolchain ships its own glibc-linked Python, which cannot run here. Point every `python3*` entry at
Termux's Python instead:

```sh
TOOLCHAIN="$HOME/.ufbt/toolchain/aarch64-linux"

for py in "$TOOLCHAIN"/bin/python3*; do
    [ -e "$py" ] || [ -L "$py" ] || continue
    ln -sf "$PREFIX/bin/python3" "$py"
done
```

## 6. Wrap the toolchain binaries

This is the step everything else depends on. Each wrapper invokes the real binary through glibc's dynamic
linker with an explicit `--library-path`, so the binary itself is never modified.

```sh
TOOLCHAIN="$HOME/.ufbt/toolchain/aarch64-linux"
GLIBC_LIB="$PREFIX/glibc/lib"

wrap() {
    for f in "$@"; do
        [ -f "$f" ] || continue
        case "$f" in *.real | *.so | *.so.*) continue ;; esac
        [ -e "$f.real" ] && continue
        file -b "$f" | grep -q '^ELF' || continue

        mv "$f" "$f.real"
        cat > "$f" <<WRAPPER
#!$PREFIX/bin/bash
exec "$GLIBC_LIB/ld-linux-aarch64.so.1" --library-path "$GLIBC_LIB:$TOOLCHAIN/lib" "$f.real" "\$@"
WRAPPER
        chmod +x "$f"
        echo "wrapped $f"
    done
}

wrap "$TOOLCHAIN"/bin/arm-none-eabi-*
wrap "$TOOLCHAIN"/arm-none-eabi/bin/*
wrap "$TOOLCHAIN"/libexec/gcc/arm-none-eabi/*/*
```

Those three globs are three separate directories of ELF binaries. Missing one gives a different failure:

| Directory | Contains | Error if not wrapped |
|---|---|---|
| `bin/` | `arm-none-eabi-gcc`, `-g++`, ... | `cannot execute` / `No such file or directory` |
| `arm-none-eabi/bin/` | `as`, `ld`, `ar`, `objcopy`, ... | `cannot execute as` |
| `libexec/gcc/arm-none-eabi/<gcc-version>/` | `cc1`, `cc1plus`, `collect2`, `lto1` | `cannot execute cc1` |

The guards matter, especially if you re-run this later:

- `*.so` is skipped because `liblto_plugin.so` lives in `libexec/` and is a shared library, not an
  executable. Wrapping it gives `invalid ELF header`.
- `*.real` and the `$f.real` check keep the function idempotent, so re-running it after a toolchain update
  cannot wrap a wrapper.
- In `bin/` only `arm-none-eabi-*` is wrapped, deliberately: `bin/python3` now points at Termux's Bionic
  Python and must **not** go through glibc's linker.

**`ufbt update` can replace the toolchain — re-run steps 5 and 6 after every update.**

## 7. Build

```sh
git clone https://github.com/<owner>/<some-flipper-app>.git
cd <some-flipper-app>
ufbt
```

A successful build ends like this:

```
scons: Entering directory '...'
        CC      ...
        LINK    ...
        FAP     ...
        FASTFAP ...
        APPCHK  dist/some_app.fap
                Target: 7, API: 88.2
```

The `.fap` is in `dist/`. Check the reported API against the firmware on your Flipper — a mismatch means
the app will refuse to load.

## 8. Getting the .fap onto the Flipper

`ufbt launch` and `ufbt flash_usb` need a serial connection to the Flipper (`/dev/ttyACM*`).
**On a stock, non-rooted Android this does not work** — Android does not expose USB CDC-ACM device nodes
to Termux, so there is nothing for ufbt to talk to. Only a rooted device with a working CDC-ACM driver can
use those commands.

Copy the file out instead. FAPs live in `/ext/apps/<Category>/` on the microSD card
(see [AppsOnSDCard.md](AppsOnSDCard.md)):

```sh
termux-setup-storage                        # one-time: grants Termux access to shared storage
cp dist/some_app.fap ~/storage/downloads/
```

From `Downloads` the file is visible to every other app on the phone, so you can move it to the Flipper
the way you normally would — the Flipper Mobile App, or a USB-C microSD card reader.

## Troubleshooting

### `cannot execute cc1` / `cannot execute as`

One of the three directories in step 6 was not wrapped. Re-run the `wrap` calls — they are safe to repeat.

### `liblto_plugin.so: invalid ELF header`

A shared library got wrapped. Restore it:

```sh
LIBEXEC="$(echo "$HOME"/.ufbt/toolchain/aarch64-linux/libexec/gcc/arm-none-eabi/*)"
mv "$LIBEXEC/liblto_plugin.so.real" "$LIBEXEC/liblto_plugin.so"
```

### `Segmentation fault` from cc1

The binary was corrupted, almost always by `patchelf`. Re-download the toolchain and re-apply the wrappers:

```sh
rm -rf ~/.ufbt/toolchain
ufbt -h
```

Then redo steps 5 and 6. **Never use `patchelf` on these binaries.**

### `No module named 'colorlog'` / `'heatshrink2'` / `'elftools'`

```sh
pip install colorlog heatshrink2 pyelftools
```

### `convert: No such file or directory`

`pillow` is missing: `pip install pillow`, or `pkg install python-pillow`.

### `python3: No such file or directory`

The toolchain's Python symlink is broken or was restored by an update. Re-run step 5.

### `CANNOT LINK EXECUTABLE "python3": library "ld-linux-aarch64.so.1" not found`

`LD_PRELOAD` is pointed at glibc's `libc.so.6`, which breaks Termux's own Bionic binaries. Clear it:

```sh
unset LD_PRELOAD
unset -f ufbt          # in case a shell function is shadowing ufbt
```

Then check your shell startup files for leftovers:

```sh
grep -r "glibc\|LD_PRELOAD" ~/.bashrc ~/.profile ~/.bash_profile ~/.zshrc 2>/dev/null
```

### `ufbt update` says the SDK is up to date, but the toolchain is missing

ufbt tracks the SDK and the toolchain separately. Force both:

```sh
rm -rf ~/.ufbt/current ~/.ufbt/toolchain
ufbt update --index-url=https://up.unleashedflip.com/directory.json --channel=dev
ufbt -h
```

Then redo steps 5 and 6.

### The app builds but reports the wrong API version

An `ufbt update` was run without `--index-url`, so ufbt switched back to the official SDK. Re-run the
command from [step 4](#4-point-ufbt-at-the-unleashed-sdk).

### The wrappers disappeared after an update

Expected — `ufbt update` can replace the whole toolchain directory. Re-run steps 5 and 6.

## What actually happens

Each wrapper invokes its real binary through glibc's dynamic linker with an explicit `--library-path`
pointing at glibc's libraries. When `arm-none-eabi-gcc` execs `cc1`, it hits a shell script that chains
through the linker to the untouched `.real` binary. The cross-compiled output is unaffected — it targets
bare-metal Cortex-M, not Linux.

```
ufbt (Python, Bionic)
  └─ arm-none-eabi-gcc (wrapper)
       └─ ld-linux-aarch64.so.1 → arm-none-eabi-gcc.real (glibc)
            ├─ cc1 (wrapper)      → ld-linux-aarch64.so.1 → cc1.real
            ├─ as (wrapper)       → ld-linux-aarch64.so.1 → as.real
            └─ collect2 → ld (wrapper) → ld-linux-aarch64.so.1 → ld.real
                 └─ output: some_app.fap (ARM Cortex-M4, for the Flipper Zero)
```

## Credits

Worked out and written up by [@CamsShaft](https://github.com/CamsShaft) through extensive trial and error
on a Samsung Galaxy S22 running Termux — no PC, no proot, no virtual machines. Original guide and its MIT
license: [BUILD-FLIPPER-FAPS-IN-TERMUX](https://github.com/CamsShaft/BUILD-FLIPPER-FAPS-IN-TERMUX).
