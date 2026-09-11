"""Compile through the native SDK and derive a read-only ELF comparison view."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import re
import subprocess
import struct
from pathlib import Path

from scripts.kf.delink import image_key
from scripts.kf.model import scan_data_claims
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv


# Native probe compilers: tool basenames and provenance labels. The labels are
# recorded in every .o.json so a base object always names the probe that made
# it; none of them is a historical attribution claim.
C_COMPILERS = {
    "gcc260-native": (
        "cpppsx-260",
        "cc1psx-260",
        "Decompals old-gcc 0.17 GCC 2.6.0 PSX rebuild",
    ),
    "gcc257-native": (
        "cpppsx-257",
        "cc1psx-257",
        "Decompals old-gcc 0.17 GCC 2.5.7 PSX rebuild",
    ),
}

def _tool(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        raise RuntimeError(f"{name} not found; enter nix develop")
    return path


def _run(
    arguments: list[str], *, input_data: bytes | None = None,
    trace_environment: dict[str, str] | None = None,
) -> bytes:
    # Trace only the explicitly selected compiler invocation. Inherited trace
    # settings must not contaminate normal builds or the auxiliary sizeof TU.
    environment = {key: value for key, value in os.environ.items()
                   if not key.startswith("KF_GCC257_TRACE")
                   and key != "KF_GCC257_SOURCE_SHA256"}
    environment.update(trace_environment or {})
    process = subprocess.run(
        arguments,
        input=input_data,
        capture_output=True,
        check=False,
        env=environment,
    )
    if process.returncode:
        details = (process.stderr or process.stdout).decode(errors="replace")
        raise RuntimeError(f"command failed ({process.returncode}): {details}")
    return process.stdout


def _write_bytes_if_changed(path: Path, content: bytes) -> bool:
    if path.is_file() and path.read_bytes() == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    try:
        temporary.write_bytes(content)
        temporary.replace(path)
    finally:
        temporary.unlink(missing_ok=True)
    return True


def _write_text_if_changed(path: Path, content: str) -> bool:
    return _write_bytes_if_changed(path, content.encode("utf-8"))


def _validate_mips_elf(data: bytes, output: Path) -> None:
    if len(data) < 52 or data[:7] != b"\x7fELF\x01\x01\x01":
        raise RuntimeError(f"compiler produced an incomplete/non-ELF object for {output}")
    machine = struct.unpack_from("<H", data, 18)[0]
    if machine != 8:
        raise RuntimeError(f"compiler produced ELF machine {machine}, expected MIPS (8)")


def _target_names(delink_dir: Path, image: str) -> set[str]:
    manifest = delink_dir / image_key(image) / "objects.tsv"
    if not manifest.is_file():
        raise ValueError(f"{manifest}: run kf-delink first")
    _, rows = read_tsv(manifest)
    return {
        Path(row["object"]).name
        for row in rows
        if row.get("scope", "decomp") in {"decomp", "module"}
    }


def _source_data_sizes(preprocessed: Path, names: tuple[str, ...],
                       compiler_arguments: list[str], scratch: Path,
                       small_data: int) -> dict[str, int]:
    """Query target sizeof through the same compiler and native assembler.

    The auxiliary object is inspection evidence only. It never replaces or
    augments the actual object passed to PSYLINK.
    """
    from scripts.kf.lnk import read
    from scripts.kf.sdk import assemble_many

    content = preprocessed.read_bytes()
    marker = '__kf_source_data_sizes'
    while marker.encode() in content:
        marker += '_'
    expression = ', '.join(f'sizeof({name})' for name in names)
    declaration = (f'\nconst unsigned long {marker}[] = '
                   f'{{0x4b46535aUL, sizeof(unsigned long), {expression}}};\n')
    source, assembly = scratch / 'SIZES.I', scratch / 'SIZES.S'
    source.write_bytes(content + declaration.encode())
    _run([*compiler_arguments, str(source), '-o', str(assembly)])
    obj = read(assemble_many(scratch, ('UNIT', 'SIZES'), small_data)['SIZES'])
    symbols = [symbol for symbol in obj.symbols.values() if symbol.name == marker]
    if len(symbols) != 1 or symbols[0].section is None or symbols[0].common:
        raise RuntimeError('source-size probe did not define its constant table')
    symbol = symbols[0]
    section = obj.sections[symbol.section]
    start, size = symbol.value, 4 * (len(names) + 2)
    if section.name not in ('.rdata', '.data', '.sdata') or start + size > len(section.data):
        raise RuntimeError('source-size probe table is not initialized')
    if any(start <= offset < start + size for offset, _, _ in section.patches):
        raise RuntimeError('source-size probe table is not constant')
    values = struct.unpack_from(f'<{len(names) + 2}I', section.data, start)
    if values[:2] != (0x4B46535A, 4):
        raise RuntimeError('source-size probe does not use target 32-bit words')
    return dict(zip(names, values[2:], strict=True))


def compile_source(
    source: Path, image: str, output: Path,
    delink_dir: Path = Path("build/delink"), optimization: str | None = None,
    small_data: int = 0, aspsx_version: str = "1.07",
    include_dirs: tuple[Path, ...] = (), cc1_flags: tuple[str, ...] = (),
    compiler: str = "gcc260-native",
    defines: tuple[str, ...] = (), *, cc1_override: Path | None = None,
    trace_path: Path | None = None, trace_function: str | None = None,
) -> Path:
    from scripts.kf.lnk import elf_view
    from scripts.kf.sdk import assemble, compile_c

    source, output = source.resolve(), output.resolve()
    if not source.is_file():
        raise ValueError(f"{source}: source is missing")
    if aspsx_version != '1.07':
        raise ValueError('the executable chain uses pinned ASPSX 1.07')
    if trace_path is not None and (cc1_override is None or compiler != 'gcc257-native'):
        raise ValueError('tracing requires an explicit GCC 2.5.7 cc1 override')
    if trace_path is not None and source.suffix.lower() != '.c':
        raise ValueError('compiler tracing requires a C source')
    if trace_function is not None and trace_path is None:
        raise ValueError('trace_function requires trace_path')
    if output.name not in _target_names(delink_dir, image):
        raise ValueError(f'{output.name}: no non-vendored target with this filename for {image}')
    output.parent.mkdir(parents=True, exist_ok=True)
    scratch = output.parent / '.tmp' / str(os.getpid()) / output.stem
    scratch.mkdir(parents=True, exist_ok=True)
    trace_environment = None
    if trace_path is not None:
        if source.suffix.lower() != '.c':
            raise ValueError('compiler tracing requires a C source')
        trace_path = trace_path.resolve()
        trace_path.parent.mkdir(parents=True, exist_ok=True)
        if trace_path.exists():
            raise ValueError(f'refusing to overwrite compiler trace: {trace_path}')
        trace_environment = {'KF_GCC257_TRACE': str(trace_path),
                             'KF_GCC257_SOURCE_SHA256': hashlib.sha256(source.read_bytes()).hexdigest()}
        if trace_function is not None:
            trace_environment['KF_GCC257_TRACE_FUNCTION'] = trace_function
    data_claims = scan_data_claims(source) if source.suffix.lower() == '.c' else ()
    source_sizes = {}
    if source.suffix.lower() == '.c':
        if optimization is None:
            raise ValueError('C compilation requires --optimization; retail profile remains unproven')
        result = compile_c(source, scratch, 'UNIT', compiler=compiler, optimization=optimization,
                           small_data=small_data, include_dirs=include_dirs, cc1_flags=cc1_flags,
                           defines=defines, cc1_override=cc1_override,
                           trace_environment=trace_environment)
        if trace_path is not None and (not trace_path.is_file() or not trace_path.stat().st_size):
            raise RuntimeError('instrumented compiler produced no trace')
        if data_claims:
            source_sizes = _source_data_sizes(
                scratch / 'UNIT.I', tuple(claim.name for claim in data_claims),
                result['compiler_command'][:-3], scratch, small_data)
    elif source.suffix.lower() in ('.s', '.asm'):
        shutil.copyfile(source, scratch / 'UNIT.S')
        result = {}
    else:
        raise ValueError(f'unsupported source suffix {source.suffix!r}')
    native = ((scratch / 'UNIT.OBJ').read_bytes() if data_claims
              else assemble(scratch, 'UNIT', small_data))
    assembly = (scratch / 'UNIT.S').read_text()
    functions = tuple(re.findall(r'^\s*\.ent\s+([\w$]+)', assembly, re.M))
    view = elf_view(native, functions=functions, sizes=source_sizes)
    _validate_mips_elf(view, output)
    _write_bytes_if_changed(output.with_suffix('.OBJ'), native)
    _write_bytes_if_changed(output.with_suffix('.S'), (scratch / 'UNIT.S').read_bytes())
    _write_bytes_if_changed(output, view)
    metadata = {
        **result, 'image': image, 'source': os.path.relpath(source, Path.cwd()),
        'output': os.path.relpath(output, Path.cwd()),
        'language': 'c' if source.suffix.lower() == '.c' else 'assembly',
        'compiler': C_COMPILERS[compiler][2], 'compiler_probe': compiler,
        'optimization': optimization, 'small_data': small_data, 'cc1_flags': list(cc1_flags),
        'assembler_model': 'native ASPSX 1.07; LNK object -> ELF inspection view',
        'native_object': str(output.with_suffix('.OBJ')),
        'native_object_sha256': hashlib.sha256(native).hexdigest(),
        'assembler_sha256': hashlib.sha256(Path(os.environ['PSYQ_ASPSX']).read_bytes()).hexdigest(),
        'attribution': 'candidate probe; exact retail compiler/profile unproven',
        'data_claims': [{'name': claim.name, 'va': f'{claim.va:#x}', 'size': claim.size}
                        for claim in data_claims],
        'data_symbol_sizes': {'method': 'pinned-compiler-sizeof-probe', 'sizes': source_sizes},
        **({'cc1_override': str(cc1_override.resolve())} if cc1_override else {}),
        **({'trace': str(trace_path), 'source_sha256': trace_environment['KF_GCC257_SOURCE_SHA256'],
            'preprocessed_sha256': hashlib.sha256((scratch / 'UNIT.I').read_bytes()).hexdigest()}
           if trace_path is not None else {}),
    }
    _write_text_if_changed(output.with_suffix(output.suffix + '.json'), json.dumps(metadata, indent=2) + '\n')
    shutil.rmtree(scratch.parent, ignore_errors=True)
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", required=True, choices=tuple(IMAGE_LAYOUTS))
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--delink-dir", type=Path, default=Path("build/delink"))
    parser.add_argument(
        "--optimization", choices=("O0", "O1", "O2", "O3")
    )
    parser.add_argument("--small-data", type=int, default=0)
    parser.add_argument("--aspsx-version", default="1.07")
    parser.add_argument("--include", action="append", type=Path, default=[])
    parser.add_argument("--cc1-flag", action="append", default=[])
    parser.add_argument(
        "--compiler", choices=tuple(C_COMPILERS), default="gcc260-native"
    )
    parser.add_argument("--define", action="append", default=[])
    args = parser.parse_args()

    key = image_key(args.image)
    output = args.output or (
        Path("build/objdiff") / key / "base" / f"{args.source.stem}.o"
    )
    include_dirs = list(args.include)
    for project_include in (Path("include"), Path("vendor/include")):
        if project_include.is_dir():
            include_dirs.append(project_include)
    psyq_include = os.environ.get("PSYQ_INCLUDE")
    if psyq_include:
        include_dirs.append(Path(psyq_include))
    result = compile_source(
        args.source,
        args.image,
        output,
        args.delink_dir,
        args.optimization,
        args.small_data,
        args.aspsx_version,
        tuple(include_dirs),
        tuple(args.cc1_flag),
        args.compiler,
        defines=tuple(args.define),
    )
    print(result)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
