"""Image-qualified semantic bindings over the curated retail inventories."""

from __future__ import annotations

import bisect
from dataclasses import dataclass, field
from functools import lru_cache
from pathlib import Path

from scripts.kf.delink import image_key, sanitize_symbol
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv


@dataclass(frozen=True)
class Alias:
    name: str
    va: int
    source: str


@dataclass(frozen=True)
class Binding:
    image: str
    va: int
    size: int
    space: str
    kind: str
    name: str
    curated_name: str
    confidence: str
    provenance: str
    note: str = ""
    body_size: int = 0
    fragments: int = 1
    datatype: str = ""
    provider: str = ""
    library: str = ""
    unit: str = ""
    aliases: tuple[Alias, ...] = field(default_factory=tuple)

    @property
    def end(self) -> int:
        return self.va + self.size

    @property
    def body_end(self) -> int:
        return self.va + (self.body_size or self.size)

    @property
    def generated(self) -> bool:
        return not self.curated_name

    @property
    def vendored(self) -> bool:
        return bool(self.provider)

    def contains(self, va: int, size: int = 1) -> bool:
        return self.va <= va and va + size <= self.end

    def contains_body(self, va: int, size: int = 1) -> bool:
        return self.va <= va and va + size <= self.body_end

    def as_dict(self) -> dict[str, object]:
        return {
            "image": self.image,
            "va": self.va,
            "size": self.size,
            "body_size": self.body_size,
            "space": self.space,
            "kind": self.kind,
            "name": self.name,
            "curated_name": self.curated_name or None,
            "generated": self.generated,
            "confidence": self.confidence,
            "provenance": self.provenance,
            "note": self.note,
            "datatype": self.datatype or None,
            "provider": self.provider or None,
            "library": self.library or None,
            "unit": self.unit or None,
            "fragments": self.fragments,
            "aliases": [
                {"name": alias.name, "va": alias.va, "source": alias.source}
                for alias in self.aliases
            ],
        }


def parse_address(token: str) -> int | None:
    try:
        return int(token, 0)
    except ValueError:
        try:
            return int(token, 16)
        except ValueError:
            return None


def _labels(value: str, base: int) -> list[Alias]:
    aliases: list[Alias] = []
    for item in value.split(";"):
        item = item.strip()
        if not item:
            continue
        offset, separator, name = item.partition("=")
        if not separator or not name:
            continue
        aliases.append(Alias(name, base + parse_int(offset), "function-label"))
    return aliases


