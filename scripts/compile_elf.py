#!/usr/bin/env python3

import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path

try:
    import fcntl
except ImportError:
    fcntl = None

# Keep sibling helper imports stable when the script is launched from outside
# the repository root.
sys.path.insert(0, str(Path(__file__).resolve().parent))

from archtest_common import (
    DEFAULT_TARGET_PROFILE,
    REPO_ROOT,
    display_path,
    load_artifact_map,
    repo_rel,
    save_artifact_map,
    target_default_platform,
)

REGISTER_FILE = REPO_ROOT / "test_register.c"
TMP_ROOT = REPO_ROOT / ".tmp" / "hyptest_compile"
TMP_REGISTER_DIR = TMP_ROOT / "register"
TMP_DIR = TMP_ROOT / "tmp"
LOCK_DIR = TMP_ROOT / "locks"
OUT_ROOT = REPO_ROOT / "case_elf_asm"
DEFAULT_CROSS_COMPILE = "riscv64-unknown-elf-"
DEFAULT_JOBS = max(1, min(os.cpu_count() or 1, 16))
MAKE_OUTPUT_TAIL_LINES = 80
MAX_LIST_LINES = 120
PROGRESS_BAR_WIDTH = 32
PROGRESS_LOG_INTERVAL_SEC = 5.0
PROGRESS_LOG_STEP = 50
ALIAS_HASH_LEN = 16

DEFAULT_CASE_ROOTS = (
    "test_cases/manual_test_cases",
    "test_cases/ai_test_cases",
    "manual_test_cases",
    "ai_test_cases",
)

REGISTER_RE = re.compile(
    r"^(?P<indent>\s*)(?://\s*)?TEST_REGISTER\((?P<name>[A-Za-z_][A-Za-z0-9_]*)\)\s*;?\s*(?://.*)?$"
)
CASE_DEF_RE = re.compile(r"^\s*bool\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(")
CASE_REF_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")
ASM_GLOBAL_RE = re.compile(r"^\s*\.global\s+([A-Za-z_.$][A-Za-z0-9_.$]*)\b")
ASM_SYMBOL_RE = re.compile(r"\b([A-Za-z_.$][A-Za-z0-9_.$]*)\b")
C_BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
C_LINE_COMMENT_RE = re.compile(r"//.*?$", re.MULTILINE)
C_STRING_OR_CHAR_RE = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')


@dataclass(frozen=True)
class RegisteredTest:
    line: int
    name: str
    active: bool
    discovered: bool = False


@dataclass(frozen=True)
class CaseSourcePlan:
    c_sources: tuple
    asm_sources: tuple


@dataclass(frozen=True)
class BuildFailure:
    case_name: str
    reason: str
    command: tuple = ()
    output: str = ""


class BuildLock:
    def __init__(self, platform):
        self.path = LOCK_DIR / f"{platform}.lock"
        self.handle = None

    def __enter__(self):
        LOCK_DIR.mkdir(parents=True, exist_ok=True)
        self.handle = self.path.open("w", encoding="utf-8")
        if fcntl is not None:
            fcntl.flock(self.handle, fcntl.LOCK_EX)
        self.handle.write(f"pid={os.getpid()}\n")
        self.handle.flush()
        return self

    def __exit__(self, exc_type, exc, tb):
        if self.handle is not None:
            if fcntl is not None:
                fcntl.flock(self.handle, fcntl.LOCK_UN)
            self.handle.close()
        return False


def repo_path(path_text):
    path = Path(os.path.expandvars(str(path_text))).expanduser()
    if path.is_absolute():
        return path
    return REPO_ROOT / path


def existing_case_roots(extra_roots=()):
    roots = []
    seen = set()
    for root_name in tuple(extra_roots or ()) + DEFAULT_CASE_ROOTS:
        root = repo_path(root_name)
        key = root.resolve() if root.exists() else root
        if key in seen or not root.is_dir():
            continue
        seen.add(key)
        roots.append(root)
    return roots


