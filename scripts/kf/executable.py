"""Link the three programs with supplied SDK libraries and compare PS-X EXEs.

The initial links use current source objects, native SDK archive conversions,
retail overlay startup and explicitly reported inventory-backed data. They are
comparison candidates, not proof of completed source/data reconstruction.
"""

from __future__ import annotations

import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import struct
import subprocess

from elftools.elf.elffile import ELFFile

from scripts.kf.delink import load_catalog
from scripts.kf.manifest import Unit, load as load_manifest
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv
from scripts.kf.sdk_link import SYMBOL, archives, parse, verify_linked
from scripts.kf.sema.image import RetailImage


RUNTIME = ('.rodata', '.text', '.data', '.sdata', '.sbss', '.bss')
METADATA = '.reginfo .MIPS.abiflags .pdr .mdebug.* .comment .note.GNU-stack .gnu.attributes'

# Complete-object text placements prove this contiguous PSX chain; SNDEF has
# no text and supplies the SDK's default stack-size datum. See object-link-order.md.
PSX_SDK_ORDER = (('LIBSN', 'SNMAIN'), ('LIBAPI', 'A36'), ('LIBAPI', 'C113'),
                 ('LIBAPI', 'C57'), ('LIBAPI', 'C66'), ('LIBAPI', 'C67'),
                 ('LIBAPI', 'C114'), ('LIBSN', 'SNDEF'))


def elf(path: Path) -> ELFFile:
    return ELFFile(io.BytesIO(path.read_bytes()))


def marker(index: int, section: str) -> str:
    return f'__kf_input_{index}_{section[1:]}'


def script(image: RetailImage, units: list[Unit], objects: list[Path],
           definitions: list[str]) -> str:
    """Keep whole input sections and their alignment; do not scatter symbols."""
    lines = [*definitions, 'SECTIONS {']
    for section in RUNTIME:
        address = (f' {image.load_start:#x}' if section == '.rodata'
                   else f' {units[0].va:#x}' if section == '.text' else '')
        native = 'rdata' if section == '.rodata' else section[1:]
        lines.append(f'{section}{address} : {{ __kf_sdk_start_{native} = .;')
        for index, obj in enumerate(objects):
            contribution = elf(obj).get_section_by_name(section)
            if contribution is not None and contribution['sh_size']:
                lines += [f' . = ALIGN({contribution["sh_addralign"]});',
                          f' {marker(index, section)} = .;', f' "{obj}"({section})']
        pattern = '*(.rodata .rdata)' if section == '.rodata' else f'*({section})'
        lines.append(pattern)
        if section == '.bss':
            lines.append('*(COMMON)')
        lines.append(f'__kf_sdk_end_{native} = .; }}')
    lines += [f'/DISCARD/ : {{ *({METADATA}) }}', '}']
    return '\n'.join(lines) + '\n'


def undefined(diagnostics: str) -> list[str]:
    return sorted(set(re.findall(r"undefined reference to [`']([^']+)'", diagnostics)))


def sdk_aliases(image: RetailImage, missing: list[str], sdk_root: Path) -> tuple[list[str], list[dict]]:
    """Resolve curated/API spelling differences only with identical body bytes."""
    catalog = load_catalog(RETAIL_CONFIG)
    by_address = {f.va: f for f in catalog.functions[image.image]}
    lines, rows = [], []
    for row in read_tsv(RETAIL_CONFIG / 'functions_vendored.tsv')[1]:
        if row['image'] != image.image:
            continue
        function = by_address[int(row['va'], 0)]
        if function.symbol not in missing or function.symbol == row['name']:
            continue
        listing = sdk_root / Path(row['library']).stem / (row['module'] + '.txt')
        if not listing.is_file():
            continue
        member = parse(listing.read_text())
        exports = [(number, offset) for name, number, offset in member.exports if name == row['name']]
        if len(exports) != 1:
            continue
        number, offset = exports[0]
        section = member.sections[number]
        raw = image.require(function.va, function.body_size)
        if (section.name != '.text' or section.data[offset:offset + len(raw)] != raw
                or any(offset <= p[0] < offset + len(raw) for p in section.patches)):
            continue
        if not SYMBOL.fullmatch(function.symbol) or not SYMBOL.fullmatch(row['name']):
            raise ValueError('unsupported SDK alias spelling')
        lines += [f'EXTERN({row["name"]})', f'{function.symbol} = {row["name"]};']
        rows.append({'symbol': function.symbol, 'sdk_symbol': row['name'],
                     'retail_va': function.va, 'library': row['library'], 'member': row['module'],
                     'body_bytes_verified': len(raw), 'body_sha256': hashlib.sha256(raw).hexdigest()})
    return lines, rows


