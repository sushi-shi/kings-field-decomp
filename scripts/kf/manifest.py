"""Load and validate the explicit reconstruction-unit manifest."""

from __future__ import annotations

import re
import tomllib
from dataclasses import dataclass
from pathlib import Path

from scripts.kf.delink import Function, load_catalog
from scripts.kf.paths import REPO, RETAIL_CONFIG, UNITS_MANIFEST
from scripts.kf.retail import IMAGE_LAYOUTS


UNIT_RE = re.compile(r"[a-z0-9][a-z0-9_.-]*$")
LANGUAGE_SUFFIXES = {
    "c": {".c"},
    "assembly": {".s", ".asm"},
}


@dataclass(frozen=True)
class Profile:
    name: str
    language: str
    compiler: str
    optimization: str | None
    small_data: int
    aspsx_version: str
    cc1_flags: tuple[str, ...]


@dataclass(frozen=True)
class Unit:
    unit: str
    image: str
    va: int
    source: str
    profile: str
    function: Function

    @property
    def image_key(self) -> str:
        return self.image.removesuffix(".EXE").lower()

    @property
    def object_name(self) -> str:
        return f"{self.va:08x}_{self.function.symbol}.o"

    @property
    def source_path(self) -> Path:
        return REPO / self.source


@dataclass(frozen=True)
class Manifest:
    profiles: dict[str, Profile]
    units: tuple[Unit, ...]

    def by_name(self) -> dict[str, Unit]:
        return {unit.unit: unit for unit in self.units}

    def by_identity(self) -> dict[tuple[str, int], Unit]:
        return {(unit.image, unit.va): unit for unit in self.units}


def _profile(name: str, row: object) -> Profile:
    if not isinstance(row, dict):
        raise ValueError(f"profile {name!r} must be a TOML table")
    allowed = {
        "language", "compiler", "optimization", "small_data",
        "aspsx_version", "cc1_flags",
    }
    extra = set(row) - allowed
    if extra:
        raise ValueError(f"profile {name!r} has unknown keys: {sorted(extra)}")
    language = str(row.get("language", ""))
    if language not in LANGUAGE_SUFFIXES:
        raise ValueError(
            f"profile {name!r}: language must be one of {sorted(LANGUAGE_SUFFIXES)}"
        )
    compiler = str(row.get("compiler", ""))
    if language == "c" and compiler != "gcc260-native":
        raise ValueError(
            f"profile {name!r}: only the explicit gcc260-native probe is supported"
        )
    if language == "assembly" and compiler != "gnu-as":
        raise ValueError(f"profile {name!r}: assembly compiler must be gnu-as")
    optimization = row.get("optimization")
    if language == "c" and optimization not in {"O0", "O1", "O2", "O3"}:
        raise ValueError(f"profile {name!r}: C optimization must be O0..O3")
    if language == "assembly" and optimization is not None:
        raise ValueError(f"profile {name!r}: assembly has no optimization setting")
    small_data = int(row.get("small_data", 0))
    if small_data < 0:
        raise ValueError(f"profile {name!r}: small_data must be non-negative")
    flags = row.get("cc1_flags", [])
    if not isinstance(flags, list) or not all(isinstance(flag, str) for flag in flags):
        raise ValueError(f"profile {name!r}: cc1_flags must be an array of strings")
    return Profile(
        name=name,
        language=language,
        compiler=compiler,
        optimization=str(optimization) if optimization is not None else None,
        small_data=small_data,
        aspsx_version=str(row.get("aspsx_version", "1.07")),
        cc1_flags=tuple(flags),
    )


def load(
    path: Path = UNITS_MANIFEST,
    *,
    config_dir: Path = RETAIL_CONFIG,
    require_sources: bool = True,
) -> Manifest:
    try:
        with path.open("rb") as stream:
            data = tomllib.load(stream)
    except FileNotFoundError:
        raise ValueError(f"{path}: missing reconstruction-unit manifest") from None
    if set(data) - {"profiles", "unit"}:
        raise ValueError(f"{path}: unknown top-level keys: {sorted(set(data) - {'profiles', 'unit'})}")
    raw_profiles = data.get("profiles", {})
    if not isinstance(raw_profiles, dict) or not raw_profiles:
        raise ValueError(f"{path}: [profiles] must define at least one profile")
    profiles = {name: _profile(name, row) for name, row in raw_profiles.items()}
    raw_units = data.get("unit", [])
    if not isinstance(raw_units, list):
        raise ValueError(f"{path}: [[unit]] entries must be an array of tables")

    catalog = load_catalog(config_dir)
    units: list[Unit] = []
    seen_names: set[str] = set()
    seen_identities: set[tuple[str, int]] = set()
    for index, row in enumerate(raw_units, 1):
        if not isinstance(row, dict):
            raise ValueError(f"{path}: unit #{index} is not a table")
        required = {"unit", "image", "va", "source", "profile"}
        missing = required - set(row)
        extra = set(row) - required
        if missing or extra:
            raise ValueError(
                f"{path}: unit #{index} missing={sorted(missing)} unknown={sorted(extra)}"
            )
        name = str(row["unit"])
        if not UNIT_RE.fullmatch(name):
            raise ValueError(f"{path}: invalid unit id {name!r}")
        if name in seen_names:
            raise ValueError(f"{path}: duplicate unit id {name!r}")
        seen_names.add(name)
        image = str(row["image"]).upper()
        if image not in IMAGE_LAYOUTS:
            raise ValueError(f"{path}: unit {name!r} has unknown image {image!r}")
        va = int(row["va"])
        identity = image, va
        if identity in seen_identities:
            raise ValueError(f"{path}: duplicate target {image}:{va:#x}")
        seen_identities.add(identity)
        function = catalog.function_starts[image].get(va)
        if function is None:
            raise ValueError(f"{path}: unit {name!r} target {image}:{va:#x} is not admitted")
        if function.scope == "vendored":
            raise ValueError(f"{path}: unit {name!r} targets vendored function {function.symbol}")
        if function.fragments != 1:
            raise ValueError(f"{path}: unit {name!r} targets a fragmented function")
        profile_name = str(row["profile"])
        if profile_name not in profiles:
            raise ValueError(f"{path}: unit {name!r} references unknown profile {profile_name!r}")
        source = Path(str(row["source"]))
        if source.is_absolute() or ".." in source.parts:
            raise ValueError(f"{path}: unit {name!r} source must be repo-relative")
        source_path = REPO / source
        if require_sources and not source_path.is_file():
            raise ValueError(f"{path}: unit {name!r} source is missing: {source}")
        profile = profiles[profile_name]
        if source.suffix.lower() not in LANGUAGE_SUFFIXES[profile.language]:
            raise ValueError(
                f"{path}: unit {name!r} source suffix does not match {profile.language}"
            )
        units.append(Unit(name, image, va, source.as_posix(), profile_name, function))
    return Manifest(profiles, tuple(units))
