"""Load and validate the explicit reconstruction-unit manifest."""

from __future__ import annotations

import re
import tomllib
from dataclasses import dataclass, replace
from pathlib import Path

from scripts.kf.delink import Datum, Function, Module, load_catalog
from scripts.kf.model import (
    Claim,
    DataClaim,
    DataIdentity,
    data_identities,
    identity_names,
    scan_rodata_claims,
    scan_source,
    stale_address_names,
    write_bindings,
)
from scripts.kf.paths import REPO, RETAIL_CONFIG, UNITS_MANIFEST
from scripts.kf.retail import IMAGE_LAYOUTS


DEFINE_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*(=[A-Za-z0-9_x]+)?")
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


# Native probe compilers. Each is a Decompals old-gcc rebuild of a PSX GCC
# target; neither is proven to be the historical King's Field compiler.
C_COMPILERS = ("gcc260-native", "gcc257-native")


@dataclass(frozen=True)
class Unit:
    """One reconstructed translation unit: a source and its claimed functions.

    ``functions`` ascend by address and cover a contiguous retail run, so the
    unit's target object is that run carved as one section. ``va`` is the
    first function's address; the ledger still keys progress per function.
    """

    unit: str
    image: str
    source: str
    profile: str
    functions: tuple[Function, ...]
    data: tuple[Datum, ...] = ()
    rodata: tuple[int, int] | None = None
    defines: tuple[str, ...] = ()
    scope: str = "decomp"

    @property
    def va(self) -> int:
        return self.functions[0].va

    @property
    def function(self) -> Function:
        return self.functions[0]

    @property
    def image_key(self) -> str:
        return self.image.removesuffix(".EXE").lower()

    @property
    def stem(self) -> str:
        prefix = f"{self.image_key}."
        return self.unit[len(prefix):] if self.unit.startswith(prefix) else self.unit

    @property
    def object_name(self) -> str:
        return f"{self.va:08x}_{self.stem}.o"

    @property
    def source_path(self) -> Path:
        return REPO / self.source


@dataclass(frozen=True)
class Manifest:
    profiles: dict[str, Profile]
    units: tuple[Unit, ...]

    def by_name(self) -> dict[str, Unit]:
        return {unit.unit: unit for unit in self.units}

    def modules(self) -> tuple[Module, ...]:
        return tuple(
            Module(
                unit.image,
                unit.unit,
                unit.stem,
                tuple(f.va for f in unit.functions),
                unit.data,
                unit.rodata,
            )
            for unit in self.units
        )

    def by_identity(self) -> dict[tuple[str, int], Unit]:
        return {
            (unit.image, function.va): unit
            for unit in self.units
            for function in unit.functions
        }


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
    if language == "c" and compiler not in C_COMPILERS:
        raise ValueError(
            f"profile {name!r}: C compiler must be one of the explicit probes "
            f"{C_COMPILERS}"
        )
    if language == "assembly" and compiler != "aspsx":
        raise ValueError(f"profile {name!r}: assembly compiler must be aspsx")
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


