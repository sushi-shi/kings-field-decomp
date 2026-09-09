"""Reviewed fixed BSS reservation extents, separate from the C object size."""

from pathlib import Path

from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv


def load(config_dir: Path) -> dict[tuple[str, int], int]:
    path = config_dir / 'data_reservations.tsv'
    if not path.is_file():
        return {}
    fields, rows = read_tsv(path)
    if fields != ('image', 'va', 'name', 'size', 'class', 'evidence'):
        raise ValueError(f'{path}: invalid reservation columns')
    identities = {(r['image'], int(r['va'], 0)): r
                  for r in read_tsv(config_dir / 'data_identities.tsv')[1]}
    result = {}
    for row in rows:
        key = row['image'], int(row['va'], 0)
        identity = identities.get(key)
        size = int(row['size'], 0)
        if (row['image'] not in IMAGE_LAYOUTS or key in result or not identity
                or row['name'] != identity['name'] or identity['storage'] != 'bss'
                or identity.get('scope') != 'static' or row['class'] != 'fixed'
                or size < int(identity['size'], 0) or not row['evidence'].strip()
                or key[1] + size > 0x80200000):
            raise ValueError(f'{path}: invalid fixed reservation {key!r}')
        for (image, va), other in identities.items():
            if image == key[0] and va != key[1] and (
                    va < key[1] + size and key[1] < va + int(other['size'], 0)):
                raise ValueError(f'{path}: reservation overlaps {other["name"]}')
        result[key] = size
    order = {name: index for index, name in enumerate(IMAGE_LAYOUTS)}
    if list(result) != sorted(result, key=lambda k: (order[k[0]], k[1])):
        raise ValueError(f'{path}: reservations are not canonically sorted')
    return result