def parse_registers(include_commented=False):
    tests = []
    for line_no, line in enumerate(REGISTER_FILE.read_text().splitlines(), 1):
        match = REGISTER_RE.match(line)
        if not match:
            continue
        active = not line.lstrip().startswith("//")
        if active or include_commented:
            tests.append(RegisteredTest(line_no, match.group("name"), active))
    return tests


def strip_line_comments(line):
    return line.split("//", 1)[0].strip()


def is_case_definition(lines, start_index):
    paren_depth = 0
    saw_open = False
    for line in lines[start_index:]:
        code = strip_line_comments(line)
        if not code:
            continue
        for char in code:
            if char == "(":
                paren_depth += 1
                saw_open = True
            elif char == ")" and paren_depth > 0:
                paren_depth -= 1
            elif saw_open and paren_depth == 0:
                if char == "{":
                    return True
                if char == ";":
                    return False
        if saw_open and paren_depth == 0:
            if "{" in code:
                return True
            if ";" in code:
                return False
    return False


def build_case_source_index(case_roots):
    case_to_source = {}
    duplicates = {}
    for root in case_roots:
        for path in sorted(root.rglob("*.c")):
            try:
                lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
            except OSError:
                continue
            for line_no, line in enumerate(lines, 1):
                match = CASE_DEF_RE.match(line)
                if not match or not is_case_definition(lines, line_no - 1):
                    continue
                case_name = match.group(1)
                location = (path, line_no)
                if case_name in case_to_source:
                    duplicates.setdefault(case_name, [case_to_source[case_name]]).append(location)
                else:
                    case_to_source[case_name] = location
    return case_to_source, duplicates


def build_asm_symbol_index(case_roots):
    symbol_to_source = {}
    duplicates = {}
    for root in case_roots:
        for path in sorted(root.rglob("*.S")):
            try:
                lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
            except OSError:
                continue
            for line_no, line in enumerate(lines, 1):
                match = ASM_GLOBAL_RE.match(line)
                if not match:
                    continue
                symbol = match.group(1)
                location = (path, line_no)
                if symbol in symbol_to_source:
                    duplicates.setdefault(symbol, [symbol_to_source[symbol]]).append(location)
                else:
                    symbol_to_source[symbol] = location
    return symbol_to_source, duplicates


def discover_all_cases(case_to_source):
    return [
        RegisteredTest(0, name, active=False, discovered=True)
        for name in sorted(case_to_source)
    ]


def strip_c_non_code(text):
    text = C_BLOCK_COMMENT_RE.sub(" ", text)
    text = C_LINE_COMMENT_RE.sub(" ", text)
    text = C_STRING_OR_CHAR_RE.sub('""', text)
    return text


def find_indexed_references(path, token_re, source_index, duplicates, cache):
    if path in cache:
        return cache[path]
    try:
        text = path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        cache[path] = frozenset()
        return cache[path]
    refs = set()
    for match in token_re.finditer(strip_c_non_code(text)):
        name = match.group(1)
        if name in source_index and name not in duplicates:
            refs.add(name)
    cache[path] = frozenset(refs)
    return cache[path]


def expand_case_source_dependencies(source_paths, case_to_source, duplicates, cache, dep_cache):
    key = tuple(sorted(source_paths))
    if key in dep_cache:
        return dep_cache[key]
    resolved = set(source_paths)
    pending = list(source_paths)
    while pending:
        path = pending.pop()
        for name in find_indexed_references(path, CASE_REF_RE, case_to_source, duplicates, cache):
            dep_path = case_to_source[name][0]
            if dep_path in resolved:
                continue
            resolved.add(dep_path)
            pending.append(dep_path)
    result = tuple(sorted(resolved, key=display_path))
    dep_cache[key] = result
    return result