def _bind_claims(
    path: Path,
    unit: str,
    image: str,
    source: Path,
    claims: tuple[Claim, ...],
    catalog,
    identities: dict[tuple[str, int], str],
    claimed: dict[tuple[str, int], str],
    unit_scope: str = "decomp",
) -> tuple[Function, ...]:
    """Validate one source's claims and return its admitted functions in order."""
    functions: list[Function] = []
    starts = catalog.function_starts[image]
    for claim in claims:
        where = f"{source}:{claim.line}"
        function = starts.get(claim.va)
        if function is None:
            raise ValueError(f"{where}: ADDRESS({claim.va:#x}) is not an admitted {image} function")
        if function.scope != unit_scope:
            if function.scope == "vendored":
                raise ValueError(
                    f"{where}: {function.symbol} is vendored library code; "
                    "the unit must declare scope = \"vendored\""
                )
            raise ValueError(
                f"{where}: vendored verification unit claims non-vendored "
                f"function {function.symbol}"
            )
        if function.fragments != 1:
            raise ValueError(f"{where}: {function.symbol} is fragmented")
        if claim.size != function.body_size:
            raise ValueError(
                f"{where}: ADDRESS({claim.va:#x}, {claim.size:#x}) disagrees with the "
                f"admitted body size {function.body_size:#x} of {function.symbol}"
            )
        expected = identities.get((image, claim.va), function.symbol)
        if claim.name != expected:
            raise ValueError(
                f"{where}: ADDRESS({claim.va:#x}) defines {claim.name!r} but the identity "
                f"inventory names it {expected!r}"
            )
        owner = claimed.get((image, claim.va))
        if owner is not None:
            raise ValueError(f"{where}: {function.symbol} is already claimed by unit {owner!r}")
        if functions and function.va <= functions[-1].va:
            raise ValueError(
                f"{where}: ADDRESS({claim.va:#x}) does not ascend after "
                f"{functions[-1].va:#x}; sources follow the linked order"
            )
        claimed[(image, claim.va)] = unit
        functions.append(function)
    # A unit owns a contiguous run: every admitted function between its first
    # and last claim must be claimed by the same source.
    ordered = [function for function in catalog.functions[image]
               if functions[0].va <= function.va <= functions[-1].va]
    for function in ordered:
        if function.va not in {item.va for item in functions}:
            raise ValueError(
                f"{path}: unit {unit!r} spans {function.symbol} at {function.va:#x} "
                f"without claiming it; split the unit or reconstruct that function"
            )
    for previous, following in zip(functions, functions[1:]):
        if previous.va + previous.size != following.va:
            raise ValueError(
                f"{path}: unit {unit!r}: {previous.symbol} ends at "
                f"{previous.va + previous.size:#x} but {following.symbol} starts at "
                f"{following.va:#x}; the run is not contiguous"
            )
    return tuple(functions)


def _rebind_claims_by_name(
    source: Path,
    image: str,
    claims: tuple[Claim, ...],
    identities: dict[tuple[str, int], str],
) -> tuple[Claim, ...]:
    """Re-address a shared source's ADDRESS() claims through IMAGE's identities."""
    by_name: dict[str, list[int]] = {}
    for (item_image, va), item_name in identities.items():
        if item_image == image:
            by_name.setdefault(item_name, []).append(va)
    rebound: list[Claim] = []
    for claim in claims:
        where = f"{source}:{claim.line}"
        candidates = by_name.get(claim.name, [])
        if len(candidates) != 1:
            raise ValueError(
                f"{where}: {claim.name!r} is not a unique labelled {image} function "
                f"({len(candidates)} identities); name binding needs one"
            )
        rebound.append(replace(claim, va=candidates[0]))
    return tuple(rebound)


def _rebind_data_claims_by_name(
    source: Path,
    image: str,
    claims: tuple[DataClaim, ...],
    identities: dict[tuple[str, int], DataIdentity],
) -> tuple[DataClaim, ...]:
    """Re-address a shared source's DATA() claims through IMAGE's identities."""
    by_name: dict[str, list[int]] = {}
    for (item_image, va), item in identities.items():
        if item_image == image:
            by_name.setdefault(item.name, []).append(va)
    rebound: list[DataClaim] = []
    for claim in claims:
        where = f"{source}:{claim.line}"
        candidates = by_name.get(claim.name, [])
        if len(candidates) != 1:
            raise ValueError(
                f"{where}: {claim.name!r} is not a unique curated {image} datum "
                f"({len(candidates)} identities); name binding needs one"
            )
        rebound.append(replace(claim, va=candidates[0]))
    return tuple(rebound)


