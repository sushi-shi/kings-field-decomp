"""Complete config-owned data contributions with independent provider bytes.

This is an explicit comparison manifest, not permission to turn every census
gap or referenced prefix into a reconstruction. The current provider imports
complete, non-relocating Psy-Q .data sections. Unsupported SDK records fail
closed; they must not be stripped to manufacture a match.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass, field
from functools import lru_cache
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
from typing import TYPE_CHECKING

from scripts.kf.delink import Datum, Module, _write_bytes_if_changed, load_catalog
from scripts.kf.mips_elf import DefinedSymbol, STB_GLOBAL, STB_LOCAL, STT_OBJECT, write_mips_elf
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv
from scripts.kf.seed_vendored_functions import run_psyk

if TYPE_CHECKING:
    from scripts.kf.sema.image import RetailImage


FIELDS = ("image", "unit", "identity", "provider", "library", "member", "section",
          "alignment", "object_sha256", "evidence")


def _elf(blob: bytes):
    # Inventory validation remains usable with the standard-library Python.
    from elftools.elf.elffile import ELFFile

    return ELFFile(io.BytesIO(blob))


@dataclass(frozen=True)
class Contribution:
    image: str
    unit: str
    identity: str
    provider: str
    library: str
    member: str
    section: str
    alignment: int
    object_sha256: str
    evidence: str
    va: int
    size: int
    scope: str
    functions: tuple = ()
    rodata: None = None

    @property
    def image_key(self) -> str:
        return self.image.removesuffix(".EXE").lower()

    @property
    def object_name(self) -> str:
        return f"{self.va:08x}_{self.unit.removeprefix(self.image_key + '.')}.o"

    @property
    def data(self) -> tuple[Datum, ...]:
        return (Datum(self.va, self.size, self.identity, "load", self.scope),)

    @property
    def binding(self) -> int:
        return STB_GLOBAL if self.scope == "global" else STB_LOCAL

    def target_path(self, root: Path = BUILD / "delink") -> Path:
        return root / self.image_key / "data" / self.object_name

    def base_path(self, root: Path = BUILD / "objdiff") -> Path:
        return root / self.image_key / "data" / self.object_name


def load(config_dir: Path = RETAIL_CONFIG, modules: tuple[Module, ...] = ()) -> tuple[Contribution, ...]:
    path = config_dir / "data_contributions.tsv"
    if not path.is_file():
        return ()
    fields, rows = read_tsv(path)
    if tuple(fields) != FIELDS:
        raise ValueError(f"{path}: invalid config-data contribution columns")
    identities = read_tsv(config_dir / "data_identities.tsv")[1]
    catalog = load_catalog(config_dir)
    result = []
    for row in rows:
        if any(value is None for value in row.values()) or None in row:
            raise ValueError(f"{path}: malformed config-data row")
        image, name, unit = row["image"], row["identity"], row["unit"]
        if image not in IMAGE_LAYOUTS or not re.fullmatch(r"[a-z0-9][a-z0-9_.-]*", unit):
            raise ValueError(f"invalid config-data image/unit: {image}/{unit}")
        if not unit.startswith(image.removesuffix(".EXE").lower() + ".sdk."):
            raise ValueError(f"{unit}: SDK contributions must have an image-local SDK namespace")
        if row["provider"] != "psyq" or row["section"] != ".data":
            raise ValueError(f"{unit}: unsupported data provider/section")
        for key in ("library", "member"):
            if not re.fullmatch(r"[A-Za-z0-9_-]+\.[A-Za-z0-9]+", row[key]):
                raise ValueError(f"{unit}: invalid SDK {key}")
        if not re.fullmatch(r"[0-9a-f]{64}", row["object_sha256"]) or not row["evidence"]:
            raise ValueError(f"{unit}: missing SDK hash/evidence")
        matching = [r for r in identities if r["image"] == image and r["name"] == name]
        if len(matching) != 1:
            raise ValueError(f"{unit}: missing or ambiguous complete data identity {name}")
        identity = matching[0]
        if (identity["storage"] != "load" or identity["scope"] not in {"global", "static"}
                or identity["confidence"] not in {"supported", "proven"}):
            raise ValueError(f"{unit}: data owner lacks supported extent/storage/linkage")
        va, size, alignment = int(identity["va"], 0), int(identity["size"], 0), int(row["alignment"], 0)
        if (size <= 0 or not IMAGE_LAYOUTS[image].contains(va, size) or alignment <= 0
                or alignment > 0x80000000 or alignment & (alignment - 1) or va % alignment):
            raise ValueError(f"{unit}: invalid data extent/alignment")
        overlaps = [(f.va, f.body_size) for f in catalog.functions[image]]
        for module in modules:
            if module.image == image:
                overlaps.extend((d.va, d.size) for d in module.data)
                if module.rodata:
                    overlaps.append(module.rodata)
        overlaps.extend((c.va, c.size) for c in result if c.image == image)
        if any(start < va + size and va < start + extent for start, extent in overlaps):
            raise ValueError(f"{unit}: contribution overlaps another code/source/data owner")
        if any(c.unit == unit for c in result) or any(m.unit == unit for m in modules):
            raise ValueError(f"{unit}: duplicate contribution unit")
        result.append(Contribution(**{**row, "alignment": alignment}, va=va, size=size,
                                   scope=identity["scope"]))
    if result != sorted(result, key=lambda c: (tuple(IMAGE_LAYOUTS).index(c.image), c.va)):
        raise ValueError("config data contributions must ascend within each image")
    return tuple(result)


@dataclass(frozen=True)
class SdkSection:
    data: bytes
    alignment: int
    exports: tuple[tuple[str, int], ...]


def parse_sdk_section(listing: str, name: str) -> SdkSection:
    """Import a whole non-relocating contribution; no ranges or patch masking."""
    declarations = re.findall(r"Section symbol number ([0-9a-f]+) '([^']+)' in group \d+ alignment (\d+)", listing)
    selected = [(number, int(alignment)) for number, section, alignment in declarations if section == name]
    if len(selected) != 1:
        raise ValueError(f"missing or ambiguous SDK section {name}")
    number, alignment = selected[0]
    active, pending, chunk, data = None, 0, bytearray(), bytearray()
    exports = []
    for line in listing.splitlines():
        switch = re.fullmatch(r"6 : Switch to section ([0-9a-f]+)", line)
        code = re.fullmatch(r"2 : Code (\d+) bytes", line)
        dump = re.fullmatch(r"([0-9a-f]+): ([0-9a-f ]+)", line)
        if pending:
            if not line.strip():
                continue
            if dump is None or int(dump.group(1), 16) != len(chunk):
                raise ValueError("truncated or disordered SDK section bytes")
            chunk.extend(bytes.fromhex(dump.group(2)))
            if len(chunk) > pending:
                raise ValueError("oversized SDK code record")
            if len(chunk) == pending:
                data.extend(chunk)
                pending, chunk = 0, bytearray()
            continue
        if switch:
            active = switch.group(1)
        elif code and active == number:
            pending = int(code.group(1))
        elif active == number and re.match(r"(?:8|10) :", line):
            raise ValueError("SDK data relocation/reservation is not yet supported")
        elif "XDEF symbol" in line and re.search(rf"in section {number}$", line):
            match = re.search(r"'([^']+)' at offset ([0-9a-f]+) in section", line)
            if match is None:
                raise ValueError("unsupported SDK data symbol")
            exports.append((match.group(1), int(match.group(2), 16)))
        elif "symbol" in line.lower() and re.search(rf"in section {number}(?: |$)", line):
            raise ValueError("unsupported SDK data symbol record")
        elif active == number and dump:
            raise ValueError("extra SDK data bytes beyond the declared record")
        elif active == number and re.match(r"\d+ :", line) and not re.match(r"(?:0|16|28|46) :", line):
            raise ValueError("unsupported SDK data record")
    if pending or not data:
        raise ValueError("missing or truncated SDK data payload")
    return SdkSection(bytes(data), alignment, tuple(exports))


@lru_cache(maxsize=16)
def sdk_section(library: str, member: str, section: str, object_sha256: str,
                sdk_library_dir: str, tool: str) -> SdkSection:
    with tempfile.TemporaryDirectory(prefix="kf-sdk-data-") as temporary:
        root = Path(temporary)
        run_psyk(Path(tool), "extract", str(Path(sdk_library_dir) / library), cwd=root)
        obj = root / member
        if hashlib.sha256(obj.read_bytes()).hexdigest() != object_sha256:
            raise ValueError(f"{library}/{member}: SDK object hash differs")
        return parse_sdk_section(run_psyk(Path(tool), "list", "--code", str(obj)), section)


def source_section(contribution: Contribution) -> SdkSection:
    tool, directory = shutil.which("psyk"), os.environ.get("PSYQ_LIB")
    if tool is None or directory is None:
        raise ValueError("pinned Psy-Q archive tools are required; use nix develop")
    section = sdk_section(contribution.library, contribution.member, contribution.section,
                          contribution.object_sha256, directory, tool)
    # A private anonymous section gets a curated local owner, not an invented
    # SDK export. Named/offset exports still require exact global identity.
    expected_exports = {"global": ((contribution.identity, 0),), "static": ()}
    if (section.alignment != contribution.alignment or len(section.data) != contribution.size
            or contribution.scope not in expected_exports
            or section.exports != expected_exports[contribution.scope]):
        raise ValueError(f"{contribution.unit}: SDK whole-section extent/export/alignment differs")
    return section


def build_base(contribution: Contribution, output: Path) -> None:
    section = source_section(contribution)
    assembler, objcopy = shutil.which("mipsel-linux-gnu-as"), shutil.which("mipsel-linux-gnu-objcopy")
    if assembler is None or objcopy is None:
        raise ValueError("pinned MIPS assembler and objcopy are required")
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", contribution.identity):
        raise ValueError("unsupported SDK data identity spelling")
    with tempfile.TemporaryDirectory(prefix="kf-sdk-data-base-") as temporary:
        root = Path(temporary)
        (root / "section.bin").write_bytes(section.data)
        name = contribution.identity
        # Mechanical SDK-object conversion, not reconstructed game assembly.
        linkage = '.globl' if contribution.scope == 'global' else '.local'
        source = ('.section .kf_sdk_data,"aw",@progbits\n.balign 1\n' + f'{linkage} {name}\n'
                  + f'.type {name}, @object\n{name}:\n.incbin "section.bin"\n'
                  + f'.size {name}, .-{name}\n')
        (root / "section.s").write_text(source)
        subprocess.run([assembler, "-EL", "-mips1", "-mabi=32", "-o", "section.o", "section.s"],
                       cwd=root, check=True, capture_output=True)
        # MIPS GAS rounds its ordinary .data contribution to 16 bytes. A
        # byte-aligned temporary section preserves the SDK extent. Restore the
        # original name/alignment only after checking ordinary .data is empty.
        ordinary = _elf((root / "section.o").read_bytes()).get_section_by_name('.data')
        if ordinary is not None and ordinary['sh_size']:
            raise ValueError("SDK conversion emitted unexpected ordinary data")
        subprocess.run([objcopy, "--remove-section", ".data", "--rename-section", ".kf_sdk_data=.data",
                        "--set-section-alignment", f".kf_sdk_data={section.alignment}",
                        "section.o", "base.o"], cwd=root, check=True, capture_output=True)
        blob = (root / "base.o").read_bytes()
        validate_object(blob, contribution)
        if _elf(blob).get_section_by_name(".data").data() != section.data:
            raise ValueError("SDK conversion changed the complete data contribution")
        _write_bytes_if_changed(output, blob)


def validate_object(blob: bytes, contribution: Contribution) -> None:
    """Exact allocation identity and all runtime sections, not a symbol slice."""
    elf = _elf(blob)
    if elf.elfclass != 32 or not elf.little_endian or elf['e_machine'] != 'EM_MIPS' or elf['e_type'] != 'ET_REL':
        raise ValueError("unsupported config-data ELF format")
    for section in elf.iter_sections():
        metadata_type = {".reginfo": "SHT_MIPS_REGINFO", ".MIPS.abiflags": "SHT_MIPS_ABIFLAGS"}.get(section.name)
        if metadata_type and (section['sh_type'] != metadata_type or section['sh_size'] != 24
                              or section['sh_flags'] != 2):
            raise ValueError("invalid SDK ELF ABI metadata section")
    sections = [s for s in elf.iter_sections() if s['sh_flags'] & 2 and s['sh_size']
                and s.name not in {".reginfo", ".MIPS.abiflags"}]
    if len(sections) != 1 or sections[0].name != contribution.section:
        raise ValueError("extra or missing config-data runtime sections")
    section = sections[0]
    if (section['sh_size'] != contribution.size or section['sh_type'] != 'SHT_PROGBITS'
            or section['sh_flags'] != 3 or section['sh_addralign'] != contribution.alignment):
        raise ValueError("config-data whole-section storage/extent/alignment differs")
    index = elf.get_section_index(section.name)
    symtab = elf.get_section_by_name('.symtab')
    if symtab is None:
        raise ValueError("missing config-data symbol table")
    owned = [s for s in symtab.iter_symbols() if s['st_shndx'] == index
             and s['st_info']['type'] != 'STT_SECTION']
    if (len(owned) != 1 or owned[0].name != contribution.identity or owned[0]['st_value'] != 0
            or owned[0]['st_size'] != contribution.size or owned[0]['st_info']['type'] != 'STT_OBJECT'
            or owned[0]['st_info']['bind'] != ('STB_GLOBAL' if contribution.scope == 'global' else 'STB_LOCAL')
            or owned[0]['st_other']['visibility'] != 'STV_DEFAULT'):
        raise ValueError("config-data allocation identity/size/linkage differs")
    # The current provider has no patches. New providers must implement and
    # independently check their relocations; never discard them here.
    if any(s['sh_type'] in {'SHT_REL', 'SHT_RELA'} and s['sh_size'] for s in elf.iter_sections()):
        raise ValueError("unsupported config-data relocation")


def delink_object(contribution: Contribution, image: RetailImage, rows: list[dict]) -> bytes:
    if image.image != contribution.image:
        raise ValueError("config-data image namespace mismatch")
    if any(r['image'] == image.image and r['status'] != 'rejected'
           and any(value and int(value, 0) < contribution.va + contribution.size
                   and contribution.va < int(value, 0) + 4
                   for value in (r['site_va'], r.get('paired_site_va')))
           for r in rows):
        raise ValueError(f"{contribution.unit}: unresolved data relocation rows")
    return write_mips_elf(b"", None, 0, data=image.require(contribution.va, contribution.size),
                          data_symbols=(DefinedSymbol(contribution.identity, 0, contribution.size,
                                                      STT_OBJECT, contribution.binding),),
                          data_alignment=contribution.alignment)


@dataclass
class Comparison:
    image: str
    unit: str
    va: int
    size: int
    identity: str
    issues: list[str] = field(default_factory=list)
    target_relink: dict = field(default_factory=dict)
    base_relink: dict = field(default_factory=dict)
    evidence: dict = field(default_factory=dict)

    @property
    def matched(self) -> bool:
        return not self.issues and all(r.get("linker_ran") and not r.get("issues")
                                       and r.get("initialized_bytes_compared") == self.size
                                       for r in (self.target_relink, self.base_relink))


def compare(contribution: Contribution, image: RetailImage, target: Path, base: Path,
            scratch: Path) -> Comparison:
    from elftools.common.exceptions import ELFError
    from scripts.kf.data_match import Elf, _diff_init_section
    from scripts.kf.roundtrip import verify_unit

    result = Comparison(contribution.image, contribution.unit, contribution.va,
                        contribution.size, contribution.identity)
    try:
        provider = source_section(contribution)
        result.evidence = {"provider": contribution.provider, "library": contribution.library,
                           "member": contribution.member, "sdk_object_sha256": contribution.object_sha256,
                           "sdk_section": contribution.section, "sdk_alignment": contribution.alignment,
                           "sdk_payload_sha256": hashlib.sha256(provider.data).hexdigest()}
        for path in (target, base):
            validate_object(path.read_bytes(), contribution)
        result.evidence.update(target_object_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
                               base_object_sha256=hashlib.sha256(base.read_bytes()).hexdigest())
        if _elf(base.read_bytes()).get_section_by_name('.data').data() != provider.data:
            raise ValueError("base payload differs from independent SDK contribution")
        diff = _diff_init_section(contribution.section, Elf(target), Elf(base))
        if diff is None or diff.status != "match":
            raise ValueError(f"config-data comparison differs: {diff}")
        for role, path in (("target", target), ("base", base)):
            linked = verify_unit(contribution, image, {}, path, scratch)
            setattr(result, role + "_relink", asdict(linked))
            if linked.issues:
                result.issues.append(f"{role} relink failed: {linked.issues}")
    except (ELFError, OSError, RuntimeError, ValueError) as error:
        result.issues.append(str(error))
    return result


def audit(images, *, config_dir=RETAIL_CONFIG, delink_dir=BUILD / 'delink', objdiff_dir=BUILD / 'objdiff',
          modules=(), scratch=BUILD / 'roundtrip') -> list[Comparison]:
    from scripts.kf.sema.image import RetailImage

    selected = set(images)
    contributions = [c for c in load(config_dir, modules) if c.image in selected]
    scratch.mkdir(parents=True, exist_ok=True)
    results = []
    for contribution in contributions:
        result = Comparison(contribution.image, contribution.unit, contribution.va,
                            contribution.size, contribution.identity)
        try:
            image = RetailImage.load(contribution.image)
            target, base = contribution.target_path(delink_dir), contribution.base_path(objdiff_dir)
            result = compare(contribution, image, target, base, scratch)
            report_path = objdiff_dir / contribution.image_key / 'report.json'
            report = json.loads(report_path.read_text())
            result.issues.extend(report_failures(report, (contribution,)))
            if max(path.stat().st_mtime_ns for path in (target, base)) > report_path.stat().st_mtime_ns:
                result.issues.append('config-data native report is stale')
            result.evidence['native_report_sha256'] = hashlib.sha256(report_path.read_bytes()).hexdigest()
        except (OSError, ValueError) as error:
            result.issues.append(str(error))
        results.append(result)
    return results


def run(images, *, delink_dir=BUILD / 'delink', objdiff_dir=BUILD / 'objdiff') -> int:
    from scripts.kf.manifest import load as load_manifest

    reports = audit(images, modules=load_manifest().modules(), delink_dir=delink_dir, objdiff_dir=objdiff_dir)
    for image in images:
        selected = [r for r in reports if r.image == image]
        if not selected:
            continue
        print(f"{image} config data: {sum(r.matched for r in selected)}/{len(selected)} complete "
              f"contributions match provider and relink to retail")
        for row in selected:
            for issue in row.issues:
                print(f"  {row.unit}: {issue}")
    return int(any(not row.matched for row in reports))


def report_failures(document: dict | None, contributions) -> list[str]:
    """Require real data-only units in the shared native CLI/GUI report."""
    failures = []
    for contribution in contributions:
        valid = isinstance(document, dict) and isinstance(document.get('units'), list)
        units = [u for u in document['units'] if isinstance(u, dict) and u.get("name") == contribution.unit] if valid else []
        if len(units) != 1:
            failures.append(f"config-data report unit {contribution.unit!r} is missing or duplicated")
            continue
        unit = units[0]
        measures = unit.get("measures", {})
        data = [s for s in unit.get("sections", []) if s.get("name") == contribution.section]
        if (int(measures.get("total_data") or 0) != contribution.size
                or int(measures.get("matched_data") or 0) != contribution.size
                or int(measures.get("total_code") or 0) or int(measures.get("total_functions") or 0)
                or unit.get("functions") or len(data) != 1
                or any(int(s.get('size') or 0) for s in unit.get('sections', []) if s.get('name') != contribution.section)
                or int(data[0].get("size") or 0) != contribution.size
                or data[0].get("fuzzy_match_percent") != 100.0):
            failures.append(f"config-data report unit {contribution.unit!r} is not a complete exact data-only match")
    return failures
