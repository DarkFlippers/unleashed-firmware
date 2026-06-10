#!/usr/bin/env python3
"""Summarize public firmware API changes between two api_symbols.csv revisions.

The public API surface available to external apps (.fap) is defined by
``targets/<hw>/api_symbols.csv``. Only entries with status ``+`` are exported and
callable by apps; ``-`` entries are known-but-disabled (e.g. STM32 HAL symbols).

A removed or signature-changed ``+`` Function/Variable, or a MAJOR version bump,
is *breaking* for prebuilt apps: they fail at load time with an API-version
mismatch (lib/flipper_application/application_manifest.c) or MissingImports.
Additive-only changes bump the MINOR version and keep existing apps compatible.

Emits a Markdown report (``--out``) and prints a JSON status line to stdout:
    {"changed": <bool>, "breaking": <bool>}

Base/head sources:
  * CI mode:   --base-ref <git-ref>  (head = working tree; all targets scanned)
  * Test mode: --base-file / --head-file  (single --target)
"""

import argparse
import csv
import io
import json
import subprocess
from pathlib import Path


def parse_api(text):
    """Parse api_symbols.csv text.

    Returns (version, funcs, variables, headers) where funcs/variables/headers
    only contain status ``+`` (exported) entries:
      funcs:     name -> (return_type, params)
      variables: name -> type
      headers:   set(name)
    """
    version = None
    funcs, variables, headers = {}, {}, set()
    for row in csv.reader(io.StringIO(text)):
        if not row or row[0] == "entry":
            continue
        entry = row[0]
        status = row[1] if len(row) > 1 else ""
        name = row[2] if len(row) > 2 else ""
        rtype = row[3] if len(row) > 3 else ""
        params = row[4] if len(row) > 4 else ""
        if entry == "Version":
            version = name
            continue
        if status != "+":
            continue  # disabled / not exported to apps
        if entry == "Function":
            funcs[name] = (rtype, params)
        elif entry == "Variable":
            variables[name] = rtype
        elif entry == "Header":
            headers.add(name)
    return version, funcs, variables, headers


def _ver_tuple(v):
    try:
        parts = v.split(".")
        return int(parts[0]), (int(parts[1]) if len(parts) > 1 else 0)
    except (ValueError, AttributeError, IndexError):
        return None


def classify_version(old, new):
    o, n = _ver_tuple(old), _ver_tuple(new)
    if not o or not n or o == n:
        return "none"
    if n[0] > o[0]:
        return "major"
    if n[0] == o[0] and n[1] > o[1]:
        return "minor"
    return "other"


def _table(title, names, fmt):
    if not names:
        return []
    out = [f"<details><summary>{title} ({len(names)})</summary>", "",
           "| Symbol | Signature |", "|---|---|"]
    out += [fmt(n) for n in names]
    out += ["", "</details>", ""]
    return out