def data_inputs(image: RetailImage, missing: list[str], units: list[Unit],
                objects: list[Path], root: Path) -> tuple[Path | None, list[str], list[dict]]:
    """Supply reviewed inventory storage, explicitly separated from C progress."""
    identities = [row for row in read_tsv(RETAIL_CONFIG / 'data_identities.tsv')[1]
                  if row['image'] == image.image]
    by_name = {row['name']: row for row in identities}
    catalog = load_catalog(RETAIL_CONFIG)
    census = {d.symbol: d for d in catalog.data[image.image]}
    owners = [(d.va, d.va + d.size, index, d.symbol)
              for index, unit in enumerate(units) for d in unit.data]
    definitions, rows, allocations = [], [], []
    for name in missing:
        if not SYMBOL.fullmatch(name):
            raise ValueError(f'unsupported data spelling: {name}')
        identity = by_name.get(name)
        datum = census.get(name)
        if identity is None and datum is None:
            continue
        va = int(identity['va'], 0) if identity else datum.va
        size = int(identity['size'], 0) if identity else datum.size
        storage = identity['storage'] if identity else 'load'
        if size <= 0 or storage not in {'load', 'bss'}:
            raise ValueError(f'{name}: unsupported inventory storage or extent')
        row = {'symbol': name, 'retail_va': va, 'size': size, 'storage': storage,
               'evidence': identity['evidence'] if identity else 'data.tsv',
               'source_reconstructed': False, 'extent_completeness_proven': False}
        containing = [owner for owner in owners if owner[0] <= va < owner[1]]
        if containing:
            if len(containing) != 1 or va + size > containing[0][1]:
                raise ValueError(f'{name}: ambiguous or cross-owner data reference')
            start, _, index, owner = containing[0]
            source = elf(objects[index])
            symbols = source.get_section_by_name('.symtab').get_symbol_by_name(owner)
            if len(symbols or ()) != 1 or not isinstance(symbols[0]['st_shndx'], int):
                raise ValueError(f'{name}: source owner has no unique section symbol')
            symbol = symbols[0]
            section = source.get_section(symbol['st_shndx']).name
            definitions.append(f'{name} = {marker(index, section)} + {symbol["st_value"] + va-start};')
            row.update(provider='source-owner-alias', owner=owner, offset=va-start)
        else:
            if any(va < end and start < va + size for start, end, _, _ in owners):
                raise ValueError(f'{name}: allocation overlaps source-owned storage')
            if any(u.rodata and va < u.rodata[0] + u.rodata[1]
                   and u.rodata[0] < va + size for u in units):
                raise ValueError(f'{name}: compiler-owned literal needs an owner-relative reference')
            if storage == 'load':
                # String-census rows exclude the NUL. Retain it after checking
                # the retail byte; the generated report exposes the extra byte.
                if identity is None and image.require(va + size, 1) == b'\0':
                    size += 1
                    row['size'] = size
                    row['terminator_bytes'] = 1
                payload = image.require(va, size)
                # No pointer word may quietly retain an obsolete address when
                # the linked source/library objects move. These first inputs
                # are plain strings, sound references, scalars and matrices.
                for offset in range((-va) & 3, size - 3, 4):
                    word = struct.unpack_from('<I', payload, offset)[0]
                    if 0x80010000 <= word < 0x80200000:
                        raise ValueError(f'{name}+{offset:#x}: retail pointer requires relocation ownership')
                row['payload_sha256'] = hashlib.sha256(payload).hexdigest()
                row['provider'] = 'retail-inventory-data'
            else:
                payload = None
                row['provider'] = 'inventory-bss-reservation'
            allocations.append((va, size, name, storage, payload))
        rows.append(row)
    allocations.sort()
    for left, right in zip(allocations, allocations[1:]):
        if left[0] + left[1] > right[0]:
            raise ValueError(f'overlapping inventory allocations: {left[2]}, {right[2]}')
    if not allocations:
        return None, definitions, rows
    lines = []
    for va, size, name, storage, payload in allocations:
        section = '.data' if storage == 'load' else '.bss'
        alignment = min(4, va & -va)
        lines += [f'.section .kf_link{section},"aw",@{"progbits" if payload is not None else "nobits"}',
                  f'.balign {alignment}', f'.globl {name}', f'.type {name}, @object', f'{name}:']
        if payload is None:
            lines.append(f'.space {size}')
        else:
            path = root / (name + '.bin')
            path.write_bytes(payload)
            lines.append(f'.incbin "{path.name}"')
        lines.append(f'.size {name}, {size}')
    (root / 'inventory.s').write_text('\n'.join(lines) + '\n')
    subprocess.run(['mipsel-linux-gnu-as', '-EL', '-mips1', '-mabi=32', '-G0', '-no-pad-sections',
                    '-o', 'inventory.raw.o', 'inventory.s'], cwd=root, check=True, capture_output=True)
    subprocess.run(['mipsel-linux-gnu-objcopy', '--remove-section', '.data', '--remove-section', '.bss',
                    '--rename-section', '.kf_link.data=.data', '--rename-section', '.kf_link.bss=.bss',
                    'inventory.raw.o', 'inventory.o'], cwd=root, check=True, capture_output=True)
    return root / 'inventory.o', definitions, rows