def same_directory_asm_sources(case_sources):
    asm_sources = set()
    for path in case_sources:
        asm_sources.update(path.parent.glob("*.S"))
    return asm_sources


def resolve_case_asm_sources(case_sources, symbol_to_source, duplicates, cache):
    asm_sources = same_directory_asm_sources(case_sources)
    for path in case_sources:
        for symbol in find_indexed_references(path, ASM_SYMBOL_RE, symbol_to_source, duplicates, cache):
            asm_sources.add(symbol_to_source[symbol][0])
    return tuple(sorted(asm_sources, key=display_path))


def format_locations(locations):
    return ", ".join(f"{display_path(path)}:{line}" for path, line in locations)


def resolve_case_plan(test, case_to_source, case_duplicates, symbol_to_source, asm_duplicates, caches):
    if test.name in case_duplicates:
        return None, "case function is defined more than once: " + format_locations(case_duplicates[test.name])

    c_info = case_to_source.get(test.name)
    if c_info is None:
        asm_info = symbol_to_source.get(test.name)
        if asm_info is None:
            return None, "cannot find bool case_name(...) source or matching asm global symbol"
        return CaseSourcePlan(c_sources=(), asm_sources=(asm_info[0],)), ""

    c_sources = expand_case_source_dependencies(
        (c_info[0],),
        case_to_source,
        case_duplicates,
        caches["case_refs"],
        caches["case_deps"],
    )
    asm_sources = resolve_case_asm_sources(
        c_sources,
        symbol_to_source,
        asm_duplicates,
        caches["asm_refs"],
    )
    return CaseSourcePlan(c_sources=c_sources, asm_sources=asm_sources), ""


def build_source_plan(tests, case_to_source, case_duplicates, symbol_to_source, asm_duplicates):
    caches = {"case_refs": {}, "case_deps": {}, "asm_refs": {}}
    plans = {}
    errors = {}
    for test in tests:
        if test.name in plans or test.name in errors:
            continue
        plan, error = resolve_case_plan(
            test,
            case_to_source,
            case_duplicates,
            symbol_to_source,
            asm_duplicates,
            caches,
        )
        if error:
            errors[test.name] = error
        else:
            plans[test.name] = plan
    return plans, errors


def normalize_case_name(token):
    name = token.strip()
    if not name:
        return ""
    name = name.split("#", 1)[0].split("//", 1)[0].strip()
    if not name:
        return ""
    name = Path(name).name
    if name.endswith(".ELF"):
        name = name[:-4]
    return name


def read_name_files(paths):
    names = []
    for item in paths or []:
        path = repo_path(item)
        try:
            lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
        except FileNotFoundError as err:
            raise SystemExit(f"missing name file: {display_path(path)}") from err
        for line in lines:
            name = normalize_case_name(line)
            if name:
                names.append(name)
    return names


def compile_patterns(patterns, flag):
    compiled = []
    for pattern in patterns or []:
        try:
            compiled.append(re.compile(pattern))
        except re.error as err:
            raise SystemExit(f"invalid {flag} regexp {pattern!r}: {err}") from err
    return compiled


def parse_ranges(range_args, positional):
    ranges = []
    for item in range_args or []:
        for chunk in item.split(","):
            chunk = chunk.strip()
            if not chunk:
                continue
            if "-" in chunk:
                start, end = chunk.split("-", 1)
                ranges.append((int(start), int(end)))
            else:
                value = int(chunk)
                ranges.append((value, value))

    if positional and positional[0] not in ("all",):
        if len(positional) > 2:
            raise SystemExit("line range mode accepts at most two positional numbers")
        try:
            start = int(positional[0])
            end = int(positional[1]) if len(positional) == 2 else start
        except ValueError as err:
            raise SystemExit("positional range arguments must be integers or 'all'") from err
        ranges.append((start, end))
    return ranges


