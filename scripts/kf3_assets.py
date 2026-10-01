"""Extract KFIII model archives from a local ISO or MODE2/2352 BIN.

Usage: python scripts/kf3_assets.py DISC --output build/avatar-source/extracted
Outputs retain archive slot aliases and content hashes; no inferred NPC names.
Format reference: IvanDSM/KingsFieldRE, Tools/KFModTool/{core/tfile.cpp,
datahandlers/model.cpp}. Parsing is independent and rejects invalid extents.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import struct
import zlib


def extent(data: bytes, offset: int, size: int) -> bytes:
    if offset < 0 or size < 0 or offset > len(data) - size:
        raise ValueError(f"Invalid extent {offset}+{size} in {len(data)} bytes")
    return data[offset : offset + size]


def unpack(data: bytes, offset: int, fmt: str) -> tuple:
    return struct.unpack("<" + fmt, extent(data, offset, struct.calcsize("<" + fmt)))


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


class Disc:
    def __init__(self, path: Path):
        self.file = path.open("rb")
        self.size = path.stat().st_size
        self.stride, self.payload = 2048, 0
        self.file.seek(16 * 2048)
        if self.file.read(7) != b"\x01CD001\x01":
            self.stride, self.payload = 2352, 24
        pvd = self.read(16, 2048)
        if pvd[:7] != b"\x01CD001\x01":
            self.file.close()
            raise ValueError("Expected an ISO or a MODE2/2352 BIN data track")
        self.volume = pvd[40:72].decode("ascii").strip()
        if self.volume not in ("SLUS-00255", "SLPS-00377", "SLPS-03580", "SLPS-91089"):
            self.file.close()
            raise ValueError(f"Unsupported KFIII disc: {self.volume}")
        self.files: dict[str, tuple[int, int]] = {}
        self._seen: set[int] = set()
        self.directory(unpack(pvd, 158, "I")[0], unpack(pvd, 166, "I")[0])

    def read(self, lba: int, size: int) -> bytes:
        if size > 128 * 1024 * 1024 or (lba + (size + 2047) // 2048) * self.stride > self.size:
            raise ValueError("Disc extent exceeds its data track or resource limit")
        out = bytearray()
        while len(out) < size:
            self.file.seek(lba * self.stride + self.payload)
            chunk = self.file.read(min(2048, size - len(out)))
            if not chunk:
                raise ValueError("Truncated disc sector")
            out.extend(chunk)
            lba += 1
        return bytes(out)

    def directory(self, lba: int, size: int, prefix: str = "", depth: int = 0):
        if depth > 12 or size > 1024 * 1024 or lba in self._seen:
            raise ValueError("Invalid ISO directory")
        self._seen.add(lba)
        data, offset = self.read(lba, size), 0
        while offset < len(data):
            length = data[offset]
            if length == 0:
                offset = (offset // 2048 + 1) * 2048
                continue
            row = extent(data, offset, length)
            offset += length
            if length < 34:
                raise ValueError("Truncated ISO directory record")
            name = extent(row, 33, row[32])
            if name in (b"\0", b"\1"):
                continue
            name = name.decode("ascii").split(";")[0]
            if not name or name in (".", "..") or any(c in name for c in "/\\\0"):
                raise ValueError("Invalid ISO file name")
            start, count = unpack(row, 2, "I")[0], unpack(row, 10, "I")[0]
            if row[25] & ~3:
                raise ValueError("Unsupported ISO file flags")
            path = prefix + name
            if row[25] & 2:
                self.directory(start, count, path + "/", depth + 1)
            else:
                if path in self.files:
                    raise ValueError("Duplicate ISO path")
                self.files[path] = (start, count)


def archive_entries(data: bytes) -> list[tuple[list[int], bytes]]:
    count = unpack(data, 0, "H")[0]
    if not 0 < count <= 1022 or len(data) % 2048:
        raise ValueError("Invalid .T archive size or slot count")
    offsets = [value * 2048 for value in unpack(data, 2, "H" * (count + 1))]
    if offsets[0] != 2048 or offsets[-1] != len(data) or offsets != sorted(offsets):
        raise ValueError("Invalid .T sector offsets")
    # Repeated offsets are aliases of the next nonempty entry, not empty files.
    slots = defaultdict(list)
    for slot, offset in enumerate(offsets[:-1]):
        if offset < len(data):
            slots[offset].append(slot)
    unique = sorted(set(offsets))
    return [(slots[a], extent(data, a, b - a)) for a, b in zip(unique, unique[1:])]


def read_tmd(data: bytes, offset: int) -> list[dict]:
    identifier, flags, count = unpack(data, offset, "III")
    if identifier != 0x41 or flags != 0 or not 0 < count <= 64:
        raise ValueError("Expected relative, standard TMD model")
    base, objects = offset + 12, []
    extent(data, base, count * 28)
    for i in range(count):
        vo, nv, no, nn, po, np, scale = unpack(data, base + i * 28, "IIIIIIi")
        if not 0 < nv <= 16384 or nn > 16384 or np > 32768:
            raise ValueError("TMD exceeds mesh limits")
        vertices = [list(v[:3]) for v in struct.iter_unpack("<hhhh", extent(data, base + vo, nv * 8))]
        normals = [list(n[:3]) for n in struct.iter_unpack("<hhhh", extent(data, base + no, nn * 8))]
        faces, cursor = [], base + po
        for _ in range(np):
            _, words, flag, mode = unpack(data, cursor, "BBBB")
            body = extent(data, cursor + 4, words * 4)
            cursor += 4 + words * 4
            if mode & 0xE0 != 0x20 or flag & ~7:
                raise ValueError(f"Unsupported TMD primitive {mode:02x}/{flag:02x}")
            corners, textured, smooth, unlit = 4 if mode & 8 else 3, bool(mode & 4), bool(mode & 16), bool(flag & 1)
            indices_at = corners * 4 if textured else 4
            colors = [[255, 255, 255]] * corners
            if not textured or unlit:
                color_at = corners * 4 if textured else 0
                ncolors = corners if flag & 4 else 1
                colors = [list(unpack(body, color_at + j * 4, "BBBB")[:3]) for j in range(ncolors)]
                if ncolors == 1:
                    colors *= corners
                indices_at = color_at + ncolors * 4
            if unlit:
                vi = list(unpack(body, indices_at, "H" * corners))
                ni = []
            elif smooth:
                pairs = unpack(body, indices_at, "HH" * corners)
                ni, vi = list(pairs[::2]), list(pairs[1::2])
            else:
                indices = unpack(body, indices_at, "H" * (corners + 1))
                ni, vi = [indices[0]], list(indices[1:])
            if any(v >= nv for v in vi) or any(n >= nn for n in ni):
                raise ValueError("TMD vertex/normal index exceeds its object")
            face = {"vertices": vi, "normals": ni, "colors": colors, "mode": mode, "flags": flag}
            if textured:
                face.update(uv=[list(unpack(body, j * 4, "BB")) for j in range(corners)],
                            clut=unpack(body, 2, "H")[0], page=unpack(body, 6, "H")[0])
            faces.append(face)
        objects.append({"vertices": vertices, "normals": normals, "faces": faces, "scale": scale})
    return objects


def read_model(data: bytes) -> dict:
    size, animations, tmd = unpack(data, 0, "III")
    if not 12 <= size <= len(data) or animations > 256 or not 12 <= tmd < size:
        raise ValueError("Invalid MO header")
    data = data[:size]
    objects = read_tmd(data, tmd)
    targets, clips = [], []
    if animations:
        if tmd < 20 or len(objects) != 1:
            raise ValueError("Unsupported animated MO object layout")
        target_table, clip_table = unpack(data, 12, "II")
        first = unpack(data, target_table, "I")[0]
        if first < target_table or (first - target_table) % 4 or first - target_table > 16384:
            raise ValueError("Invalid MO morph table")
        offsets = unpack(data, target_table, "I" * ((first - target_table) // 4))
        nv = len(objects[0]["vertices"])
        for at in offsets:
            packets = unpack(data, at, "H")[0]
            at += 2
            deltas = []
            for _ in range(packets):
                x, y = unpack(data, at, "hh")
                at += 4
                if x == -32768:
                    if y < 0 or len(deltas) + y > nv:
                        raise ValueError("Invalid MO zero-delta run")
                    deltas.extend([[0, 0, 0]] * y)
                else:
                    z = unpack(data, at, "h")[0]
                    at += 2
                    deltas.append([x, y, z])
                if len(deltas) > nv:
                    raise ValueError("MO target exceeds vertex count")
            if len(deltas) != nv:
                raise ValueError("MO target does not cover its vertices")
            targets.append(deltas)
        for at in unpack(data, clip_table, "I" * animations):
            count = unpack(data, at, "I")[0]
            if count > 1024:
                raise ValueError("MO animation exceeds frame limit")
            frames = []
            for frame_at in unpack(data, at + 4, "I" * count):
                unknown, weight, frame_id, count = unpack(data, frame_at, "hhhh")
                if not 0 <= count <= 256:
                    raise ValueError("Invalid MO frame target count")
                ids = list(unpack(data, frame_at + 8, "H" * count))
                if any(index >= len(targets) for index in ids):
                    raise ValueError("MO frame references missing morph target")
                frames.append({"unknown": unknown, "weight": weight, "frame_id": frame_id, "targets": ids})
            clips.append(frames)
    return {"objects": objects, "targets": targets, "clips": clips, "sha256": digest(data)}


def obj_text(model: dict) -> str:
    lines, offset = ["# KFIII base mesh; original units, Y down; no inferred rig"], 1
    for i, obj in enumerate(model["objects"]):
        lines.append(f"o object_{i}")
        lines.extend("v " + " ".join(map(str, v)) for v in obj["vertices"])
        for face in obj["faces"]:
            vi = face["vertices"]
            # TMD quads use strip order; OBJ polygons use perimeter order.
            order = (0, 1, 3, 2) if len(vi) == 4 else (0, 1, 2)
            lines.append("f " + " ".join(str(offset + vi[j]) for j in order))
        offset += len(obj["vertices"])
    return "\n".join(lines) + "\n"


def texture_blocks(data: bytes, *, prefix: bool = False) -> tuple[list[dict], int]:
    """Read one RTIM list, stopping at its zero/FFFF sentinel."""
    blocks, at = [], 0
    while at < len(data):
        header = extent(data, at, 16)
        if header in (bytes(16), bytes([255]) * 16):
            return blocks, at
        # Some map bundles append data outside the RTIM rectangle stream.
        # Prefix mode reports the unconsumed boundary; it never scans for magic.
        if prefix and blocks and header[:8] != header[8:]:
            return blocks, at
        pair = []
        for _ in range(2):
            header = extent(data, at, 16)
            if header[:8] != header[8:]:
                raise ValueError(f"RTIM rectangle header copies disagree at {at}: {header.hex()}")
            x, y, width, height = unpack(header, 0, "HHHH")
            if not width or not height or x + width > 1024 or y + height > 512:
                raise ValueError("RTIM rectangle exceeds texture memory")
            pixels = extent(data, at + 16, width * height * 2)
            pair.append({"x": x, "y": y, "width": width, "height": height, "pixels": pixels})
            at += 16 + len(pixels)
        blocks.append({"palette": pair[0], "image": pair[1]})
    return blocks, at


def texture_memory(blocks: list[dict]) -> tuple[list[int], bytearray]:
    pixels, present = [0] * (1024 * 512), bytearray(1024 * 512)
    for block in blocks:
        for rect in (block["palette"], block["image"]):
            words = unpack(rect["pixels"], 0, "H" * (rect["width"] * rect["height"]))
            for y in range(rect["height"]):
                at = (rect["y"] + y) * 1024 + rect["x"]
                src = y * rect["width"]
                pixels[at : at + rect["width"]] = words[src : src + rect["width"]]
                present[at : at + rect["width"]] = bytes([1]) * rect["width"]
    return pixels, present


def texture_rgba(memory: tuple[list[int], bytearray], page: int, clut: int,
                 x: int, y: int, width: int, height: int) -> bytes:
    pixels, present = memory
    mode = (page >> 7) & 3
    if mode > 2 or not 0 <= x < x + width <= 256 or not 0 <= y < y + height <= 256:
        raise ValueError("Unsupported texture page or UV rectangle")
    page_x, page_y, divisor = (page & 15) * 64, ((page >> 4) & 1) * 256, 4 >> mode
    palette = (clut >> 6) * 1024 + (clut & 63) * 16
    out = bytearray()
    for v in range(y, y + height):
        for u in range(x, x + width):
            px = page_x + u // divisor
            at = (page_y + v) * 1024 + px
            if px >= 1024 or at >= len(pixels) or not present[at]:
                raise ValueError("Character texture references an unloaded image")
            word = pixels[at]
            if mode != 2:
                bits = 4 << mode
                index = (word >> (u % divisor * bits)) & ((1 << bits) - 1)
                at = palette + index
                if at >= len(pixels) or not present[at]:
                    raise ValueError("Character texture references an unloaded palette")
                word = pixels[at]
            rgb = [(word >> shift) & 31 for shift in (0, 5, 10)]
            out.extend([(c << 3) | (c >> 2) for c in rgb])
            out.append(255 if word else 0)
    return bytes(out)


def png_bytes(width: int, height: int, rgba: bytes) -> bytes:
    if len(rgba) != width * height * 4:
        raise ValueError("Invalid RGBA image extent")

    def chunk(kind: bytes, payload: bytes) -> bytes:
        return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload))

    scanlines = b"".join(b"\0" + rgba[y * width * 4 : (y + 1) * width * 4] for y in range(height))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(scanlines, 9)) + chunk(b"IEND", b""))


def export_textured_obj(model: dict, memory: tuple[list[int], bytearray], root: Path, stem: str):
    """Export a reviewable source mesh with per-material cropped PNG textures."""
    groups = defaultdict(list)
    for obj in model["objects"]:
        for face in obj["faces"]:
            if "uv" in face:
                groups[(face["page"], face["clut"])].extend(face["uv"])
    materials, rects = [], {}
    for (page, clut), points in sorted(groups.items()):
        lo = [min(p[i] for p in points) for i in range(2)]
        hi = [max(p[i] for p in points) + 1 for i in range(2)]
        x, y, width, height = lo[0], lo[1], hi[0] - lo[0], hi[1] - lo[1]
        rgba = texture_rgba(memory, page, clut, x, y, width, height)
        material = f"p{page}_c{clut}"
        filename = f"{stem}_{material}.png"
        (root / filename).write_bytes(png_bytes(width, height, rgba))
        materials.extend([f"newmtl {material}", "Kd 1 1 1", f"map_Kd {filename}", f"map_d {filename}", ""])
        rects[(page, clut)] = (x, y, width, height)
    lines = [f"mtllib {stem}.mtl", "# Original units; Y up for interchange"]
    vi, ti = 1, 1
    for i, obj in enumerate(model["objects"]):
        lines.append(f"o object_{i}")
        lines.extend(f"v {v[0]} {-v[1]} {-v[2]}" for v in obj["vertices"])
        for face in obj["faces"]:
            order = (0, 1, 3, 2) if len(face["vertices"]) == 4 else (0, 1, 2)
            if "uv" in face:
                page, clut = face["page"], face["clut"]
                x, y, width, height = rects[(page, clut)]
                lines.append(f"usemtl p{page}_c{clut}")
                lines.extend(f"vt {(u-x+0.5)/width:.9f} {1-(v-y+0.5)/height:.9f}" for u, v in face["uv"])
                lines.append("f " + " ".join(f"{vi+face['vertices'][j]}/{ti+j}" for j in order))
                ti += len(face["uv"])
            else:
                color = face["colors"][0]
                material = "solid_" + "_".join(map(str, color))
                materials.extend([f"newmtl {material}", "Kd " + " ".join(str(c/255) for c in color), ""])
                lines.append(f"usemtl {material}")
                lines.append("f " + " ".join(str(vi + face["vertices"][j]) for j in order))
        vi += len(obj["vertices"])
    (root / (stem + ".obj")).write_text("\n".join(lines) + "\n")
    (root / (stem + ".mtl")).write_text("\n".join(materials))


def avatar_mesh(model: dict, memory: tuple[list[int], bytearray], slot: int) -> bytes:
    """Derived presentation mesh, with its own atlas (never KF1 texture memory)."""
    materials = {}
    vertices = [v for obj in model["objects"] for v in obj["vertices"]]
    if any(obj["scale"] for obj in model["objects"]):
        raise ValueError("Unsupported model scale")
    bottom, top = max(v[1] for v in vertices), min(v[1] for v in vertices)
    if bottom <= top:
        raise ValueError("Avatar has no height")
    for obj in model["objects"]:
        for face in obj["faces"]:
            if "uv" not in face:
                continue
            key = (face["page"], face["clut"])
            bounds = materials.setdefault(key, [256, 256, 0, 0])
            for u, v in face["uv"]:
                bounds[:] = min(bounds[0], u), min(bounds[1], v), max(bounds[2], u), max(bounds[3], v)
    # One white texel row supplies untextured faces. Each material is cropped
    # before stacking; UVs address texel centers to prevent atlas-edge bleeding.
    atlas, rectangles, height = bytearray([255] * (256 * 4)), {}, 1
    for key, (x, y, right, lower) in sorted(materials.items()):
        w, h = right - x + 1, lower - y + 1
        rgba = texture_rgba(memory, *key, x, y, w, h)
        rectangles[key] = x, y, height
        for row in range(h):
            atlas.extend(rgba[row*w*4:(row+1)*w*4])
            atlas.extend(bytes((256 - w) * 4))
        height += h
    if height > 4096:
        raise ValueError("Avatar atlas too tall")
    triangles = bytearray()
    for obj in model["objects"]:
        for face in obj["faces"]:
            for indices in ((0, 1, 2), (1, 3, 2)) if len(face["vertices"]) == 4 else ((0, 1, 2),):
                for corner in indices:
                    x, y, z = obj["vertices"][face["vertices"][corner]]
                    position = [round(c * 2000 / (bottom - top)) for c in (x, y - bottom, z)]
                    if any(abs(c) > 8192 for c in position):
                        raise ValueError("Avatar position outside presentation bounds")
                    u, v = 0, 0
                    if "uv" in face:
                        ox, oy, row = rectangles[(face["page"], face["clut"])]
                        u, v = face["uv"][corner]
                        u, v = u - ox, v - oy + row
                    color = face["colors"][corner]
                    if "uv" in face and not face["flags"] & 1:
                        color = [128, 128, 128]
                    unlit = face["flags"] & 1
                    normal = [0, 0, 0] if unlit else obj["normals"][face["normals"][corner if len(face["normals"]) > 1 else 0]]
                    if any(abs(c) > 4096 for c in normal):
                        raise ValueError("Invalid avatar normal")
                    triangles.extend(struct.pack("<6hHH4B", *position, *normal, u, v, *color, unlit))
    count = len(triangles) // 60
    if not 0 < count <= 4096:
        raise ValueError("Avatar triangle limit exceeded")
    return struct.pack("<HHI", slot, height, count) + triangles + atlas


def extract(disc_path: Path, output: Path):
    disc = Disc(disc_path)
    try:
        manifest = {"version": 1, "volume": disc.volume, "archives": []}
        for name in ("MO", "MOF", "RTIM", "FDAT"):
            path = f"CD/COM/{name}.T"
            data = disc.read(*disc.files[path])
            root = output / name.lower()
            root.mkdir(parents=True, exist_ok=True)
            archive = {"path": path, "sha256": digest(data), "entries": []}
            for slots, entry in archive_entries(data):
                stem = f"{slots[0]:04d}"
                (root / (stem + ".bin")).write_bytes(entry)
                row = {"slots": slots, "file": f"{name.lower()}/{stem}.bin", "sha256": digest(entry)}
                if name in ("MO", "MOF"):
                    model = read_model(entry)
                    (root / (stem + ".json")).write_text(json.dumps(model, separators=(",", ":")) + "\n")
                    (root / (stem + ".obj")).write_text(obj_text(model))
                    geometry = json.dumps(model["objects"], separators=(",", ":")).encode()
                    row.update(model_sha256=model["sha256"], geometry_sha256=digest(geometry),
                               vertices=sum(len(o["vertices"]) for o in model["objects"]),
                               faces=sum(len(o["faces"]) for o in model["objects"]),
                               clips=len(model["clips"]))
                archive["entries"].append(row)
            manifest["archives"].append(archive)
        # This mapping is verified against the SLUS-00255 source archive.
        if disc.volume == "SLUS-00255":
            blocks, consumed = texture_blocks((output / "fdat/0084.bin").read_bytes())
            root = output / "npc"
            root.mkdir(exist_ok=True)
            model_maps = defaultdict(list)
            for area in range(28):
                db = (output / "fdat" / f"{area*3+1:04d}.bin").read_bytes()
                if unpack(db, 0, "I")[0] != 12992:
                    raise ValueError("Unexpected SLUS-00255 entity-definition section")
                for index in range(32):
                    model = db[4 + index * 120]
                    if model < 43 and area not in model_maps[model]:
                        model_maps[model].append(area)
            audit, avatars = [], []
            for row in manifest["archives"][0]["entries"]:
                slot = row["slots"][0]
                if slot > 42 or slot == 35:  # Slot 35 is the literal DUMMY marker.
                    continue
                stem = f"{slot:04d}"
                model = json.loads((output / "mo" / (stem + ".json")).read_text())
                areas = sorted({area for alias in row["slots"] for area in model_maps[alias]})
                if not areas:
                    raise ValueError(f"NPC {slot} has no map reference")
                area = areas[0]
                textures = (output / "rtim" / f"{area:04d}.bin").read_bytes()
                start = 0
                if area < 12:
                    _, start = texture_blocks(textures)
                    if extent(textures, start, 64) != bytes([255]) * 64:
                        raise ValueError("Missing map texture section separator")
                    start += 64
                local, end = texture_blocks(textures[start:], prefix=True)
                memory = texture_memory(blocks + local)
                result = {"slot": slot, "areas": areas, "texture_area": area,
                          "texture_start": start, "texture_end": start + end}
                try:
                    export_textured_obj(model, memory, root, stem)
                    avatars.append(avatar_mesh(model, memory, slot))
                    result["status"] = "exported"
                except ValueError as error:
                    # Keep raw geometry and report unresolved runtime palettes.
                    # Do not silently replace them with another character's skin.
                    result.update(status="unresolved_texture", error=str(error))
                audit.append(result)
            manifest["npc_textures"] = {"archive": "FDAT.T", "slot": 84, "blocks": len(blocks), "consumed": consumed}
            manifest["npcs"] = audit
            # The throne figure is a map object, not an MO NPC. Retain the MO
            # IDs and append MOF 545 (area 17, local object 1) at presentation ID 43.
            model = json.loads((output / "mof/0545.json").read_text())
            local, end = texture_blocks((output / "rtim/0017.bin").read_bytes(), prefix=True)
            memory = texture_memory(blocks + local)
            export_textured_obj(model, memory, root, "mof0545")
            avatars.append(avatar_mesh(model, memory, 43))
            manifest["map_characters"] = [{"slot": 43, "archive": "MOF.T", "source_slot": 545,
                                          "texture_area": 17, "texture_end": end, "status": "exported"}]
            # Stable IDs are independent of list order.
            (output / "characters.kfa").write_bytes(b"KFA1" + struct.pack("<HH", 1, len(avatars)) + b"".join(avatars))
        (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        for archive in manifest["archives"]:
            print(f"{archive['path']}: {len(archive['entries'])} entries")
    finally:
        disc.file.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("disc", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    extract(args.disc, args.output)
