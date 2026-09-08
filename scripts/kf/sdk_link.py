"""Convert supplied Psy-Q LNK v2 library members into MIPS ELF link inputs.

This is a library format adapter, never reconstructed game code. The pinned
psyk parser decodes records; GNU as records the original payload and relocations.
Unsupported records, expressions and allocations fail instead of being dropped.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

from elftools.elf.elffile import ELFFile


ALIGNMENTS = {2: 1, 4: 2, 8: 4, 16: 16}
SECTION_NAMES = {'.text', '.rdata', '.data', '.sdata', '.sbss', '.bss'}
SYMBOL = re.compile(r'[A-Za-z_$][A-Za-z0-9_$]*\Z')
PATCH_TYPES = {16: 'R_MIPS_32', 74: 'R_MIPS_26', 82: 'R_MIPS_HI16', 84: 'R_MIPS_LO16'}


@dataclass
class Section:
    name: str
    alignment: int
    data: bytearray = field(default_factory=bytearray)
    size: int = 0
    patches: list[tuple[int, int, str]] = field(default_factory=list)
    reservations: list[tuple[int, int]] = field(default_factory=list)

    @property
    def bss(self) -> bool:
        return self.name in {'.bss', '.sbss'}


@dataclass
class Member:
    sections: dict[str, Section] = field(default_factory=dict)
    # Indexed XREF / XDEF / XBSS identities and exported allocations.
    names: dict[str, str] = field(default_factory=dict)
    exports: list[tuple[str, str, int]] = field(default_factory=list)
    common: list[tuple[str, str, int]] = field(default_factory=list)


def parse(listing: str) -> Member:
    if re.findall(r'^Header : LNK version (\d+)$', listing, re.M) != ['2']:
        raise ValueError('expected one LNK v2 member')
    if re.findall(r'^46 : Processor type (\d+)$', listing, re.M) != ['7']:
        raise ValueError('expected one MIPS processor record')
    member = Member()
    active = None
    pending = 0
    chunk = bytearray()
    record_base = 0
    ended = False
    for line in listing.splitlines():
        if pending:
            if not line.strip():
                continue
            dump = re.fullmatch(r'([0-9a-f]+): ([0-9a-f ]+)', line)
            if dump is None or int(dump[1], 16) != len(chunk):
                raise ValueError('truncated or disordered code record')
            chunk.extend(bytes.fromhex(dump[2]))
            if len(chunk) > pending:
                raise ValueError('oversized code record')
            if len(chunk) == pending:
                active.data.extend(chunk)
                active.size += len(chunk)
                pending = 0
            continue
        record = re.match(r'^(\d+) : ', line)
        if record is None:
            continue  # Header, whitespace and non-runtime debug record details.
        if ended:
            raise ValueError('record after EOF')
        kind = int(record[1])
        if kind == 16:
            match = re.fullmatch(
                r"16 : Section symbol number ([0-9a-f]+) '([^']+)' in group 0 alignment (\d+)", line)
            if (match is None or match[1] in member.sections or match[2] not in SECTION_NAMES
                    or int(match[3]) not in ALIGNMENTS
                    or any(s.name == match[2] for s in member.sections.values())):
                raise ValueError(f'unsupported section: {line}')
            member.sections[match[1]] = Section(match[2], ALIGNMENTS[int(match[3])])
        elif kind == 6:
            active = member.sections[line.rsplit(' ', 1)[1]]
        elif kind == 2:
            if active is None or active.bss:
                raise ValueError('code outside an initialized section')
            pending = int(re.fullmatch(r'2 : Code (\d+) bytes', line)[1])
            record_base, chunk = active.size, bytearray()
        elif kind == 8:
            if active is None:
                raise ValueError('reservation outside a section')
            size = int(re.fullmatch(r'8 : Uninitiali[sz]ed data, (\d+) bytes', line)[1])
            active.reservations.append((active.size, size))
            if not active.bss:
                # Flat output fills reserved holes with zeros; these are not
                # supplied SDK payload bytes and remain explicit in the report.
                active.data.extend(bytes(size))
            active.size += size
        elif kind == 10:
            match = re.fullmatch(r'10 : Patch type (\d+) at offset ([0-9a-f]+) with (.+)', line)
            if active is None or active.bss or match is None or int(match[1]) not in PATCH_TYPES:
                raise ValueError(f'unsupported patch: {line}')
            offset = record_base + int(match[2], 16)
            if offset & 3 or offset + 4 > active.size:
                raise ValueError('patch outside preceding code record')
            if any(p[0] == offset for p in active.patches):
                raise ValueError('multiple SDK patches at one word')
            active.patches.append((offset, int(match[1]), match[3]))
        elif kind in {12, 14, 48}:
            if kind == 14:
                match = re.fullmatch(r"14 : XREF symbol number ([0-9a-f]+) '([^']+)'", line)
            elif kind == 12:
                match = re.fullmatch(
                    r"12 : XDEF symbol number ([0-9a-f]+) '([^']+)' at offset ([0-9a-f]+) in section ([0-9a-f]+)", line)
            else:
                match = re.fullmatch(
                    r"48 : XBSS symbol number ([0-9a-f]+) '([^']+)' size ([0-9a-f]+) in section ([0-9a-f]+)", line)
            if match is None or not SYMBOL.fullmatch(match[2]) or match[1] in member.names:
                raise ValueError(f'unsupported symbol: {line}')
            member.names[match[1]] = match[2]
            if kind != 14:
                (member.exports if kind == 12 else member.common).append(
                    (match[2], match[4], int(match[3], 16)))
        elif kind == 0:
            ended = True
        elif kind not in {18, 28, 46, 74, 76}:
            raise ValueError(f'unsupported LNK record: {line}')
    if pending or not ended:
        raise ValueError('truncated LNK listing')
    for name, section, offset in member.exports:
        if section not in member.sections or offset > member.sections[section].size:
            raise ValueError(f'export outside section: {name}')
    for name, section, size in member.common:
        if section not in member.sections or member.sections[section].name != '.bss' or not size:
            raise ValueError(f'unsupported XBSS allocation: {name}')
    return member


def expression(value: str, member: Member, prefix: str) -> tuple[str | None, int]:
    """Reduce a native expression to one ELF referent plus its explicit addend."""
    if value.startswith('$') and re.fullmatch(r'\$[0-9a-f]+', value):
        return None, int(value[1:], 16)
    match = re.fullmatch(r'\[([0-9a-f]+)\]', value)
    if match:
        return member.names[match[1]], 0
    match = re.fullmatch(r'sect(base|start|end)\(([0-9a-f]+)\)', value)
    if match:
        section = member.sections[match[2]]
        if match[1] == 'base':
            return f'{prefix}_{match[2]}', 0
        return f'__kf_sdk_{match[1]}_{section.name[1:]}', 0
    if value.startswith('(') and value.endswith(')'):
        depth = 0
        for index, char in enumerate(value[1:-1], 1):
            if char == '(':
                depth += 1
            elif char == ')':
                depth -= 1
            elif char in '+-' and depth == 0:
                left, a = expression(value[1:index], member, prefix)
                right, b = expression(value[index + 1:-1], member, prefix)
                if char == '+' and not (left and right):
                    return left or right, (a + b) & 0xffffffff
                # Native PSYLINK consumes subtraction operands in reverse
                # order from psyk's printed expression (native controls).
                if char == '-' and left is None:
                    return right, (b - a) & 0xffffffff
                break
    raise ValueError(f'unsupported SDK expression: {value}')


def convert(member: Member, output: Path, *, identity: str) -> dict:
    """Materialize full SDK sections; retain the assembler's separate provenance."""
    output.parent.mkdir(parents=True, exist_ok=True)
    prefix = '__kf_sdk_' + hashlib.sha256(identity.encode()).hexdigest()[:16]
    assembly = []
    payloads = {}
    for number, section in member.sections.items():
        internal = '.kf_sdk' + section.name
        flags = 'ax' if section.name == '.text' else 'a' if section.name == '.rdata' else 'aw'
        assembly += [f'.section {internal},"{flags}",@{"nobits" if section.bss else "progbits"}',
                     f'{prefix}_{number}:']
        payload = bytearray(section.data)
        patches = []
        for offset, kind, value in section.patches:
            referent, addend = expression(value, member, prefix)
            word = struct.unpack_from('<I', payload, offset)[0]
            if kind == 16:
                word = addend
            elif kind == 74:
                if addend & 3:
                    raise ValueError('unaligned SDK jump addend')
                word = (word & 0xfc000000) | ((addend >> 2) & 0x3ffffff)
            else:
                immediate = (addend + 0x8000) >> 16 if kind == 82 else addend
                word = (word & 0xffff0000) | (immediate & 0xffff)
            struct.pack_into('<I', payload, offset, word)
            if referent:
                patches.append(f'.reloc {prefix}_{number}+{offset}, {PATCH_TYPES[kind]}, {referent}')
        if section.bss:
            assembly.append(f'.space {section.size}')
        else:
            blob = output.with_name(output.stem + f'.{number}.bin')
            blob.write_bytes(payload)
            assembly.append(f'.incbin "{blob.name}"')
        assembly += patches
        payloads[section.name] = bytes(payload)
    for name, number, offset in member.exports:
        assembly += [f'.globl {name}', f'.set {name}, {prefix}_{number}+{offset}']
    for name, _number, size in member.common:
        # XBSS controls establish four-byte reservation rounding. Allocation
        # order remains the GNU linker's, visible in the resulting map.
        assembly.append(f'.comm {name}, {(size + 3) & ~3}, 4')
    source = output.with_suffix('.s')
    source.write_text('\n'.join(assembly) + '\n')
    raw = output.with_suffix('.raw.o')
    subprocess.run(['mipsel-linux-gnu-as', '-EL', '-mips1', '-mabi=32', '-G0', '-no-pad-sections',
                    '-o', raw.name, source.name], cwd=output.parent, check=True,
                   capture_output=True)
    arguments = ['mipsel-linux-gnu-objcopy']
    for name in ('.text', '.data', '.bss'):
        arguments += ['--remove-section', name]
    for section in member.sections.values():
        internal = '.kf_sdk' + section.name
        arguments += ['--rename-section', f'{internal}={section.name}',
                      '--set-section-alignment', f'{internal}={section.alignment}']
    subprocess.run([*arguments, raw.name, output.name], cwd=output.parent, check=True,
                   capture_output=True)
    elf = ELFFile(io.BytesIO(output.read_bytes()))
    for section in member.sections.values():
        actual = elf.get_section_by_name(section.name)
        if (actual is None or actual['sh_size'] != section.size
                or actual['sh_addralign'] != section.alignment
                or (not section.bss and actual.data() != payloads[section.name])):
            raise ValueError(f'SDK conversion changed {section.name} layout or payload')
    return {'identity': identity, 'object': str(output),
            'object_sha256': hashlib.sha256(output.read_bytes()).hexdigest(),
            'sections': {s.name: {'size': s.size, 'alignment': s.alignment,
                                  'reservations': s.reservations}
                         for s in member.sections.values()},
            'relocations': sum(len(s.patches) for s in member.sections.values()),
            'exports': [name for name, _, _ in (*member.exports, *member.common)]}