def sector_padding(extent: int, policy: str) -> bytes:
    """Container compatibility, independent of source data and retail payload.

    All three retail tails retain CPE v1 magic, unit zero and the start of a
    PC register record. The stale-buffer mechanism is inferred, not reproduced
    by either supplied CPE2X 1.3 binary. Keep that provenance in link reports.
    """
    size = (-extent) % 0x800
    if policy == 'zero':
        return bytes(size)
    if policy != 'cpe-v1-prefix':
        raise ValueError(f'unknown sector padding policy: {policy}')
    prefix = b'CPE\x01' + b'\x08\x00' + b'\x03\x90\x00'
    return (prefix + bytes(size))[:size]


def serialize(linked: ELFFile, image: RetailImage, *,
              padding_policy: str = 'zero') -> tuple[bytes, list[dict]]:
    """Write full linked load sections, a PS-X header and explicit sector padding."""
    if (linked.elfclass != 32 or not linked.little_endian
            or linked['e_machine'] != 'EM_MIPS' or linked['e_type'] != 'ET_EXEC'):
        raise ValueError('expected a linked little-endian MIPS ELF32 executable')
    sections = []
    for section in linked.iter_sections():
        if section['sh_flags'] & 2 and section['sh_size']:
            if section.name not in RUNTIME or section['sh_type'] not in {'SHT_PROGBITS', 'SHT_NOBITS'}:
                raise ValueError(f'unexpected linked runtime section {section.name}')
            start, size = section['sh_addr'], section['sh_size']
            if start < image.load_start or start + size > 0x80200000:
                raise ValueError(f'linked allocation outside PS1 RAM: {section.name}')
            sections.append({'name': section.name, 'address': start, 'size': size,
                             'storage': 'bss' if section['sh_type'] == 'SHT_NOBITS' else 'load'})
    ordered = sorted(sections, key=lambda s: s['address'])
    for left, right in zip(ordered, ordered[1:]):
        if left['address'] + left['size'] > right['address']:
            raise ValueError('overlapping linked allocations')
    loaded = [s for s in sections if s['storage'] == 'load']
    if not loaded:
        raise ValueError('empty linked load image')
    if not any(s['name'] == '.text' and s['address'] <= linked['e_entry']
               < s['address'] + s['size'] for s in loaded):
        raise ValueError('entry is outside linked code')
    extent = max(s['address'] + s['size'] for s in loaded) - image.load_start
    padding = sector_padding(extent, padding_policy)
    size = extent + len(padding)
    payload = bytearray(extent) + padding
    for section in loaded:
        offset = section['address'] - image.load_start
        payload[offset:offset + section['size']] = linked.get_section_by_name(section['name']).data()
    # Preserve original loader/region metadata, but regenerate the linked entry
    # and load extent. Template bytes are explicitly reported as retail input.
    header = bytearray(image.data[:0x800])
    struct.pack_into('<I', header, 0x10, linked['e_entry'])
    struct.pack_into('<II', header, 0x18, image.load_start, size)
    return bytes(header + payload), sections


