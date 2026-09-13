"""Count an explicitly specified integer field in fixed-size resource records.

The caller supplies a layout supported by independent codec/loader evidence.
Counts and byte references do not infer field meanings or object identities.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path
import struct
import sys


ENCODINGS = {
    "u8": "B", "s8": "b",
    "u16le": "<H", "s16le": "<h", "u16be": ">H", "s16be": ">h",
    "u32le": "<I", "s32le": "<i", "u32be": ">I", "s32be": ">i",
    "u64le": "<Q", "s64le": "<q", "u64be": ">Q", "s64be": ">q",
}


def census(
    data: bytes, *, offset: int, stride: int, field_offset: int, encoding: str,
    length: int | None = None, count: int | None = None, values: tuple[int, ...] = (),
) -> dict:
    """Validate the complete selected region before returning any observations."""
    if encoding not in ENCODINGS:
        raise ValueError(f"unknown encoding {encoding!r}")
    decoder = struct.Struct(ENCODINGS[encoding])
    if stride <= 0:
        raise ValueError("record stride must be positive")
    if offset < 0 or field_offset < 0:
        raise ValueError("resource and field offsets must be nonnegative")
    if field_offset + decoder.size > stride:
        raise ValueError("field extends beyond the record stride")
    if (length is None) == (count is None):
        raise ValueError("specify exactly one of length or count")
    if count is not None:
        if count < 0:
            raise ValueError("record count must be nonnegative")
        length = count * stride
    if length < 0:
        raise ValueError("region length must be nonnegative")
    if length % stride:
        raise ValueError(f"region length {length} leaves {length % stride} trailing record bytes")
    if offset > len(data) or length > len(data) - offset:
        raise ValueError(f"region [{offset:#x}, {offset + length:#x}) exceeds "
                         f"resource size {len(data):#x}")
    for value in values:
        try:
            decoder.pack(value)
        except (struct.error, OverflowError) as error:
            raise ValueError(f"requested value {value} cannot be encoded as {encoding}") from error
    histogram = {value: {"value": value, "count": 0, "references": []} for value in values}
    for index, record_offset in enumerate(range(offset, offset + length, stride)):
        absolute_field_offset = record_offset + field_offset
        value = decoder.unpack_from(data, absolute_field_offset)[0]
        group = histogram.setdefault(value, {"value": value, "count": 0, "references": []})
        group["count"] += 1
        group["references"].append({
            "row_index": index, "record_offset": record_offset,
            "field_offset": absolute_field_offset,
            "field_bytes": data[absolute_field_offset:absolute_field_offset + decoder.size].hex(),
            "record_bytes": data[record_offset:record_offset + stride].hex(),
        })
    return {
        "schema_version": 1,
        "resource_size": len(data), "resource_sha256": hashlib.sha256(data).hexdigest(),
        "region": {"offset": offset, "length": length, "count": length // stride,
                   "stride": stride, "sha256": hashlib.sha256(data[offset:offset + length]).hexdigest()},
        "field": {"offset": field_offset, "encoding": encoding, "size": decoder.size},
        "values": [histogram[value] for value in sorted(histogram)],
        "scope": "Selected fixed-record region only; layout and semantic meanings are supplied "
                 "by independent evidence, not inferred by this census.",
    }


def add_arguments(parser: argparse.ArgumentParser) -> None:
    def integer(value):
        return int(value, 0)

    parser.add_argument("resource", type=Path, help="binary resource file")
    parser.add_argument("--offset", type=integer, default=0, help="region byte offset (default 0)")
    extent = parser.add_mutually_exclusive_group(required=True)
    extent.add_argument("--length", type=integer, help="complete region byte length")
    extent.add_argument("--count", type=integer, help="number of complete records")
    parser.add_argument("--stride", type=integer, required=True, help="record size in bytes")
    parser.add_argument("--field-offset", type=integer, default=0, help="field offset within a record")
    parser.add_argument("--encoding", choices=tuple(ENCODINGS), required=True)
    parser.add_argument("--value", type=integer, action="append", default=[],
                        help="also report this value when its count is zero (repeatable)")
    parser.add_argument("--json", action="store_true", help="include hashes and full byte references")


def run(args: argparse.Namespace) -> int:
    try:
        path = args.resource.resolve()
        report = census(path.read_bytes(), offset=args.offset, length=args.length,
                        count=args.count, stride=args.stride, field_offset=args.field_offset,
                        encoding=args.encoding, values=tuple(args.value))
        report["resource"] = str(path)
    except (OSError, ValueError) as error:
        print(f"resources census: {error}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        writer = csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
        writer.writerow(("value", "count", "row_indices", "field_offsets"))
        for row in report["values"]:
            writer.writerow((row["value"], row["count"],
                             ";".join(str(ref["row_index"]) for ref in row["references"]),
                             ";".join(hex(ref["field_offset"]) for ref in row["references"])))
    print(f"{report['region']['count']} complete records; {len(report['values'])} values "
          "(including requested zero counts)", file=sys.stderr)
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    add_arguments(parser)
    return run(parser.parse_args(argv))


if __name__ == "__main__":
    raise SystemExit(main())
