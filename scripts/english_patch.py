"""Build a local translation delta from two verified resource trees, or embed it.

The delta contains replacement spans, never a disc image. Do not distribute the
generated payload until the translation's redistribution permission is settled.
"""

import argparse
import hashlib
from pathlib import Path
import re
import struct


MAGIC = b"KFEN\x01\0\0\0"
JAPANESE = "450b9f09ca34bedc1c8bc150e01b79bfe108c700dad2b2783246b926953deee2"
ENGLISH = "697b2b80d13a3e6e54b2d72f90a49e29ae6a31d6d45970b40aee908b94208595"


def read_tree(root, expected):
    files = {}
    for path in root.rglob("*"):
        if path.is_symlink():
            raise ValueError(f"Symlink in resource tree: {path}")
        if not path.is_file():
            continue
        name = path.relative_to(root).as_posix()
        if len(name) >= 128 or not re.fullmatch(r"[A-Z0-9_.-]+(?:/[A-Z0-9_.-]+)*", name):
            raise ValueError(f"Noncanonical resource path: {name}")
        files[name] = path.read_bytes()
    digest = hashlib.sha256()
    for name, data in sorted(files.items()):
        digest.update(struct.pack("<II", len(name), len(data)))
        digest.update(name.encode("ascii"))
        digest.update(data)
    if len(files) != 428 or digest.hexdigest() != expected:
        raise ValueError(f"Resource tree does not match the supported revision: {root}")
    return files


def replacement_spans(before, after):
    if len(before) != len(after):
        raise ValueError("This translation must preserve resource sizes")
    spans = []
    start = end = None
    for offset, (a, b) in enumerate(zip(before, after)):
        if a == b:
            continue
        # Bridging a short unchanged gap costs less than another span header.
        if end is not None and offset - end > 8:
            spans.append((start, after[start:end]))
            start = None
        if start is None:
            start = offset
        end = offset + 1
    if start is not None:
        spans.append((start, after[start:end]))
    return spans


def create(japanese, english):
    before, after = read_tree(japanese, JAPANESE), read_tree(english, ENGLISH)
    if before.keys() != after.keys():
        raise ValueError("Translation changed the file list")
    records = []
    for name in sorted(before):
        spans = replacement_spans(before[name], after[name])
        if not spans:
            continue
        record = bytearray(struct.pack("<III", len(name), len(after[name]), len(spans)))
        record.extend(name.encode("ascii"))
        for offset, data in spans:
            record.extend(struct.pack("<II", offset, len(data)))
            record.extend(data)
        records.append(record)
    return MAGIC + struct.pack("<I", len(records)) + b"".join(records)


def embed(payload):
    # A trailing sentinel keeps the empty-payload build standard C++.
    rows = [",".join(str(byte) for byte in payload[i:i + 32]) + ","
            for i in range(0, len(payload), 32)]
    return "// Generated; do not commit.\nstatic const unsigned char english_patch_data[] = {\n" + \
        "\n".join(rows) + "\n0};\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    build = commands.add_parser("create")
    build.add_argument("--japanese", type=Path, required=True)
    build.add_argument("--english", type=Path, required=True)
    build.add_argument("--output", type=Path, required=True)
    cpp = commands.add_parser("embed")
    cpp.add_argument("--input", type=Path)
    cpp.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.command == "create":
        payload = create(args.japanese, args.english)
        args.output.write_bytes(payload)
        print(f"Created local English delta: {len(payload)} bytes")
    else:
        args.output.write_text(embed(args.input.read_bytes() if args.input else b""))


if __name__ == "__main__":
    main()
