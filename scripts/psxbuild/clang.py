"""Compile C++ game code for the PlayStation with Clang and native SDK libraries."""

from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import subprocess
import re

from .elf_to_lnk import convert


def compile_program(units, root, index, *, repo):
    sdk = root / 'sdk'
    sdk.mkdir()
    for filename, symbol in (('LIBGPU.H', 'SetDrawOffset'), ('LIBCD.H', 'StFreeRing')):
        header = sdk / filename
        text = (Path(os.environ['PSYQ_INCLUDE']) / filename).read_text(encoding='latin-1')
        text, count = re.subn(r'(?m)^\s*\w+\s+' + symbol + r'\(\);\s*$', '', text)
        if count != 1:
            raise ValueError(f'{filename}: expected one obsolete {symbol} declaration')
        header.write_text(text, encoding='latin-1')

    def compile_one(pair):
        index, unit = pair
        output = root / f'cpp{index:03d}.o'
        command = ['clang', '--target=mipsel-none-elf', '-march=mips1', '-mabi=32',
                   '-msoft-float', '-mno-abicalls', '-mno-check-zero-division',
                   '-fno-pic', '-G0', '-std=c++20',
                   '-O2', '-ffreestanding', '-fno-builtin', '-fno-exceptions', '-fno-rtti',
                   '-fno-threadsafe-statics', '-fno-use-cxa-atexit', '-fno-strict-aliasing',
                   '-fwrapv', '-nostdinc', '-I', str(repo / 'include'),
                   '-I', str(repo / 'vendor/include'), '-isystem', str(sdk),
                   '-isystem', os.environ['PSYQ_INCLUDE'],
                   *(f'-D{d}' for d in unit['options']['defines']),
                   '-c', str(repo / unit['source']), '-o', str(output)]
        result = subprocess.run(command, text=True, capture_output=True)
        (root / f'cpp{index:03d}.log').write_text(result.stdout + result.stderr)
        if result.returncode:
            raise RuntimeError(f'{unit["source"]}: {result.stderr}')
        return output

    with ThreadPoolExecutor(max_workers=4) as pool:
        objects = list(pool.map(compile_one, enumerate(units)))
    script = root / 'merge.ld'
    script.write_text('''SECTIONS {
 .text : { *(.text .text.*) }
 .rodata : { *(.rodata .rodata.*) }
 .data : { *(.data .data.*) }
 .sdata : { *(.sdata .sdata.*) }
 .sbss : { *(.sbss .sbss.*) }
 .bss : { *(.bss .bss.*) *(COMMON) }
 /DISCARD/ : { *(.reginfo .MIPS.abiflags .pdr .comment .note.GNU-stack .llvm_addrsig) }
}
''')
    merged = root / 'program.o'
    command = [os.environ.get('MIPS_LD', 'mipsel-linux-gnu-ld'), '-r', '-d', '-T', str(script),
               '-o', str(merged), *map(str, objects)]
    subprocess.run(command, check=True)
    (root / 'PROGRAM.OBJ').write_bytes(convert(merged.read_bytes()))
    return {'object': 'PROGRAM.OBJ', 'compiler': 'clang', 'sources': len(units)}
