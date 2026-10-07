"""Link PlayStation programs with the Psy-Q SDK."""

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

from .sdk import dos_run, tool_succeeded
from .sdk_compat import library_input


# Ordinary linker inputs. The native linker selects members from these archives.
LIBRARIES = {
    'PSX.EXE': ('LIBSN', 'LIBAPI'),
    'GAME.EXE': ('LIBSN', 'LIBCD', 'LIBSND', 'LIBSPU', 'LIBGTE', 'LIBGPU',
                 'LIBETC', 'LIBAPI', 'LIBPRESS'),
    'OPEN.EXE': ('LIBSN', 'LIBCD', 'LIBSND', 'LIBSPU', 'LIBGTE', 'LIBGPU',
                 'LIBETC', 'LIBAPI', 'LIBPRESS'),
}
ENTRY = '__SN_ENTRY_POINT'
OVERLAY_STARTUP = 'NONE2.OBJ'
# LIBAPI A74/A75/A76/A69: identical BIOS B0 selectors 4a/4b/4c/45.
GAME_SDK_ALIASES = {'InitCARD2': 'InitCARD', 'StartCARD2': 'StartCARD',
                    'StopCARD2': 'StopCARD', 'erase': 'delete'}
# Overlay main compiles its .bss start and initial heap start as numbers, as
# retail did (docs/patterns/startup-address-origins.md). Values that this
# link's layout makes stale are refreshed from its map and the users relinked.
STARTUP_LAYOUTS = {
    'GAME.EXE': ('kf/game/startup_layout.h', 'GAME_BSS_START', 'GAME_HEAP_START'),
    'OPEN.EXE': ('kf/open/startup_layout.h', 'OPEN_BSS_START', 'OPEN_HEAP_START'),
}


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def map_sections(path: Path) -> dict[str, tuple[int, int]]:
    """Read the start and exclusive end of each section in a PSYLINK map."""
    sections = {}
    for line in path.read_text(errors='replace').splitlines():
        fields = line.split()
        if len(fields) != 4 or not fields[3].startswith('.'):
            continue
        try:
            start, _, length = (int(field, 16) for field in fields[:3])
        except ValueError:
            continue
        sections[fields[3]] = (start, start + length)
    return sections


def header_values(path: Path, names: tuple[str, ...]) -> dict[str, int]:
    text = path.read_text()
    values = {}
    for name in names:
        found = re.findall(rf'(?m)^#define\s+{name}\s+(0x[0-9A-Fa-f]+)u?\s*$', text)
        if len(found) != 1:
            raise ValueError(f'{path}: expected one hexadecimal {name} definition')
        values[name] = int(found[0], 16)
    return values


def linked_startup_layout(sections: dict[str, tuple[int, int]], heap: int) -> tuple[int, int]:
    """Return this link's .bss start and a heap start above all static storage."""
    end = max(stop for _, stop in sections.values())
    start, _ = sections.get('.bss', (end, end))
    return start, max(heap, (end + 7) & ~7)