def select_cases(tests, args):
    visible = list(tests)
    if not args.discover_all and not args.include_commented:
        visible = [test for test in visible if test.active]

    exact_names = set(args.name or [])
    exact_names.update(read_name_files(args.name_file))
    include_patterns = compile_patterns(args.match, "--match")
    exclude_patterns = compile_patterns(args.exclude, "--exclude")
    ranges = parse_ranges(args.range, args.targets)
    positional_all = bool(args.targets and args.targets[0] == "all")

    selected = []
    for test in visible:
        chosen = False
        if positional_all:
            chosen = True
        if args.all_active and test.active:
            chosen = True
        if args.all_discovered and test.discovered:
            chosen = True
        if exact_names and test.name in exact_names:
            chosen = True
        if include_patterns and any(pattern.search(test.name) for pattern in include_patterns):
            chosen = True
        if ranges and test.line and any(start <= test.line <= end for start, end in ranges):
            chosen = True

        if not chosen:
            continue
        if exclude_patterns and any(pattern.search(test.name) for pattern in exclude_patterns):
            continue
        selected.append(test)

    missing = exact_names - {test.name for test in visible}
    if missing:
        raise SystemExit("unknown case(s): " + ", ".join(sorted(missing)))

    deduped = []
    seen = set()
    for test in selected:
        if test.name in seen:
            continue
        seen.add(test.name)
        deduped.append(test)
    return deduped


def print_case_list(tests, case_to_source, symbol_to_source):
    for test in tests:
        c_src = case_to_source.get(test.name, ("", 0))[0]
        asm_src = symbol_to_source.get(test.name, ("", 0))[0]
        if test.active:
            state = "active"
        elif test.discovered:
            state = "discovered"
        else:
            state = "commented"
        source = display_path(c_src) if c_src else display_path(asm_src) if asm_src else "missing-source"
        print(f"{test.line:4d} {state:10s} {test.name}: {source}")


def get_component_name_max(path):
    try:
        value = os.pathconf(path, "PC_NAME_MAX")
        if isinstance(value, int) and value > 0:
            return value
    except (AttributeError, OSError, ValueError):
        pass
    return 255


def artifact_stem(case_name, output_dir, artifact_map):
    existing = artifact_map.get(case_name)
    if existing:
        return existing
    max_stem_len = max(1, get_component_name_max(output_dir) - len(".ELF"))
    if len(case_name) <= max_stem_len:
        return case_name
    digest = hashlib.sha1(case_name.encode("utf-8")).hexdigest()[:ALIAS_HASH_LEN]
    suffix = f"__{digest}"
    prefix_len = max(1, max_stem_len - len(suffix))
    return f"{case_name[:prefix_len]}{suffix}"


def register_stem(case_name):
    TMP_REGISTER_DIR.mkdir(parents=True, exist_ok=True)
    max_stem_len = max(1, get_component_name_max(TMP_REGISTER_DIR) - len(".c"))
    if len(case_name) <= max_stem_len:
        return case_name
    digest = hashlib.sha1(case_name.encode("utf-8")).hexdigest()[:ALIAS_HASH_LEN]
    suffix = f"__{digest}"
    prefix_len = max(1, max_stem_len - len(suffix))
    return f"{case_name[:prefix_len]}{suffix}"


def write_register(case_name):
    out = TMP_REGISTER_DIR / f"register_{register_stem(case_name)}.c"
    out.write_text(
        "#include <rvh_test.h>\n\n"
        "/* Generated by compile_elf.py. Do not edit. */\n"
        f"TEST_REGISTER({case_name});\n",
        encoding="utf-8",
    )
    return out


def plan_requires_dynamic_page_table(plan):
    for path in plan.c_sources:
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        if "dynamic_page_tables.h" in text:
            return True
    return False


def case_page_table_backend(plan, requested_backend):
    if requested_backend is not None:
        return requested_backend
    return "dynamic" if plan_requires_dynamic_page_table(plan) else "static"


