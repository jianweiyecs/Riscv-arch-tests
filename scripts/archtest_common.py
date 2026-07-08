import json
import re
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
ARTIFACT_MAP = "artifact_name_map.json"
DEFAULT_PLATFORM = "spike"
DEFAULT_TARGET_PROFILE = "generic"


def display_path(path):
    path = Path(path)
    try:
        return path.resolve().relative_to(REPO_ROOT).as_posix()
    except (OSError, ValueError):
        return str(path)


def repo_rel(path):
    return Path(path).resolve().relative_to(REPO_ROOT).as_posix()


def target_mk_path(target_profile):
    return REPO_ROOT / "targets" / target_profile / "target.mk"


def parse_make_default(path, variable):
    pattern = re.compile(
        rf"^\s*{re.escape(variable)}\s*(?:\?=|:=|=)\s*([A-Za-z0-9_./+-]+)\s*$"
    )
    for line in Path(path).read_text(errors="ignore").splitlines():
        match = pattern.match(line)
        if match:
            return match.group(1)
    return ""


def target_default_platform(target_profile, default=DEFAULT_PLATFORM):
    if not target_profile or target_profile == DEFAULT_TARGET_PROFILE:
        return default

    path = target_mk_path(target_profile)
    if not path.exists():
        raise SystemExit(f"unknown target profile: {target_profile}")

    return parse_make_default(path, "PLAT") or default


def load_artifact_map(elf_dir):
    path = Path(elf_dir) / ARTIFACT_MAP
    try:
        data = json.loads(path.read_text())
    except (FileNotFoundError, json.JSONDecodeError):
        return {}
    artifacts = data.get("artifacts", {})
    return artifacts if isinstance(artifacts, dict) else {}


def save_artifact_map(elf_dir, platform, artifacts):
    path = Path(elf_dir) / ARTIFACT_MAP
    payload = {
        "version": 1,
        "platform": platform,
        "artifacts": artifacts,
    }
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