def compare(actual: bytes, image: RetailImage) -> dict:
    expected = image.data
    common = min(len(actual), len(expected))
    differences = [i for i in range(common) if actual[i] != expected[i]]
    differences.extend(range(common, max(len(actual), len(expected))))
    first = differences[0] if differences else None
    first_load = next((i for i in differences if i >= 0x800), None)
    return {'file_equal': actual == expected, 'retail_size': len(expected), 'linked_size': len(actual),
            'retail_sha256': hashlib.sha256(expected).hexdigest(),
            'linked_sha256': hashlib.sha256(actual).hexdigest(),
            'differing_bytes': len(differences), 'first_difference_file_offset': first,
            'first_difference_va': image.load_start + first - 0x800
            if first is not None and first >= 0x800 else None,
            'first_load_difference_file_offset': first_load,
            'first_load_difference_va': image.load_start + first_load - 0x800
            if first_load is not None else None,
            'first_load_difference_retail': expected[first_load:first_load+16].hex()
            if first_load is not None else None,
            'first_load_difference_linked': actual[first_load:first_load+16].hex()
            if first_load is not None else None,
            'header_differing_bytes': sum(i < 0x800 for i in differences),
            'load_differing_bytes': sum(i >= 0x800 for i in differences)}


def symbol_placements(linked: ELFFile, units: list[Unit]) -> list[dict]:
    table = linked.get_section_by_name('.symtab')
    rows = []
    for unit in units:
        for item, kind in [*((f, 'function') for f in unit.functions),
                           *((d, 'data') for d in unit.data)]:
            symbols = table.get_symbol_by_name(item.symbol) or []
            if len(symbols) == 1:
                symbol = symbols[0]
                rows.append({'unit': unit.unit, 'symbol': item.symbol, 'kind': kind,
                             'retail_va': item.va, 'linked_va': symbol['st_value'],
                             'address_delta': symbol['st_value'] - item.va,
                             'retail_size': item.body_size if kind == 'function' else item.size,
                             'linked_size': symbol['st_size']})
            else:
                rows.append({'unit': unit.unit, 'symbol': item.symbol, 'kind': kind,
                             'issue': 'missing-or-ambiguous-linked-symbol', 'count': len(symbols)})
    return rows