def _bind_data_claims(
    unit: str,
    image: str,
    source: Path,
    claims: tuple[DataClaim, ...],
    identities: dict[tuple[str, int], DataIdentity],
    claimed: dict[tuple[str, int], str],
) -> tuple[Datum, ...]:
    """Validate one source's DATA() claims against the curated data identities."""
    data: list[Datum] = []
    for claim in claims:
        where = f"{source}:{claim.line}"
        identity = identities.get((image, claim.va))
        if identity is None:
            raise ValueError(
                f"{where}: DATA({claim.va:#x}) is not a curated {image} datum in "
                "data_identities.tsv"
            )
        if claim.name != identity.name:
            raise ValueError(
                f"{where}: DATA({claim.va:#x}) defines {claim.name!r} but the identity "
                f"inventory names it {identity.name!r}"
            )
        if claim.size != identity.size:
            raise ValueError(
                f"{where}: DATA({claim.va:#x}, {claim.size:#x}) disagrees with the curated "
                f"size {identity.size:#x} of {identity.name}"
            )
        if identity.storage not in {"load", "bss"}:
            raise ValueError(
                f"{where}: {identity.name} has storage {identity.storage!r}; only load and "
                "bss data can be claimed"
            )
        owner = claimed.get((image, claim.va))
        if owner is not None:
            raise ValueError(f"{where}: {identity.name} is already claimed by unit {owner!r}")
        if data and claim.va <= data[-1].va:
            raise ValueError(
                f"{where}: DATA({claim.va:#x}) does not ascend after {data[-1].va:#x}; "
                "data claims follow the linked order"
            )
        claimed[(image, claim.va)] = unit
        data.append(Datum(claim.va, claim.size, identity.name, identity.storage, identity.scope,
                          identity.section, identity.reservation_size))
    return tuple(data)