def diff_target(target, base_text, head_text):
    """Return (markdown_section, changed, breaking) for one target."""
    b_ver, b_func, b_var, b_hdr = parse_api(base_text)
    h_ver, h_func, h_var, h_hdr = parse_api(head_text)

    func_added = sorted(set(h_func) - set(b_func))
    func_removed = sorted(set(b_func) - set(h_func))
    func_changed = sorted(n for n in set(b_func) & set(h_func) if b_func[n] != h_func[n])
    var_added = sorted(set(h_var) - set(b_var))
    var_removed = sorted(set(b_var) - set(h_var))
    var_changed = sorted(n for n in set(b_var) & set(h_var) if b_var[n] != h_var[n])
    hdr_added = sorted(h_hdr - b_hdr)
    hdr_removed = sorted(b_hdr - h_hdr)
    vbump = classify_version(b_ver, h_ver)

    changed = any([b_ver != h_ver, func_added, func_removed, func_changed,
                   var_added, var_removed, var_changed, hdr_added, hdr_removed])
    breaking = bool(func_removed or func_changed or var_removed or var_changed
                    or vbump == "major")
    if not changed:
        return "", False, False

    L = [f"### Target `{target}`"]
    if b_ver != h_ver:
        tag = {"major": " &mdash; ⚠️ **MAJOR (breaking)**",
               "minor": " &mdash; additive",
               "other": "", "none": ""}.get(vbump, "")
        L.append(f"**SDK API version:** `{b_ver}` → `{h_ver}`{tag}")
    else:
        L.append(f"**SDK API version:** `{h_ver}` (unchanged)")
    L.append("")

    if func_removed or var_removed or func_changed or var_changed:
        L.append("#### \U0001f534 Removed / changed (breaking)")
        L += _table("Functions removed", func_removed,
                    lambda n: f"| `{n}` | `{b_func[n][0]} {n}({b_func[n][1]})` |")
        L += _table("Functions changed", func_changed,
                    lambda n: f"| `{n}` | `{b_func[n][0]} ({b_func[n][1]})` → "
                              f"`{h_func[n][0]} ({h_func[n][1]})` |")
        L += _table("Variables removed", var_removed,
                    lambda n: f"| `{n}` | `{b_var[n]}` |")
        L += _table("Variables changed", var_changed,
                    lambda n: f"| `{n}` | `{b_var[n]}` → `{h_var[n]}` |")

    if func_added or var_added:
        L.append("#### \U0001f7e2 Added (non-breaking)")
        L += _table("Functions added", func_added,
                    lambda n: f"| `{n}` | `{h_func[n][0]} {n}({h_func[n][1]})` |")
        L += _table("Variables added", var_added,
                    lambda n: f"| `{n}` | `{h_var[n]}` |")

    if hdr_added or hdr_removed:
        bits = []
        if hdr_added:
            bits.append(f"{len(hdr_added)} added")
        if hdr_removed:
            bits.append(f"{len(hdr_removed)} removed")
        L.append(f"#### \U0001f4c4 Headers: {', '.join(bits)}")
        L += [f"- \U0001f7e2 `{n}`" for n in hdr_added]
        L += [f"- \U0001f534 `{n}`" for n in hdr_removed]
        L.append("")

    if (func_removed or func_changed or var_removed or var_changed) and vbump != "major":
        L.append("> ⚠️ **Note:** symbols were removed/changed but the major "
                 "version was not bumped. fbt normally forces a MAJOR bump on removal "
                 "&mdash; double-check `api_symbols.csv`.")
        L.append("")

    return "\n".join(L), changed, breaking


def git_show(ref, path):
    """Return file content at a git ref, or '' if it does not exist there."""
    if not ref:
        return ""
    try:
        return subprocess.check_output(
            ["git", "show", f"{ref}:{path}"], stderr=subprocess.DEVNULL
        ).decode("utf-8", "replace")
    except subprocess.CalledProcessError:
        return ""


def _read(path):
    p = Path(path)
    return p.read_text(encoding="utf-8", errors="replace") if p.exists() else ""


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--base-ref", help="git ref for the PR base commit")
    ap.add_argument("--out", default="api_report.md")
    ap.add_argument("--base-file", help="test mode: base csv path")
    ap.add_argument("--head-file", help="test mode: head csv path")
    ap.add_argument("--target", default="f7", help="target label for test mode")
    args = ap.parse_args()

    sections, any_changed, any_breaking = [], False, False

    if args.base_file or args.head_file:
        # Test mode: a single explicit base/head pair.
        pairs = [(args.target, _read(args.base_file), _read(args.head_file))]
    else:
        # CI mode: every target, base from git, head from the working tree.
        pairs = [(p.parent.name, git_show(args.base_ref, p.as_posix()), _read(p))
                 for p in sorted(Path("targets").glob("*/api_symbols.csv"))]

    for target, base_text, head_text in pairs:
        section, changed, breaking = diff_target(target, base_text, head_text)
        if section:
            sections.append(section)
        any_changed |= changed
        any_breaking |= breaking

    if any_changed:
        head = "## \U0001f4cb Public API changes\n"
        if any_breaking:
            head += ("\n> \U0001f534 **This PR changes the public app API in a breaking way.** "
                     "Prebuilt community apps (all-the-plugins) built against the old API "
                     "will fail to load until rebuilt.\n")
        else:
            head += "\n> \U0001f7e2 Additive API changes only &mdash; existing apps stay compatible.\n"
        body = head + "\n" + "\n".join(sections)
    else:
        body = "## \U0001f4cb Public API changes\n\n✅ No public API changes in this PR.\n"

    Path(args.out).write_text(body, encoding="utf-8")
    print(json.dumps({"changed": any_changed, "breaking": any_breaking}))


if __name__ == "__main__":
    main()
