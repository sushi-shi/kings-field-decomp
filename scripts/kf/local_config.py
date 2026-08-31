"""Resolve and initialize machine-local retail executable configuration."""

from __future__ import annotations

import json
import os
import tomllib
from pathlib import Path

from scripts.kf.paths import LOCAL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, parse_psx_exe


def validate_retail_dir(path: Path) -> Path:
    path = path.expanduser().resolve()
    for image, expected in IMAGE_LAYOUTS.items():
        executable = path / image
        if not executable.is_file():
            raise ValueError(f"{executable}: missing retail executable")
        if parse_psx_exe(executable) != expected:
            raise ValueError(f"{executable}: retail identity/layout mismatch")
    return path


def configured_retail_dir(cli_path: Path | None = None, *, validate: bool = True) -> Path:
    candidate: Path | None = cli_path
    if candidate is None and os.environ.get("KF_RETAIL_DIR"):
        candidate = Path(os.environ["KF_RETAIL_DIR"])
    if candidate is None and LOCAL_CONFIG.is_file():
        with LOCAL_CONFIG.open("rb") as stream:
            data = tomllib.load(stream)
        value = data.get("retail", {}).get("dir")
        if value:
            candidate = Path(str(value))
    if candidate is None:
        raise ValueError(
            "retail directory is not configured; run `kf init --retail-dir PATH` "
            "or set KF_RETAIL_DIR"
        )
    return validate_retail_dir(candidate) if validate else candidate.expanduser().resolve()


def initialize(path: Path) -> Path:
    retail = validate_retail_dir(path)
    content = "[retail]\n" + f"dir = {json.dumps(str(retail))}\n"
    if LOCAL_CONFIG.is_file() and LOCAL_CONFIG.read_text(encoding="utf-8") == content:
        return retail
    LOCAL_CONFIG.parent.mkdir(parents=True, exist_ok=True)
    temporary = LOCAL_CONFIG.with_name(f".{LOCAL_CONFIG.name}.tmp")
    temporary.write_text(content, encoding="utf-8")
    temporary.replace(LOCAL_CONFIG)
    return retail
