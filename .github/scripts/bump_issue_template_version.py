#!/usr/bin/env python3
"""Set the bug-report template's "Found in version" dropdown to [<release>, dev].

Called by .github/workflows/bump-issue-template-version.yml when a stable
release is published. Does a surgical text edit (minimal one-line diff);
the result is validated in the workflow with yq.
"""

import re
import sys
from pathlib import Path

TEMPLATE = Path(".github/ISSUE_TEMPLATE/01_bug_report.yml")

# Matches the fw-version dropdown's options block and captures the stable
# entry (the line before "- dev") so only that one line gets rewritten:
#   options:
#     - <stable>   <- replaced with the new release tag
#     - dev
OPTIONS_RE = re.compile(r"(^\s*options:\n\s*-\s*)[^\n]+(\n\s*-\s*dev\s*$)", re.MULTILINE)


def main() -> int:
    if len(sys.argv) != 2 or not sys.argv[1].strip():
        print("usage: bump_issue_template_version.py <release-tag>", file=sys.stderr)
        return 2
    tag = sys.argv[1].strip()

    text = TEMPLATE.read_text(encoding="utf-8")
    new, n = OPTIONS_RE.subn(lambda m: m.group(1) + tag + m.group(2), text)
    if n != 1:
        print(
            f"::error::expected exactly 1 'options' block ending in 'dev', found {n}",
            file=sys.stderr,
        )
        return 1

    if new != text:
        TEMPLATE.write_text(new, encoding="utf-8")
    print(f"Set Found-in-version stable option to {tag}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
