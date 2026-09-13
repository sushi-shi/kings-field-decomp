"""Link PlayStation programs with the Psy-Q SDK."""

import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

from .sdk import dos_run, dos_text, tool_succeeded


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


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def build_image(name, root, units, compile_one, *, repo, load_address, bounds_source) -> dict:
    """Compile and link a program using its source units and SDK libraries."""
    root = root.resolve()
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    report = {'image': name, 'linked': False, 'phase': 'compile', 'units': [],
              'libraries': [], 'startup': None, 'tools': {}}
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
        assembler_commands = [u['assembler_command'] for u in report['units']]
        if name != 'PSX.EXE':
            assembler = Path(os.environ['PSYQ_ASMPSX'])
            shutil.copyfile(assembler, root / 'ASMPSX.EXE')
            report['tools']['ASMPSX.EXE'] = {
                'path': str(assembler), 'sha256': file_hash(assembler)}
            source = repo / bounds_source
            shutil.copyfile(source, root / 'BOUNDS.ASM')
            dos_text(root / 'BOUNDS.ASM')
            report['boundaries'] = {
                'source': str(source.relative_to(repo)), 'source_sha256': file_hash(source),
                'object': 'BOUNDS.OBJ',
                'assembler_command': 'asmpsx /l /oc+ BOUNDS.ASM,BOUNDS.OBJ > BOUNDS.TXT'}
            assembler_commands.append(report['boundaries']['assembler_command'])
            source = Path(os.environ['PSYQ_H2000_LIB']) / OVERLAY_STARTUP
            shutil.copyfile(source, root / OVERLAY_STARTUP)
            report['startup'] = {
                'file': OVERLAY_STARTUP,
                'path': str(source),
                'sha256': file_hash(source),
                'provenance': 'exact Release 2.5 H2000/LIB2000 object',
            }
        report['phase'] = 'assemble'
        dos_run(root, assembler_commands, 'asm')
        for unit in report['units']:
            tool_succeeded(root, unit['log'], unit['object'], b'LNK\x02')
            unit['object_sha256'] = file_hash(root / unit['object'])
        if report['startup'] and not (root / OVERLAY_STARTUP).read_bytes().startswith(b'LNK\x02'):
            raise ValueError(f'{OVERLAY_STARTUP}: expected a Psy-Q LNK object')
        if report['startup']:
            tool_succeeded(root, 'BOUNDS.TXT', 'BOUNDS.OBJ', b'LNK\x02')
            report['boundaries']['object_sha256'] = file_hash(root / 'BOUNDS.OBJ')
        for library in LIBRARIES[name]:
            filename = library + '.LIB'
            source = Path(os.environ['PSYQ_LIB']) / filename
            shutil.copyfile(source, root / filename)
            report['libraries'].append({'file': filename, 'path': str(source),
                                        'sha256': file_hash(source)})
        report['phase'] = 'link'
        stem = name.removesuffix('.EXE')
        commands = [f'\torg ${load_address:08x}',
                    *(f'\tinclude "{u["object"]}"' for u in report['units']),
                    *([f'\tinclude "{OVERLAY_STARTUP}"'] if report['startup'] else []),
                    *(['\tinclude "BOUNDS.OBJ"'] if report['startup'] else []),
                    *(f'\tinclib "{library["file"]}"' for library in report['libraries']),
                    *(f'{alias} alias {symbol}' for alias, symbol in
                      (GAME_SDK_ALIASES.items() if name == 'GAME.EXE' else ())),
                    'bssdata group bss', '\tsection .sbss,bssdata',
                    *(['\tsection .bss_start,bssdata'] if report['startup'] else []),
                    '\tsection .bss,bssdata',
                    *(['\tsection .bss_end,bssdata'] if report['startup'] else []),
                    f'\tregs pc={ENTRY}']
        (root / 'LINK.LNK').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
        report['linker_command'] = f'psylink /c @LINK.LNK,{stem}.CPE,{stem}.SYM,{stem}.MAP > LINK.TXT'
        dos_run(root, [report['linker_command']], 'link')
        tool_succeeded(root, 'LINK.TXT', stem + '.CPE', b'CPE\x01')
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
