"""Reviewed load-section assignments; omitted identities retain .data."""

from pathlib import Path

from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv


def load(config_dir: Path) -> dict[tuple[str, int], str]:
    path = config_dir / "data_sections.tsv"
    if not path.is_file():
        return {}
    fields, rows = read_tsv(path)
    if fields != ("image", "va", "name", "section", "evidence"):
        raise ValueError(f"{path}: invalid section assignment columns")
    identities = {(r["image"], int(r["va"], 0)): r
                  for r in read_tsv(config_dir / "data_identities.tsv")[1]}
    result = {}
    for row in rows:
        key = row["image"], int(row["va"], 0)
        identity = identities.get(key)
        if (row["image"] not in IMAGE_LAYOUTS or key in result or not identity or row["name"] != identity["name"]
                or identity["storage"] != "load" or row["section"] not in {".data", ".sdata"}
                or not row["evidence"].strip()):
            raise ValueError(f"{path}: invalid load-section assignment {key!r}")
        result[key] = row["section"]
    order = {name: index for index, name in enumerate(IMAGE_LAYOUTS)}
    if list(result) != sorted(result, key=lambda k: (order[k[0]], k[1])):
        raise ValueError(f"{path}: section assignments are not canonically sorted")
    return result