def make_env(args):
    tmpdir = repo_path(args.tmpdir)
    tmpdir.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env["TMPDIR"] = str(tmpdir.resolve())
    env["CROSS_COMPILE"] = args.cross_compile or env.get("HYPTEST_CROSS_COMPILE") or env.get("CROSS_COMPILE") or DEFAULT_CROSS_COMPILE
    return env


def remove_link_outputs(platform):
    target = REPO_ROOT / "build" / platform / "rvh_test"
    for suffix in (".elf", ".asm", ".bin", ".elf.txt"):
        target.with_suffix(suffix).unlink(missing_ok=True)


def tail_text(text, max_lines=MAKE_OUTPUT_TAIL_LINES):
    if not text:
        return ""
    lines = text.splitlines()
    if len(lines) > max_lines:
        lines = [f"... ({len(lines) - max_lines} lines omitted)"] + lines[-max_lines:]
    return "\n".join(lines)


def run_silent(cmd, env):
    return subprocess.run(
        cmd,
        cwd=REPO_ROOT,
        env=env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )


def run_make(case_name, plan, backend, args, clean=False):
    register_src = write_register(case_name)
    env = make_env(args)

    if clean:
        clean_cmd = ["make", f"PLAT={args.plat}", "clean"]
        result = run_silent(clean_cmd, env)
        if result.returncode != 0:
            return BuildFailure(case_name, "make clean failed", tuple(clean_cmd), tail_text(result.stdout))

    remove_link_outputs(args.plat)
    cmd = [
        "make",
        f"-j{max(1, args.jobs)}",
        f"PLAT={args.plat}",
        f"TARGET_PROFILE={args.target_profile}",
        f"TEST_REGISTER_SRC={repo_rel(register_src)}",
        "CASE_LINK_MODE=selected",
        "CASE_C_SRCS=" + " ".join(repo_rel(path) for path in plan.c_sources),
        "CASE_ASM_SRCS=" + " ".join(repo_rel(path) for path in plan.asm_sources),
        f"PAGE_TABLE_BACKEND={backend}",
        f"GENERATE_BIN={1 if args.keep_bin else 0}",
        f"GENERATE_ASM={0 if args.skip_asm else 1}",
        "GENERATE_DUMP=0",
        f"GENERATE_READELF={1 if args.keep_readelf else 0}",
        f"SAVE_PREPROCESSED={1 if args.keep_preprocessed else 0}",
    ]
    if args.log_level:
        cmd.append(f"LOG_LEVEL={args.log_level}")
    if args.make_arg:
        cmd.extend(args.make_arg)

    result = run_silent(cmd, env)
    if result.returncode != 0:
        return BuildFailure(case_name, "make failed", tuple(cmd), tail_text(result.stdout))
    return None


def export_case(case_name, args):
    build = REPO_ROOT / "build" / args.plat
    out_dir = OUT_ROOT / args.plat
    out_dir.mkdir(parents=True, exist_ok=True)
    artifacts = load_artifact_map(out_dir)
    stem = artifact_stem(case_name, out_dir, artifacts)

    elf_out = out_dir / f"{stem}.ELF"
    asm_out = out_dir / f"{stem}.asm"
    shutil.copy2(build / "rvh_test.elf", elf_out)
    if not args.skip_asm and (build / "rvh_test.asm").exists():
        shutil.copy2(build / "rvh_test.asm", asm_out)

    artifacts[case_name] = stem
    save_artifact_map(out_dir, args.plat, artifacts)
    return elf_out


