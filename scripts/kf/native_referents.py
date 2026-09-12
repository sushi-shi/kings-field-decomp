"""Reviewed, per-site data referents for native LNK comparison views."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re

from scripts.kf.paths import REPO
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv


MANIFEST = REPO / 'config/native_reloc_referents.tsv'
FIELDS = ('image', 'function', 'site_offset', 'paired_site_offset', 'owner', 'addend',
          'provenance')


@dataclass(frozen=True)
class NativeReferent:
    image: str
    function: str
    site_offset: int
    paired_site_offset: int
    owner: str
    addend: int
    provenance: str


def load(path: Path = MANIFEST) -> tuple[NativeReferent, ...]:
    fields, rows = read_tsv(path)
    if fields != FIELDS:
        raise ValueError(f'{path}: invalid native referent columns')
    result = []
    sites = set()
    for number, row in enumerate(rows, 1):
        if set(row) != set(FIELDS) or any(value is None for value in row.values()):
            raise ValueError(f'{path}: row {number}: incomplete native referent row')
        if row['image'] not in IMAGE_LAYOUTS or not row['provenance'].strip():
            raise ValueError(f'{path}: row {number}: image and provenance required')
        if any(not re.fullmatch(r'[A-Za-z_][\w.$]*', row[key])
               for key in ('function', 'owner')):
            raise ValueError(f'{path}: row {number}: invalid function/owner name')
        site, paired, addend = (int(row[key], 0)
                                for key in ('site_offset', 'paired_site_offset', 'addend'))
        if site < 0 or paired <= site or site % 4 or paired % 4:
            raise ValueError(f'{path}: row {number}: invalid HI/LO offsets')
        if not -(1 << 31) <= addend < (1 << 31):
            raise ValueError(f'{path}: row {number}: addend must be signed 32-bit')
        for offset in (site, paired):
            key = (row['image'], row['function'], offset)
            if key in sites:
                raise ValueError(f'{path}: row {number}: duplicate native referent site')
            sites.add(key)
        result.append(NativeReferent(row['image'], row['function'], site, paired,
                                    row['owner'], addend, row['provenance']))
    return tuple(result)