def extract(library: Path, root: Path) -> list[tuple[Path, Member]]:
    """Use the pinned archive parser; a fresh directory prevents stale members."""
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    subprocess.run(['psyk', 'extract', str(library.resolve())], cwd=root, check=True,
                   capture_output=True)
    result = []
    for path in sorted(root.glob('*.OBJ')):
        listing = subprocess.run(['psyk', 'list', '--code', str(path)], check=True,
                                 capture_output=True, text=True).stdout
        path.with_suffix('.txt').write_text(listing)
        result.append((path, parse(listing)))
    return result


def archives(root: Path) -> tuple[list[Path], dict]:
    """Build original-library ELF archives, independently of all retail images."""
    library_dir = os.environ.get('PSYQ_LIB')
    if not library_dir:
        raise ValueError('PSYQ_LIB is required; enter nix develop')
    # Explicit archive search order. Recovered retail order remains WIP.
    names = ('LIBCD', 'LIBSND', 'LIBSPU', 'LIBGTE', 'LIBGPU', 'LIBETC',
             'LIBAPI', 'LIBPRESS', 'LIBSN')
    libraries = [Path(library_dir) / (name + '.LIB') for name in names]
    inputs = {str(p): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in [Path(__file__), *libraries]}
    inputs['tools'] = {tool: shutil.which(tool) for tool in (
        'psyk', 'mipsel-linux-gnu-as', 'mipsel-linux-gnu-objcopy', 'mipsel-linux-gnu-ar')}
    if not all(inputs['tools'].values()):
        raise ValueError('pinned SDK conversion tools are required')
    index = root / 'libraries.json'
    outputs = [root / (name + '.a') for name in names]
    if index.is_file():
        previous = json.loads(index.read_text())
        if previous.get('inputs') == inputs and all(
                p.is_file() and hashlib.sha256(p.read_bytes()).hexdigest()
                == previous.get('archives', {}).get(p.name) for p in outputs):
            return outputs, previous
    root.mkdir(parents=True, exist_ok=True)
    report = {'inputs': inputs, 'members': [], 'archives': {}}
    for library, archive in zip(libraries, outputs, strict=True):
        objects = []
        for path, member in extract(library, root / library.stem):
            obj = path.with_suffix('.o')
            result = convert(member, obj, identity=library.name + '/' + path.name)
            result['sdk_object_sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
            report['members'].append(result)
            objects.append(obj)
        # ar replaces existing members but does not remove obsolete ones.
        archive.unlink(missing_ok=True)
        subprocess.run(['mipsel-linux-gnu-ar', 'rcs', str(archive),
                        *(str(p) for p in objects)], check=True, capture_output=True)
        report['archives'][archive.name] = hashlib.sha256(archive.read_bytes()).hexdigest()
    index.write_text(json.dumps(report, indent=2) + '\n')
    return outputs, report


def verify_linked(linked: ELFFile, library_report: dict) -> dict:
    """Check every selected SDK payload/fixup at its actual linked address.

    Native patches contain explicit expressions. Re-evaluating those values
    independently detects REL pairing/addend errors in the format bridge.
    This verifies the provider conversion, not equality with the retail SDK.
    """
    table = linked.get_section_by_name('.symtab')
    symbols = {}
    for symbol in table.iter_symbols():
        if symbol.name:
            symbols.setdefault(symbol.name, []).append(symbol)
    sections = [s for s in linked.iter_sections() if s['sh_flags'] & 2 and s['sh_size']]
    result = {'members': [], 'provided_bytes_verified': 0, 'relocations_verified': 0}
    for row in library_report['members']:
        prefix = '__kf_sdk_' + hashlib.sha256(row['identity'].encode()).hexdigest()[:16]
        if not any(name.startswith(prefix + '_') for name in symbols):
            continue
        member = parse(Path(row['object']).with_suffix('.txt').read_text())
        checked = 0
        for number, section in member.sections.items():
            if section.bss or not section.size:
                continue
            bases = symbols.get(f'{prefix}_{number}', [])
            if len(bases) != 1:
                raise ValueError(f'{row["identity"]} {section.name}: missing or ambiguous SDK section base')
            base = bases[0]['st_value']
            expected = bytearray(section.data)
            for offset, kind, expr in section.patches:
                referent, addend = expression(expr, member, prefix)
                targets = symbols.get(referent, []) if referent else []
                if referent and len(targets) != 1:
                    raise ValueError(f'{row["identity"]}: ambiguous SDK referent {referent}')
                value = ((targets[0]['st_value'] if referent else 0) + addend) & 0xffffffff
                word = struct.unpack_from('<I', expected, offset)[0]
                if kind == 16:
                    word = value
                elif kind == 74:
                    word = (word & 0xfc000000) | ((value >> 2) & 0x3ffffff)
                else:
                    immediate = (value + 0x8000) >> 16 if kind == 82 else value
                    word = (word & 0xffff0000) | (immediate & 0xffff)
                struct.pack_into('<I', expected, offset, word)
            containing = [s for s in sections if s['sh_type'] == 'SHT_PROGBITS'
                          and s['sh_addr'] <= base and base + section.size <= s['sh_addr'] + s['sh_size']]
            if len(containing) != 1:
                raise ValueError(f'{row["identity"]}: SDK payload not in one linked section')
            actual = containing[0].data()[base-containing[0]['sh_addr']:][:section.size]
            if actual != expected:
                first = next(i for i, pair in enumerate(zip(actual, expected)) if pair[0] != pair[1])
                raise ValueError(f'{row["identity"]} {section.name}+{first:#x}: SDK linked-byte/fixup mismatch')
            checked += section.size - sum(size for _, size in section.reservations)
            result['relocations_verified'] += len(section.patches)
        result['provided_bytes_verified'] += checked
        result['members'].append(row['identity'])
    return result