def build_image(name, root, units, compile_one, *, repo, load_address) -> dict:
    """Compile and link a program using its source units and SDK libraries."""
    root = root.resolve()
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    report = {'image': name, 'linked': False, 'phase': 'compile', 'units': [],
              'libraries': [], 'startup': None, 'tools': {}}

    def assemble(entries) -> None:
        commands = [entry['assembler_command'] for entry in entries
                    if 'assembler_command' in entry]
        if commands:
            dos_run(root, commands, 'asm')
        for entry in entries:
            if 'assembler_command' in entry:
                tool_succeeded(root, entry['log'], entry['object'], b'LNK\x02')
            elif not (root / entry['object']).read_bytes().startswith(b'LNK\x02'):
                raise ValueError('compiler produced no native object')
            entry['object_sha256'] = file_hash(root / entry['object'])

    def link() -> None:
        dos_run(root, [report['linker_command']], 'link')
        tool_succeeded(root, 'LINK.TXT', stem + '.CPE', b'CPE\x01')

    try:
        tools = {'ASPSX.EXE': Path(os.environ['PSYQ_ASPSX']),
                 **{tool: Path(os.environ['PSYQ_BIN']) / tool
                    for tool in ('PSYLINK.EXE', 'CPE2X.EXE')}}
        for tool, source in tools.items():
            shutil.copyfile(source, root / tool)
            report['tools'][tool] = {'path': str(source), 'sha256': file_hash(source)}
        if not units:
            raise ValueError(f'{name}: no C source units')
        for index, unit in enumerate(units):
            report['units'].append(compile_one(unit, root, index))
        if name != 'PSX.EXE':
            source = Path(os.environ['PSYQ_H2000_LIB']) / OVERLAY_STARTUP
            shutil.copyfile(source, root / OVERLAY_STARTUP)
            report['startup'] = {
                'file': OVERLAY_STARTUP,
                'path': str(source),
                'sha256': file_hash(source),
                'provenance': 'exact Release 2.5 H2000/LIB2000 object',
            }
        report['phase'] = 'assemble'
        assemble(report['units'])
        if report['startup'] and not (root / OVERLAY_STARTUP).read_bytes().startswith(b'LNK\x02'):
            raise ValueError(f'{OVERLAY_STARTUP}: expected a Psy-Q LNK object')
        for library in LIBRARIES[name]:
            filename = library + '.LIB'
            source = Path(os.environ['PSYQ_LIB']) / filename
            data, corrections = library_input(filename, source.read_bytes())
            (root / filename).write_bytes(data)
            report['libraries'].append({
                'file': filename, 'path': str(source), 'sha256': file_hash(source),
                'link_input_sha256': hashlib.sha256(data).hexdigest(),
                'corrections': corrections,
            })
        report['phase'] = 'link'
        stem = name.removesuffix('.EXE')
        commands = [f'\torg ${load_address:08x}',
                    *(f'\tinclude "{u["object"]}"' for u in report['units']),
                    *([f'\tinclude "{OVERLAY_STARTUP}"'] if report['startup'] else []),
                    *(f'\tinclib "{library["file"]}"' for library in report['libraries']),
                    *(f'{alias} alias {symbol}' for alias, symbol in
                      (GAME_SDK_ALIASES.items() if name == 'GAME.EXE' else ())),
                    'bssdata group bss', '\tsection .sbss,bssdata', '\tsection .bss,bssdata',
                    f'\tregs pc={ENTRY}']
        (root / 'LINK.LNK').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
        report['linker_command'] = f'psylink /c @LINK.LNK,{stem}.CPE,{stem}.SYM,{stem}.MAP > LINK.TXT'
        link()
        if name in STARTUP_LAYOUTS:
            report['phase'] = 'startup-layout'
            header, *names = STARTUP_LAYOUTS[name]
            users = [index for index, entry in enumerate(report['units'])
                     if 'preprocessor_command' not in entry
                     or header in (root / entry['object']).with_suffix('.I').read_text(errors='replace')]
            if users:
                source = repo / 'include' / header
                values = header_values(source, tuple(names))
                sections = map_sections(root / f'{stem}.MAP')
                linked = dict(zip(names, linked_startup_layout(sections, values[names[1]]),
                                  strict=True))
                report['startup_layout'] = {
                    'header': str(source.relative_to(repo)), 'source_sha256': file_hash(source),
                    'source_values': {key: f'{value:#010x}' for key, value in values.items()},
                    'linked_values': {key: f'{value:#010x}' for key, value in linked.items()},
                    'refreshed_units': []}
                if linked != values:
                    override = root / 'LAYOUT'
                    guard = re.sub(r'\W', '_', header.upper())
                    generated = override / header
                    generated.parent.mkdir(parents=True)
                    generated.write_text(
                        f'/* Generated from {stem}.MAP for this link. */\n'
                        f'#ifndef {guard}\n#define {guard}\n'
                        + ''.join(f'#define {key} {value:#010x}u\n' for key, value in linked.items())
                        + '#endif\n')
                    for index in users:
                        report['units'][index] = compile_one(units[index], root, index, override)
                    assemble([report['units'][index] for index in users])
                    link()
                    if dict(zip(names, linked_startup_layout(map_sections(root / f'{stem}.MAP'),
                                                             values[names[1]]), strict=True)) != linked:
                        raise ValueError(f'{name}: startup layout moved after refreshing {header}')
                    report['startup_layout']['refreshed_units'] = users
        report['phase'] = 'convert'
        report['converter_command'] = f'cpe2x {stem}.CPE > CONVERT.TXT'
        dos_run(root, [report['converter_command']], 'convert')
        executable = root / name
        if not executable.is_file():
            raise RuntimeError('CPE2X produced no executable; see CONVERT.TXT')
        actual = executable.read_bytes()
        if len(actual) < 2048 or actual[:8] != b'PS-X EXE':
            raise ValueError('CPE2X produced an invalid executable')
        report.update(linked=True, phase='complete', executable=str(executable),
                      cpe=str(root / (stem + '.CPE')))
    except (KeyError, OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        (root / name).unlink(missing_ok=True)
        report['error'] = str(error)
    (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
    return report
