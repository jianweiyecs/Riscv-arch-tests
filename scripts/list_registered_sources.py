#!/usr/bin/env python3

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from archtest_common import REPO_ROOT


REGISTER_RE = re.compile(
    r"^\s*TEST_REGISTER\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)\s*;?\s*$",
    re.MULTILINE,
)
CASE_DEF_TEMPLATE = r"^\s*bool\s+{}\s*\("


def repo_rel(repo_root, path):
    return path.resolve().relative_to(repo_root).as_posix()


def parse_registered_cases(register_file):
    text = register_file.read_text(errors="ignore")
    return REGISTER_RE.findall(text)


def find_case_source(repo_root, case_roots, case_name):
    pattern = re.compile(CASE_DEF_TEMPLATE.format(re.escape(case_name)), re.MULTILINE)
    for root in case_roots:
        root_path = repo_root / root
        if not root_path.is_dir():
            continue
        for src in sorted(root_path.rglob("*.c")):
            if pattern.search(src.read_text(errors="ignore")):
                return repo_rel(repo_root, src)
    return ""


def main():
    parser = argparse.ArgumentParser(description="Print C sources used by TEST_REGISTER entries")
    parser.add_argument("--repo-root", default=str(REPO_ROOT))
    parser.add_argument("--register", default="test_register.c")
    parser.add_argument("--roots", nargs="+", required=True)
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    register_file = repo_root / args.register
    sources = []
    for case_name in parse_registered_cases(register_file):
        source = find_case_source(repo_root, args.roots, case_name)
        if source:
            sources.append(source)
    print(" ".join(dict.fromkeys(sources)))


if __name__ == "__main__":
    main()
