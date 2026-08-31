"""Repository paths shared by the King's Field command-line tools."""

from __future__ import annotations

from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
BUILD = REPO / "build"
CONFIG = REPO / "config"
RETAIL_CONFIG = CONFIG / "retail"
UNITS_MANIFEST = CONFIG / "units.toml"
BASELINE = CONFIG / "match_baseline.tsv"
LOCAL_CONFIG = BUILD / "local.toml"
NINJA = BUILD / "build.ninja"
TOOLCHAIN_ID = BUILD / "gen" / "toolchain.id"