def link_image(image: RetailImage, sdk: list[Path], sdk_root: Path, library_report: dict) -> dict:
    manifest = load_manifest()
    units = [u for u in manifest.units if u.image == image.image and u.scope != 'vendored']
    key = image.image[:-4].lower()
    root = BUILD / 'link' / key
    root.mkdir(parents=True, exist_ok=True)
    candidate = root / image.image
    output = root / 'linked.elf'
    # Failed retries must not leave an old executable looking current.
    candidate.unlink(missing_ok=True)
    output.unlink(missing_ok=True)
    objects = [BUILD / 'objdiff' / key / 'base' / u.object_name for u in units]
    report = {'image': image.image, 'source_objects': [str(p) for p in objects],
              'sdk_archives': [str(p) for p in sdk], 'retail_startup': [],
              'sdk_library_report': str(sdk_root / 'libraries.json'),
              'inventory_data': [], 'sdk_aliases': [], 'attempts': [],
              'header_provider': 'retail-template-with-linked-entry-and-load-size',
              'padding_provider': 'inferred-cpe-v1-prefix-at-linked-load-end',
              'historical_converter_reproduced': False,
              'complete_source_reconstruction_proven': False, 'game_execution_tested': False}
    entry = '__SN_ENTRY_POINT'
    if key == 'psx':
        ordered = [sdk_root / library / (member + '.o') for library, member in PSX_SDK_ORDER]
        objects.extend(ordered)
        report['sdk_explicit_order'] = [f'{library}.LIB/{member}.OBJ'
                                        for library, member in PSX_SDK_ORDER]
    if key != 'psx':
        catalog = load_catalog(RETAIL_CONFIG)
        for function in catalog.functions[image.image]:
            if function.symbol in {'__main', 'start'} and function.provider:
                path = BUILD / 'delink' / key / 'objects' / f'{function.va:08x}_{function.symbol}.o'
                objects.append(path)
                report['retail_startup'].append({'symbol': function.symbol, 'va': function.va,
                                                'size': function.size, 'object': str(path)})
        entry = 'start'
    definitions = []
    for attempt in range(2):
        (root / 'link.ld').write_text(script(image, units, objects, definitions))
        command = ['mipsel-linux-gnu-ld', '-EL', '-G0', '--entry', entry,
                   '-T', str(root / 'link.ld'), '-Map', str(root / 'link.map'), '-o', str(output),
                   *(str(p) for p in objects), '--start-group', *(str(p) for p in sdk), '--end-group']
        process = subprocess.run(command, capture_output=True, text=True, timeout=60)
        log = root / f'link-{attempt}.log'
        log.write_text(process.stdout + process.stderr)
        missing = undefined(process.stderr)
        report['attempts'].append({'returncode': process.returncode, 'undefined': missing,
                                   'log': str(log), 'command': command})
        if process.returncode == 0:
            linked = elf(output)
            entries = linked.get_section_by_name('.symtab').get_symbol_by_name(entry) or []
            if len(entries) != 1 or entries[0]['st_value'] != linked['e_entry']:
                raise ValueError('linker did not resolve the requested entry symbol')
            actual, sections = serialize(linked, image, padding_policy='cpe-v1-prefix')
            load_end = max(s['address'] + s['size'] for s in sections if s['storage'] == 'load')
            report['container_padding'] = {
                'address': load_end, 'size': len(actual) - 0x800 - (load_end - image.load_start),
                'policy': 'cpe-v1-prefix', 'mechanism_evidence': 'candidate',
            }
            report['sdk_verification'] = verify_linked(linked, library_report)
            candidate.write_bytes(actual)
            report.update(executable=str(candidate), elf=str(output), sections=sections,
                          comparison=compare(actual, image),
                          symbol_placements=symbol_placements(linked, units))
            break
        if attempt or not missing:
            break
        alias_lines, aliases = sdk_aliases(image, missing, sdk_root)
        supplied, data_lines, data_rows = data_inputs(image, missing, units, objects, root)
        definitions += alias_lines + data_lines
        report['sdk_aliases'] = aliases
        report['inventory_data'] = data_rows
        if supplied:
            objects.append(supplied)
    report['linked'] = candidate.is_file()
    report['input_sha256'] = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in objects}
    (root / 'comparison.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--image', choices=('psx', 'game', 'open'), action='append')
    args = parser.parse_args(argv)
    images = [key.upper() + '.EXE' for key in args.image] if args.image else list(IMAGE_LAYOUTS)
    try:
        from scripts.kf.graph import configure_if_needed, run_ninja

        for name in images:
            root = BUILD / 'link' / name[:-4].lower()
            root.mkdir(parents=True, exist_ok=True)
            for filename in (name, 'linked.elf', 'comparison.json'):
                (root / filename).unlink(missing_ok=True)
        (BUILD / 'link' / 'comparison.json').unlink(missing_ok=True)
        # Comparison gates remain separate: a source/data mismatch must not
        # prevent generating the very executable needed to diagnose it.
        configure_if_needed()
        if run_ninja([f'{phase}-{name[:-4].lower()}' for name in images for phase in ('base', 'target')]):
            return 1
        verified = [RetailImage.load(name) for name in images]
        sdk_root = BUILD / 'link' / 'libraries'
        sdk, library_report = archives(sdk_root)
        reports = []
        for image in verified:
            try:
                report = link_image(image, sdk, sdk_root, library_report)
            except (OSError, ValueError, subprocess.SubprocessError) as error:
                report = {'image': image.image, 'linked': False, 'error': str(error)}
                root = BUILD / 'link' / image.image[:-4].lower()
                (root / image.image).unlink(missing_ok=True)
                (root / 'comparison.json').write_text(json.dumps(report, indent=2) + '\n')
            reports.append(report)
            if report['linked']:
                diff = report['comparison']
                print(f'{image.image}: linked {diff["linked_size"]} bytes; '
                      f'{diff["differing_bytes"]} file bytes differ from retail')
            elif report.get('attempts'):
                last = report['attempts'][-1]
                print(f'{image.image}: link failed; {len(last["undefined"])} unresolved symbols; '
                      f'see {last["log"]}')
            else:
                print(f'{image.image}: {report["error"]}')
        (BUILD / 'link' / 'comparison.json').write_text(json.dumps(reports, indent=2) + '\n')
        return int(any(not report['linked'] for report in reports))
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    raise SystemExit(main())
