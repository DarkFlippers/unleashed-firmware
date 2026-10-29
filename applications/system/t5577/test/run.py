"""Build and run the T5577 host tests.

    python applications/system/t5577/test/run.py

These run on the PC, not the Flipper. The `unit_tests` app cannot link in this tree (a
pre-existing DWARF error unrelated to anything here), so host clang is how this logic gets
exercised at all - and the decode arithmetic is worth exercising, since its answers are only
checkable against a cloner otherwise.

`stub/` holds just enough of furi, furi_hal and the LL timer headers to compile the two units
away from the firmware. Nothing in it is a mock with behaviour; they exist so the real source
can be included verbatim.
"""
import pathlib
import subprocess
import sys

CLANG = r"C:\Program Files\LLVM\bin\clang.exe"

HERE = pathlib.Path(__file__).resolve().parent
APP = HERE.parent
LIB = APP.parents[2] / "lib"

# Each suite includes the real source rather than linking it, so the include paths are the whole
# build: the stubs first, then wherever the unit under test lives.
SUITES = [
    ("sniff   ", "t5577_sniff_test.c", [HERE / "stub", APP]),
    ("restore ", "t5577_restore_test.c", [HERE / "stub", HERE, LIB]),
]


def main():
    if not pathlib.Path(CLANG).exists():
        sys.exit("host clang not found at %s" % CLANG)

    failed = []

    for name, source, includes in SUITES:
        exe = HERE / (source[:-2] + ".exe")
        cmd = [CLANG, "-std=gnu11", "-O1", "-w"]
        for inc in includes:
            cmd += ["-I", str(inc)]
        cmd += ["-o", str(exe), str(HERE / source)]

        build = subprocess.run(cmd, capture_output=True, text=True)
        if build.returncode != 0:
            print("%s BUILD FAILED" % name)
            print(build.stderr.strip()[:2000])
            failed.append(name.strip())
            continue

        run = subprocess.run([str(exe)], capture_output=True, text=True)
        sys.stdout.write(run.stdout)
        if run.stderr.strip():
            sys.stderr.write(run.stderr)
        print("%s %s" % (name, "PASS" if run.returncode == 0 else "FAIL"))
        if run.returncode != 0:
            failed.append(name.strip())

    if failed:
        sys.exit("failed: " + ", ".join(failed))
    print("all suites passed")


if __name__ == "__main__":
    main()