class ProgressReporter:
    def __init__(self, total):
        self.total = max(1, total)
        self.enabled = total > 1
        self.is_tty = sys.stdout.isatty()
        self.last_log_time = time.monotonic()

    def update(self, done, ok_count, fail_count, current_name=""):
        if not self.enabled:
            return
        percent = done / self.total
        filled = min(PROGRESS_BAR_WIDTH, int(PROGRESS_BAR_WIDTH * percent))
        bar = "#" * filled + "-" * (PROGRESS_BAR_WIDTH - filled)
        current = f" current={current_name}" if current_name else ""
        msg = f"[{bar}] {done}/{self.total} ({percent * 100:5.1f}%) ok={ok_count} fail={fail_count}{current}"
        if self.is_tty:
            width = shutil.get_terminal_size((120, 20)).columns
            sys.stdout.write("\r\033[K" + msg[: max(1, width - 1)])
            sys.stdout.flush()
            return
        now = time.monotonic()
        if done == 1 or done == self.total or done % PROGRESS_LOG_STEP == 0 or now - self.last_log_time >= PROGRESS_LOG_INTERVAL_SEC:
            print(msg)
            self.last_log_time = now

    def finish(self):
        if self.enabled and self.is_tty:
            sys.stdout.write("\n")
            sys.stdout.flush()


def print_selected_tests(tests):
    print(f"selected cases: {len(tests)}")
    visible = tests
    tail = []
    if len(tests) > MAX_LIST_LINES:
        visible = tests[: MAX_LIST_LINES - 20]
        tail = tests[-20:]
    for test in visible:
        prefix = "" if test.active else "// " if not test.discovered else ""
        print(f"  {test.line}: {prefix}TEST_REGISTER({test.name});")
    if tail:
        print(f"  ... {len(tests) - MAX_LIST_LINES} cases omitted ...")
        for test in tail:
            print(f"  {test.line}: TEST_REGISTER({test.name});")


def print_source_plan(tests, plans, errors, args):
    ok_tests = [test for test in tests if test.name in plans]
    print(f"source plan: located={len(ok_tests)} errors={len(errors)}")
    if not args.plan_only and len(tests) > 10:
        return
    visible = tests if len(tests) <= MAX_LIST_LINES else tests[: MAX_LIST_LINES - 20]
    for test in visible:
        plan = plans.get(test.name)
        if plan is None:
            print(f"- {test.name}: ERROR: {errors.get(test.name, 'cannot locate sources')}")
            continue
        backend = case_page_table_backend(plan, args.page_table_backend)
        c_srcs = ", ".join(display_path(path) for path in plan.c_sources) or "-"
        asm_srcs = ", ".join(display_path(path) for path in plan.asm_sources) or "-"
        print(f"- {test.name}: PAGE_TABLE_BACKEND={backend}")
        print(f"  C: {c_srcs}")
        print(f"  ASM: {asm_srcs}")


def build_parser():
    parser = argparse.ArgumentParser(description="Compile selected arch-test cases without editing test_register.c")
    parser.add_argument("targets", nargs="*", help="optional hyper-compatible selector: all, line, or start end")
    parser.add_argument("--plat")
    parser.add_argument("--target-profile", default=DEFAULT_TARGET_PROFILE)
    parser.add_argument("--cross-compile")
    parser.add_argument("--tmpdir", default=os.environ.get("HYPTEST_TMPDIR") or os.environ.get("TMPDIR") or str(TMP_DIR))
    parser.add_argument("-j", "--jobs", type=int, default=DEFAULT_JOBS)
    parser.add_argument("--clean", action="store_true", help="run make clean once before the first selected case")
    parser.add_argument("--include-commented", action="store_true")
    parser.add_argument("--name", action="append", default=[])
    parser.add_argument("--name-file", action="append", default=[])
    parser.add_argument("--match", action="append", default=[])
    parser.add_argument("--exclude", action="append", default=[])
    parser.add_argument("--range", action="append")
    parser.add_argument("--all-active", action="store_true")
    parser.add_argument("--all-discovered", action="store_true")
    parser.add_argument("--discover-all", action="store_true", help="list/select all discovered case functions instead of test_register.c")
    parser.add_argument("--case-root", action="append", default=[], help="extra case root searched before default roots")
    parser.add_argument("--page-table-backend", choices=("static", "dynamic"))
    parser.add_argument("--list-cases", action="store_true")
    parser.add_argument("--plan-only", action="store_true")
    parser.add_argument("--log-level")
    parser.add_argument("--keep-bin", action="store_true")
    parser.add_argument("--keep-readelf", action="store_true")
    parser.add_argument("--keep-preprocessed", action="store_true")
    parser.add_argument("--skip-asm", action="store_true")
    parser.add_argument("--make-arg", action="append", help="extra VAR=value argument passed to make")
    return parser


