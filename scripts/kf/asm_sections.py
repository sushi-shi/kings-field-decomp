"""Assemble compiler directives without GNU ELF's special-section minimums."""

from __future__ import annotations

import io
from pathlib import Path
import re
import subprocess

from elftools.elf.elffile import ELFFile


SPECIAL = {'.text': ('ax', 'progbits'), '.data': ('aw', 'progbits')}
PREFIX = '.kf_compiler'


def neutral_sections(assembly: str) -> tuple[str, tuple[str, ...]]:
    """Only change section spelling/flags; leave every directive in place.

    GAS gives ordinary .text/.data a sixteen-byte ELF minimum even when
    the compiler asks for .align 2. Neutral names let GAS measure the actual
    requirements, including implicit word alignment and larger .align values.
    """
    used = {'.text'}

    def directive(name: str) -> str:
        flags, kind = SPECIAL[name]
        used.add(name)
        return f'.section {PREFIX}{name},"{flags}",@{kind}'

    # GCC's initial compiler-marker labels belong to the initial text section.
    lines = [directive('.text')]
    for line in assembly.splitlines():
        if PREFIX in line:
            raise ValueError('compiler assembly uses reserved section prefix')
        code, separator, comment = line.partition('#')
        match = re.fullmatch(r'\s*(?:\.section\s+)?(\.(?:text|data))\s*', code)
        if match:
            line = directive(match[1]) + (f' #{comment}' if separator else '')
        elif re.match(r'\s*(?:\.(?:text|data)\b|\.section\s+\.(?:text|data)\b)', code):
            raise ValueError(f'unsupported compiler section directive: {line}')
        elif re.match(r'\s*\.(?:pushsection|popsection|previous|subsection)\b', code):
            raise ValueError(f'unsupported compiler section traversal: {line}')
        lines.append(line)
    return '\n'.join(lines) + '\n', tuple(name for name in SPECIAL if name in used)


def assemble(assembly: str, output: Path) -> dict[str, dict[str, int]]:
    """Emit native ELF constraints from assembly, never from retail addresses."""
    mapped, sections = neutral_sections(assembly)
    source = output.with_suffix('.sections.s')
    raw = output.with_suffix('.sections.o')
    source.write_text(mapped)
    subprocess.run(['mipsel-linux-gnu-as', '-march=r3000', '-mabi=32', '-G0',
                    '-no-pad-sections', '-o', str(raw), str(source)],
                   capture_output=True, check=True)
    original = ELFFile(io.BytesIO(raw.read_bytes()))
    args = []
    retained = []
    for name in (*SPECIAL, '.bss'):
        default = original.get_section_by_name(name)
        if default is not None and default['sh_size']:
            if name != '.bss':
                raise ValueError(f'unexpected allocation in GNU default {name}')
            # maspsx's COMMON expansion relies on an implicit BSS alignment.
            # Preserve that separate allocation contract; PSX has no such C
            # allocations. A zero-length default contributes no reservation.
            retained.append(name)
            continue
        args += ['--remove-section', name]
    for name in sections:
        args += ['--rename-section', f'{PREFIX}{name}={name}']
    subprocess.run(['mipsel-linux-gnu-objcopy', *args, str(raw), str(output)],
                   capture_output=True, check=True)
    final = ELFFile(io.BytesIO(output.read_bytes()))
    result = {}
    for name in (*sections, *retained):
        before = original.get_section_by_name(PREFIX + name if name in sections else name)
        after = final.get_section_by_name(name)
        fields = ('sh_size', 'sh_addralign', 'sh_type', 'sh_flags')
        if (after is None or any(before[field] != after[field] for field in fields)
                or before.data() != after.data()):
            raise ValueError(f'section renaming changed {name}')
        result[name] = {'size': after['sh_size'], 'alignment': after['sh_addralign']}
    return result
