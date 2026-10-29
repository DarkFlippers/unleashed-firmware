"""Build and run the T5577 host tests.

    python applications/system/t5577/test/run.py

What the suites cover, why they are on the host rather than in unit_tests, and what is in
stub/: see README.md beside this file.
"""
import pathlib
import subprocess
import sys

CLANG = r"C:\Program Files\LLVM\bin\clang.exe"

HERE = pathlib.Path(__file__).resolve().parent
APP = HERE.parent
ROOT = APP.parents[2]
LIB = ROOT / "lib"

# The restore suite runs the real MU_TEST body. It is cut out of the unit test here, on every
# run, rather than kept as a copy: the copy this replaces had fallen two cases behind the
# original - and they were the two the README claimed as its mutation coverage.
MU_SOURCE = ROOT / "applications/debug/unit_tests/tests/lfrfid/lfrfid_protocols.c"
MU_TEST_NAME = "test_lfrfid_t5577_restore_password"
MU_BODY = HERE / "mu_body.generated.inc"


def extract_mu_body():
    """Write the named MU_TEST, verbatim, where the restore suite includes it from."""
    lines = MU_SOURCE.read_text(encoding="utf-8").splitlines()
    opener = "MU_TEST(%s) {" % MU_TEST_NAME

    start = next((i for i, line in enumerate(lines) if line.startswith(opener)), None)
    if start is None:
        sys.exit("%s not found in %s" % (MU_TEST_NAME, MU_SOURCE))

    # The closing brace in column 0: every brace inside the body is indented.
    end = next((i for i in range(start + 1, len(lines)) if lines[i] == "}"), None)
    if end is None:
        sys.exit("%s has no closing brace in column 0" % MU_TEST_NAME)

    MU_BODY.write_text("\n".join(lines[start:end + 1]) + "\n", encoding="utf-8")
    return end - start + 1

# Each suite includes the real source rather than linking it, so the include paths are the whole
# build: the stubs first, then wherever the unit under test lives.
SUITES = [
    ("sniff", "t5577_sniff_test.c", [HERE / "stub", APP]),
    ("restore", "t5577_restore_test.c", [HERE / "stub", LIB]),
]


def main():
    if not pathlib.Path(CLANG).exists():
        sys.exit("host clang not found at %s" % CLANG)

    print("%-9s %d lines from %s" % ("extract", extract_mu_body(), MU_SOURCE.name))

    failed = []

    for name, source, includes in SUITES:
        source = HERE / source
        exe = source.with_suffix(".exe")
        cmd = [CLANG, "-std=gnu11", "-O1", "-w"]
        for inc in includes:
            cmd += ["-I", str(inc)]
        cmd += ["-o", str(exe), str(source)]

        build = subprocess.run(cmd, capture_output=True, text=True)
        if build.returncode != 0:
            print("%-9s BUILD FAILED" % name)
            print(build.stderr.strip()[:2000])
            failed.append(name)
            continue

        run = subprocess.run([str(exe)], capture_output=True, text=True)
        sys.stdout.write(run.stdout)
        if run.stderr.strip():
            sys.stderr.write(run.stderr)
        print("%-9s %s" % (name, "PASS" if run.returncode == 0 else "FAIL"))
        if run.returncode != 0:
            failed.append(name)

    if failed:
        sys.exit("failed: " + ", ".join(failed))
    print("all suites passed")


if __name__ == "__main__":
    main()