def load(
    path: Path = UNITS_MANIFEST,
    *,
    config_dir: Path = RETAIL_CONFIG,
    require_sources: bool = True,
    write_bindings_file: bool = False,
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
    identities = identity_names(config_dir)
    vendored_names = {
        (function.image, function.va): function.symbol
        for image in IMAGE_LAYOUTS
        for function in catalog.functions[image]
        if function.scope == "vendored"
    }
    curated_data = data_identities(config_dir)
    units: list[Unit] = []
    seen_names: set[str] = set()
    claimed: dict[tuple[str, int], str] = {}
    claimed_data: dict[tuple[str, int], str] = {}
    binding_rows: list[dict[str, object]] = []
    for index, row in enumerate(raw_units, 1):
        if not isinstance(row, dict):
            raise ValueError(f"{path}: unit #{index} is not a table")
        required = {"unit", "image", "source", "profile"}
        optional = {"bind", "defines", "scope"}
        missing = required - set(row)
        extra = set(row) - required - optional
        if "va" in extra:
            raise ValueError(
                f"{path}: unit #{index} carries `va`; addresses live only in ADDRESS() claims"
            )
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
        profile_name = str(row["profile"])
        if profile_name not in profiles:
            raise ValueError(f"{path}: unit {name!r} references unknown profile {profile_name!r}")
        source = Path(str(row["source"]))
        if source.is_absolute() or ".." in source.parts:
            raise ValueError(f"{path}: unit {name!r} source must be repo-relative")
        source_path = REPO / source
        profile = profiles[profile_name]
        scope = str(row.get("scope", "decomp"))
        if scope not in {"decomp", "vendored"}:
            raise ValueError(
                f"{path}: unit {name!r} scope must be 'decomp' or 'vendored'"
            )
        if source.suffix.lower() not in LANGUAGE_SUFFIXES[profile.language]:
            raise ValueError(
                f"{path}: unit {name!r} source suffix does not match {profile.language}"
            )
        if not source_path.is_file():
            if require_sources:
                raise ValueError(f"{path}: unit {name!r} source is missing: {source}")
            continue
        stale = stale_address_names(
            source_path,
            image,
            {**vendored_names, **identities},
            {key: item.name for key, item in curated_data.items()},
        )
        if stale:
            shown = ", ".join(f"{token} -> {name}" for token, name in stale)
            raise ValueError(
                f"{source}: address-derived spellings of labelled identities: {shown}"
            )
        bind = str(row.get("bind", "address"))
        if bind not in {"address", "name"}:
            raise ValueError(f"{path}: unit {name!r} has unknown bind mode {bind!r}")
        raw_defines = row.get("defines", [])
        if not isinstance(raw_defines, list) or not all(
            isinstance(item, str) and DEFINE_RE.fullmatch(item) for item in raw_defines
        ):
            raise ValueError(
                f"{path}: unit {name!r} defines must be a list of preprocessor names "
                "(NAME or NAME=value)"
            )
        defines = tuple(raw_defines)
        claims, data_claims = scan_source(source_path)
        rodata_claims = scan_rodata_claims(source_path)
        image_token = image.removesuffix(".EXE")
        if any(claim.image is not None for claim in claims):
            # A shared source spells every image's addresses with ADDRESS_AT();
            # this unit keeps only the claims for its own image. Plain ADDRESS()
            # claims (image None) still default to the unit's image.
            if bind == "name":
                raise ValueError(
                    f"{source}: ADDRESS_AT() claims cannot combine with "
                    'bind = "name"'
                )
            claims = tuple(
                claim for claim in claims
                if claim.image is None or claim.image == image_token
            )
        if bind == "name":
            # A shared source carries the claims of its primary image; another
            # image reuses it by resolving every claimed definition through
            # that image's own identity tables, so the addresses stay
            # image-qualified while the C text is written once.
            if rodata_claims:
                raise ValueError(
                    f"{source}: RODATA() ranges are image-specific; unit {name!r} cannot "
                    "bind them by name"
                )
            claims = _rebind_claims_by_name(source, image, claims, identities)
            data_claims = _rebind_data_claims_by_name(source, image, data_claims, curated_data)
        rodata: tuple[int, int] | None = None
        if len(rodata_claims) > 1:
            raise ValueError(f"{source}: a unit claims at most one RODATA() range")
        if rodata_claims:
            claim = rodata_claims[0]
            layout = IMAGE_LAYOUTS[image]
            if claim.size <= 0 or claim.va & 3 or not layout.contains(claim.va, claim.size):
                raise ValueError(
                    f"{source}:{claim.line}: RODATA({claim.va:#x}, {claim.size:#x}) must be a "
                    f"word-aligned range inside the {image} load image"
                )
            for other in units:
                if other.image == image and other.rodata is not None and (
                    claim.va < other.rodata[0] + other.rodata[1]
                    and other.rodata[0] < claim.va + claim.size
                ):
                    raise ValueError(
                        f"{source}:{claim.line}: RODATA range overlaps unit {other.unit!r}"
                    )
            rodata = (claim.va, claim.size)
        if not claims:
            raise ValueError(f"{path}: unit {name!r} source has no ADDRESS() claim: {source}")
        functions = _bind_claims(
            path,
            name,
            image,
            source,
            claims,
            catalog,
            identities,
            claimed,
            scope,
        )
        data = _bind_data_claims(name, image, source, data_claims, curated_data, claimed_data)
        if units and units[-1].image == image and units[-1].va >= functions[0].va:
            raise ValueError(
                f"{path}: unit {name!r} at {functions[0].va:#x} is listed after "
                f"{units[-1].unit!r} at {units[-1].va:#x}; units follow the linked order"
            )
        units.append(
            Unit(
                name,
                image,
                source.as_posix(),
                profile_name,
                functions,
                data,
                rodata,
                defines,
                scope,
            )
        )
        for ordinal, (claim, function) in enumerate(zip(claims, functions)):
            binding_rows.append({
                "image": image,
                "va": f"{function.va:#x}",
                "kind": "function",
                "name": function.symbol,
                "unit": name,
                "source": source.as_posix(),
                "line": claim.line,
                "ordinal": ordinal,
            })
        for ordinal, (claim, datum) in enumerate(zip(data_claims, data)):
            binding_rows.append({
                "image": image,
                "va": f"{datum.va:#x}",
                "kind": "data",
                "name": datum.symbol,
                "unit": name,
                "source": source.as_posix(),
                "line": claim.line,
                "ordinal": ordinal,
            })
    if write_bindings_file:
        write_bindings(binding_rows)
    return Manifest(profiles, tuple(units))
