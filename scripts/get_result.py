#!/usr/bin/env python3

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

# Keep sibling helper imports stable when the script is launched from outside
# the repository root.
sys.path.insert(0, str(Path(__file__).resolve().parent))

from archtest_common import (
    DEFAULT_TARGET_PROFILE,
    REPO_ROOT,
    display_path,
    load_artifact_map,
    target_default_platform,
)

DEFAULT_ELF_DIRS = {
    "spike": REPO_ROOT / "case_elf_asm" / "spike",
    "linknan": REPO_ROOT / "case_elf_asm" / "linknan",
    "nemu": REPO_ROOT / "case_elf_asm" / "nemu",
}
DEFAULT_LOG_DIR = REPO_ROOT / ".tmp" / "result_log"
DEFAULT_SPIKE_ISA = "RV64gch"


def select_elves(args):
    elf_dir = Path(args.elf_dir) if args.elf_dir else DEFAULT_ELF_DIRS[args.platform]
    if not elf_dir.is_absolute():
        elf_dir = REPO_ROOT / elf_dir
    artifact_map = load_artifact_map(elf_dir)

    selected = []
    if args.all_elves:
        selected = sorted(elf_dir.glob("*.ELF")) + sorted(elf_dir.glob("*.elf"))
        return [(p.stem, p) for p in selected]

    for case in args.case or []:
        stem = artifact_map.get(case, case)
        for suffix in (".ELF", ".elf"):
            path = elf_dir / f"{stem}{suffix}"
            if path.exists():
                selected.append((case, path))
                break
        else:
            raise SystemExit(f"missing ELF for case {case} in {elf_dir}")

    if not selected:
        raise SystemExit("no ELF selected; use --case or --all-elves")
    return selected


def result_from_output(output):
    if re.search(r"\bFAILED\b", output) or "ERROR:" in output or "untested exception" in output:
        return "FAIL"
    if re.search(r"\bPASSED\b", output):
        return "PASS"
    return "UNKNOWN"


def spike_command(args, elf):
    spike = args.spike_bin or os.environ.get("HYPTEST_SPIKE_BIN") or os.environ.get("SPIKE_BIN") or "spike"
    isa = args.isa or DEFAULT_SPIKE_ISA
    return [spike, f"--isa={isa}", str(elf)]


def nemu_command(args, elf):
    nemu = args.nemu_bin or os.environ.get("HYPTEST_NEMU_BIN") or os.environ.get("NEMU_BIN")
    if not nemu:
        raise SystemExit("set --nemu-bin or HYPTEST_NEMU_BIN for nemu runs")
    return [nemu, str(elf)]


def linknan_command(args, case_name, elf):
    linknan_home = args.linknan_home or os.environ.get("HYPTEST_LINKNAN_HOME") or os.environ.get("LINKNAN_HOME")
    if not linknan_home:
        raise SystemExit("set --linknan-home or HYPTEST_LINKNAN_HOME for linknan runs")
    linknan_home = Path(linknan_home).resolve()
    comp_dir = linknan_home / "sim" / "simv" / "comp"
    simv = comp_dir / "simv"
    daidir = comp_dir / "simv.daidir"
    if not simv.exists():
        raise SystemExit(f"missing LinkNan simv: {simv}")
    if not daidir.exists():
        raise SystemExit(f"missing LinkNan simv.daidir: {daidir}")

    run_dir = linknan_home / "sim" / "simv" / case_name
    run_dir.mkdir(parents=True, exist_ok=True)
    local_simv = run_dir / "simv"
    local_daidir = run_dir / "simv.daidir"
    if local_simv.exists() or local_simv.is_symlink():
        local_simv.unlink()
    if local_daidir.exists() or local_daidir.is_symlink():
        if local_daidir.is_dir() and not local_daidir.is_symlink():
            shutil.rmtree(local_daidir)
        else:
            local_daidir.unlink()
    local_simv.symlink_to(simv)
    local_daidir.symlink_to(daidir, target_is_directory=True)
    workload = run_dir / "workload.ELF"
    if workload.exists() or workload.is_symlink():
        workload.unlink()
    workload.symlink_to(elf)

    cmd = ["./simv", f"+max-cycles={args.cycles}", "+workload=workload.ELF"]
    if args.no_diff:
        cmd.append("+no-diff")
    else:
        ref_so = args.diff_ref_so or os.environ.get("HYPTEST_DIFFTEST_REF_SO") or os.environ.get("DIFFTEST_REF_SO")
        if not ref_so:
            raise SystemExit("set --diff-ref-so/HYPTEST_DIFFTEST_REF_SO, or use --no-diff")
        cmd.append(f"+diff={ref_so}")
    if args.fsdb:
        cmd.append("+dump-wave=fsdb")
    if args.simv_arg:
        cmd.extend(args.simv_arg)
    return cmd, run_dir


def run_one(args, case_name, elf):
    log_dir = DEFAULT_LOG_DIR / args.platform
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / f"{case_name}.log"

    if args.platform == "spike":
        cmd = spike_command(args, elf)
        cwd = REPO_ROOT
    elif args.platform == "nemu":
        cmd = nemu_command(args, elf)
        cwd = REPO_ROOT
    else:
        cmd, cwd = linknan_command(args, case_name, elf)

    try:
        proc = subprocess.run(cmd, cwd=cwd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=args.timeout)
        output = proc.stdout
        status = result_from_output(output)
        if proc.returncode != 0 and status == "UNKNOWN":
            status = "FAIL"
    except subprocess.TimeoutExpired as exc:
        output = (exc.stdout or "") + "\nTIMEOUT\n"
        status = "TIMEOUT"

    log_file.write_text("$ " + " ".join(map(str, cmd)) + "\n\n" + output)
    print(f"{status:7} {case_name} -> {display_path(log_file)}")
    return status


def main():
    parser = argparse.ArgumentParser(description="Run arch-test ELFs on Spike, NEMU, or LinkNan")
    parser.add_argument("--platform", choices=("spike", "nemu", "linknan"))
    parser.add_argument("--target-profile", default=DEFAULT_TARGET_PROFILE)
    parser.add_argument("--case", action="append")
    parser.add_argument("--all-elves", action="store_true")
    parser.add_argument("--elf-dir")
    parser.add_argument("--timeout", type=int, default=20)
    parser.add_argument("--spike-bin")
    parser.add_argument("--nemu-bin")
    parser.add_argument("--isa")
    parser.add_argument("--linknan-home")
    parser.add_argument("--diff-ref-so")
    parser.add_argument("--no-diff", action="store_true")
    parser.add_argument("--fsdb", action="store_true")
    parser.add_argument("--cycles", default="1000000")
    parser.add_argument("--simv-arg", action="append")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()
    args.platform = args.platform or target_default_platform(args.target_profile)

    selected = select_elves(args)
    if args.dry_run:
        print(f"platform={args.platform}")
        for case_name, elf in selected:
            print(f"{case_name}: {display_path(elf)}")
        return

    counts = {}
    for case_name, elf in selected:
        status = run_one(args, case_name, elf.resolve())
        counts[status] = counts.get(status, 0) + 1

    print("Summary:", ", ".join(f"{k}={v}" for k, v in sorted(counts.items())))


if __name__ == "__main__":
    main()