def main():
    args = build_parser().parse_args()
    args.plat = args.plat or target_default_platform(args.target_profile)
    args.cross_compile = (
        args.cross_compile
        or os.environ.get("HYPTEST_CROSS_COMPILE")
        or os.environ.get("CROSS_COMPILE")
        or DEFAULT_CROSS_COMPILE
    )

    case_roots = existing_case_roots(args.case_root)
    case_to_source, case_duplicates = build_case_source_index(case_roots)
    symbol_to_source, asm_duplicates = build_asm_symbol_index(case_roots)
    tests = discover_all_cases(case_to_source) if args.discover_all else parse_registers(args.include_commented)

    if args.list_cases:
        visible = tests if args.discover_all or args.include_commented else [test for test in tests if test.active]
        print_case_list(visible, case_to_source, symbol_to_source)
        return 0

    selected = select_cases(tests, args)
    if not selected:
        raise SystemExit("no case selected; use --name, --match, --range, all, --all-active, or --all-discovered")

    print(f"target profile: {args.target_profile}")
    print(f"platform: {args.plat}")
    print(f"cross compile: {args.cross_compile}")
    print(f"jobs: -j{max(1, args.jobs)}")
    print(f"case roots: {', '.join(display_path(path) for path in case_roots)}")
    print_selected_tests(selected)

    plans, errors = build_source_plan(selected, case_to_source, case_duplicates, symbol_to_source, asm_duplicates)
    print_source_plan(selected, plans, errors, args)
    if args.plan_only:
        return 0 if not errors else 2

    OUT_ROOT.mkdir(exist_ok=True)
    ok_count = 0
    fail_count = 0
    failures = []
    progress = ProgressReporter(len(selected))
    clean_pending = args.clean

    with BuildLock(args.plat):
        for index, test in enumerate(selected, 1):
            plan = plans.get(test.name)
            if plan is None:
                fail_count += 1
                failures.append(BuildFailure(test.name, errors.get(test.name, "cannot locate sources")))
                progress.update(index, ok_count, fail_count, test.name)
                continue

            backend = case_page_table_backend(plan, args.page_table_backend)
            failure = run_make(test.name, plan, backend, args, clean=clean_pending)
            clean_pending = False
            if failure is None:
                try:
                    elf = export_case(test.name, args)
                    ok_count += 1
                    if len(selected) == 1:
                        print(f"{test.name}: {display_path(elf)}")
                except Exception as err:
                    fail_count += 1
                    failures.append(BuildFailure(test.name, f"export failed: {err}"))
            else:
                fail_count += 1
                failures.append(failure)
            progress.update(index, ok_count, fail_count, test.name)

    progress.finish()
    print(f"compile done: ok={ok_count} fail={fail_count}")
    if failures:
        print("failures:")
        for failure in failures:
            print(f"- {failure.case_name}: {failure.reason}")
            if failure.command:
                print("  command: " + " ".join(failure.command))
            if failure.output:
                print("  output:")
                for line in failure.output.splitlines():
                    print("    " + line)
    print(f"output dir: {display_path(OUT_ROOT / args.plat)}/")
    return 0 if fail_count == 0 else 2


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except BrokenPipeError:
        sys.stdout = open(os.devnull, "w")
        raise SystemExit(0)