class Index:
    def __init__(self, image: str, config_dir: Path = RETAIL_CONFIG):
        if image not in IMAGE_LAYOUTS:
            raise ValueError(f"unknown retail image {image!r}")
        self.image = image
        self.config_dir = config_dir
        self.functions, self.data = self._load()
        self._fstarts = [row.va for row in self.functions]
        self._dstarts = [row.va for row in self.data]
        self._functions_by_start = {row.va: row for row in self.functions}
        self._data_by_start = {row.va: row for row in self.data}
        self._by_name = self._name_index()

    def _load(self) -> tuple[tuple[Binding, ...], tuple[Binding, ...]]:
        _, vendored_rows = read_tsv(self.config_dir / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in vendored_rows
            if row["image"] == self.image
        }
        try:
            manifest = load_manifest(config_dir=self.config_dir)
            units = {(unit.image, unit.va): unit.unit for unit in manifest.units}
        except ValueError:
            units = {}

        _, function_rows = read_tsv(self.config_dir / "functions.tsv")
        functions: list[Binding] = []
        for row in function_rows:
            if row["image"] != self.image:
                continue
            va = parse_int(row["va"])
            provider = vendored.get((self.image, va), {})
            curated = row["name"] or provider.get("name", "")
            name = sanitize_symbol(curated, f"func_{va:08x}")
            aliases = _labels(row["labels"], va)
            for alias in provider.get("aliases", "").split(";"):
                alias = alias.strip()
                if alias:
                    aliases.append(Alias(alias, va, "vendored-alias"))
            functions.append(Binding(
                image=self.image,
                va=va,
                size=parse_int(row["size"]),
                body_size=parse_int(row["body_size"]),
                fragments=int(row["fragments"]),
                space="text",
                kind=row["kind"],
                name=name,
                curated_name=curated,
                confidence=row["confidence"],
                provenance=row["provenance"],
                note=row["note"],
                provider=provider.get("provider", ""),
                library=provider.get("library", ""),
                unit=units.get((self.image, va), ""),
                aliases=tuple(aliases),
            ))

        _, data_rows = read_tsv(self.config_dir / "data.tsv")
        data: list[Binding] = []
        for row in data_rows:
            if row["image"] != self.image:
                continue
            va = parse_int(row["va"])
            curated = row["name"]
            data.append(Binding(
                image=self.image,
                va=va,
                size=parse_int(row["size"]),
                space="data",
                kind=row["kind"],
                name=sanitize_symbol(curated, f"DAT_{va:08x}"),
                curated_name=curated,
                confidence=row["confidence"],
                provenance=row["provenance"],
                note=row["note"],
                datatype=row["datatype"],
            ))
        return (
            tuple(sorted(functions, key=lambda item: item.va)),
            tuple(sorted(data, key=lambda item: item.va)),
        )

    def _name_index(self) -> dict[str, tuple[int, ...]]:
        names: dict[str, set[int]] = {}

        def add(name: str, va: int) -> None:
            if name:
                names.setdefault(name, set()).add(va)
                names.setdefault(name.lower(), set()).add(va)

        for binding in (*self.functions, *self.data):
            add(binding.name, binding.va)
            add(binding.curated_name, binding.va)
            add(binding.unit, binding.va)
            for alias in binding.aliases:
                add(alias.name, alias.va)
        return {name: tuple(sorted(values)) for name, values in names.items()}

    def function(self, va: int) -> Binding | None:
        return self._functions_by_start.get(va)

    def datum(self, va: int) -> Binding | None:
        return self._data_by_start.get(va)

    @staticmethod
    def _owner(rows: tuple[Binding, ...], starts: list[int], va: int) -> Binding | None:
        position = bisect.bisect_right(starts, va) - 1
        if position < 0:
            return None
        binding = rows[position]
        return binding if binding.contains(va) else None

    def function_owner(self, va: int, *, body_only: bool = False) -> Binding | None:
        binding = self._owner(self.functions, self._fstarts, va)
        if body_only and binding is not None and not binding.contains_body(va):
            return None
        return binding

    def data_owner(self, va: int) -> Binding | None:
        return self._owner(self.data, self._dstarts, va)

    def covering(self, va: int) -> Binding | None:
        return self.function_owner(va) or self.data_owner(va)

    def resolve(self, token: str) -> tuple[int, ...]:
        address = parse_address(token)
        if address is not None:
            return (address,)
        return self._by_name.get(token, self._by_name.get(token.lower(), ()))

    def exact_label(self, va: int) -> str | None:
        binding = self.function(va) or self.datum(va)
        if binding is not None:
            return binding.name
        for function in self.functions:
            for alias in function.aliases:
                if alias.va == va:
                    return alias.name
        return None

    def label(self, va: int) -> str:
        exact = self.exact_label(va)
        if exact:
            return exact
        binding = self.covering(va)
        if binding is None:
            return f"0x{va:08x}"
        return f"{binding.name}+0x{va - binding.va:x}"

    def row_label(self, binding: Binding) -> str:
        suffix = f" [{binding.unit}]" if binding.unit else ""
        return f"{binding.name}{suffix}"

    def units(self) -> dict[str, tuple[Binding, ...]]:
        rows: dict[str, list[Binding]] = {}
        for binding in self.functions:
            if binding.unit:
                rows.setdefault(binding.unit, []).append(binding)
        return {name: tuple(values) for name, values in sorted(rows.items())}

    @property
    def key(self) -> str:
        return image_key(self.image)


@lru_cache(maxsize=3)
def index(image: str) -> Index:
    return Index(image)
