"""Compile one reconstructed source into its per-image objdiff base object."""

from __future__ import annotations

import argparse
import hashlib
import io
import json
import os
import shutil
import subprocess
import struct
from pathlib import Path

from elftools.elf.elffile import ELFFile

from scripts.kf.allocation_notes import assembly_note, compiler_requests
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

# Emit the assembly's complete explicit extents, not GNU-as end-of-section
# padding. This does not lower alignment requirements or remove explicit zeros,
# alignment directives, COMMON allocation rounding, or MIPS delay slots.
GNU_AS_SECTION_FLAGS = ("-no-pad-sections",)


def aspsx_section_alignment(data: bytes, version: str) -> tuple[bytes, dict]:
    """Represent ASPSX's section declarations in the GNU analysis container.

    ASPSX 1.07 emits LNK tag 8 (four-byte alignment) for all six standard
    sections. Its .align directives align offsets *within* the section; they
    do not raise the declaration's linker constraint. Native ASPSX/PSYLINK
    controls cover both facts. GNU's minimum/maximum ELF conventions differ.
    Change only sh_addralign, never bytes, extents, symbols or relocations.
    No source claim, retail address or comparison score enters this conversion.
    """
    if version != "1.07":
        raise ValueError("section alignment is calibrated only for ASPSX 1.07")
    elf = ELFFile(io.BytesIO(data))
    if (elf.elfclass != 32 or not elf.little_endian
            or elf['e_machine'] != 'EM_MIPS' or elf['e_type'] != 'ET_REL'
            or elf['e_shentsize'] != 40):
        raise ValueError("ASPSX section conversion requires ELF32-LE MIPS relocatable input")
    standard = {'.text', '.data', '.rodata', '.sdata', '.bss', '.sbss'}
    output = bytearray(data)
    sections = {}
    for index, section in enumerate(elf.iter_sections()):
        if not section['sh_flags'] & 2 or section.name in {'.reginfo', '.MIPS.abiflags'}:
            continue
        if section.name not in standard:
            raise ValueError(f"uncalibrated ASPSX allocated section {section.name}")
        sections[section.name] = {
            'gnu_alignment': section['sh_addralign'], 'lnk_alignment_tag': 8,
            'alignment': 4,
        }
        struct.pack_into('<I', output, elf['e_shoff'] + index * 40 + 32, 4)
    return bytes(output), {
        'method': 'aspsx-1.07-section-declarations', 'sections': sections,
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


def _source_data_sizes(
    preprocessed: Path,
    names: tuple[str, ...],
    compiler_arguments: list[str],
    assembler_arguments: list[str],
    scratch: Path,
) -> dict[str, int]:
    """Ask the same target compiler for C object sizes, without retail sizes.

    The auxiliary TU contains the identical preprocessed source plus a constant
    sizeof table. Only that table is read; this object is never a reconstruction
    input. The real object is assembled from the original, unaugmented TU.
    In particular, COMMON reservation rounding is not sizeof(the C object).
    """
    content = preprocessed.read_bytes()
    marker = '__kf_source_data_sizes'
    while marker.encode() in content:
        marker += '_'
    probe_source = scratch / 'data-sizes.i'
    probe_assembly = scratch / 'data-sizes.s'
    probe_object = scratch / 'data-sizes.o'
    expression = ', '.join(f'sizeof({name})' for name in names)
    declaration = (f'\nconst unsigned long {marker}[] = '
                   f'{{0x4b46535aUL, sizeof(unsigned long), {expression}}};\n')
    probe_source.write_bytes(content + declaration.encode())
    _run([*compiler_arguments, str(probe_source), '-o', str(probe_assembly)])
    _run([*assembler_arguments, '-o', str(probe_object)],
         input_data=probe_assembly.read_bytes())
    _validate_mips_elf(probe_object.read_bytes(), probe_object)
    with probe_object.open('rb') as stream:
        elf = ELFFile(stream)
        symbols = elf.get_section_by_name('.symtab').get_symbol_by_name(marker)
        if len(symbols or ()) != 1 or not isinstance(symbols[0]['st_shndx'], int):
            raise RuntimeError('source-size probe did not define its constant table')
        symbol = symbols[0]
        section_index = symbol['st_shndx']
        section = elf.get_section(section_index)
        start, size = symbol['st_value'], 4 * (len(names) + 2)
        if section['sh_type'] != 'SHT_PROGBITS' or start + size > section['sh_size']:
            raise RuntimeError('source-size probe table is incomplete or not initialized')
        for relocations in elf.iter_sections():
            if (relocations['sh_type'] in ('SHT_REL', 'SHT_RELA')
                    and relocations['sh_info'] == section_index
                    and any(start <= row['r_offset'] < start + size
                            for row in relocations.iter_relocations())):
                raise RuntimeError('source-size probe table is not constant')
        values = struct.unpack_from(f'<{len(names) + 2}I', section.data(), start)
        if values[:2] != (0x4B46535A, 4):
            raise RuntimeError('source-size probe does not use target 32-bit words')
    return dict(zip(names, values[2:], strict=True))


def compile_source(
    source: Path,
    image: str,
    output: Path,
    delink_dir: Path = Path("build/delink"),
    optimization: str | None = None,
    small_data: int = 0,
    aspsx_version: str = "1.07",
    include_dirs: tuple[Path, ...] = (),
    cc1_flags: tuple[str, ...] = (),
    compiler: str = "gcc260-native",
    maspsx_flags: tuple[str, ...] = (),
    defines: tuple[str, ...] = (),
    *,
    cc1_override: Path | None = None,
    trace_path: Path | None = None,
    trace_function: str | None = None,
) -> Path:
    source = source.resolve()
    if not source.is_file():
        raise ValueError(f"{source}: source is missing")
    if trace_path is not None and (cc1_override is None or compiler != "gcc257-native"):
        raise ValueError("tracing requires an explicit GCC 2.5.7 cc1 override")
    if trace_path is not None and source.suffix.lower() != ".c":
        raise ValueError("compiler tracing requires a C source")
    if trace_function is not None and trace_path is None:
        raise ValueError("trace_function requires trace_path")
    object_name = output.name
    if object_name not in _target_names(delink_dir, image):
        raise ValueError(
            f"{object_name}: no non-vendored target with this filename for {image}"
        )
    output.parent.mkdir(parents=True, exist_ok=True)
    scratch = output.parent / ".tmp" / str(os.getpid()) / output.stem
    scratch.mkdir(parents=True, exist_ok=True)
    staged = scratch / output.name
    suffix = source.suffix.lower()
    metadata: dict[str, object] = {
        "image": image,
        "source": os.path.relpath(source, Path.cwd()),
        "output": os.path.relpath(output, Path.cwd()),
        "gnu_as_section_flags": list(GNU_AS_SECTION_FLAGS),
    }

    if suffix in {".s", ".asm"}:
        assembler = _tool("mipsel-linux-gnu-as")
        _run([
            assembler,
            "-march=r3000",
            "-mabi=32",
            "-G0",
            *GNU_AS_SECTION_FLAGS,
            "-o",
            str(staged),
            str(source),
        ])
        metadata.update({
            "language": "assembly",
            "assembler": "GNU mipsel as; MIPS-I/O32/G0",
        })
    elif suffix == ".c":
        if optimization is None:
            raise ValueError(
                "C compilation requires --optimization because the retail profile "
                "is not yet proven"
            )
        if compiler not in C_COMPILERS:
            raise ValueError(f"unknown C compiler probe {compiler!r}")
        cpp_name, cc1_name, compiler_label = C_COMPILERS[compiler]
        preprocessor = _tool(cpp_name)
        cc1 = _tool(str(cc1_override)) if cc1_override is not None else _tool(cc1_name)
        maspsx = _tool("maspsx")
        intermediate = scratch / "intermediates"
        intermediate.mkdir(parents=True, exist_ok=True)
        preprocessed = intermediate / f"{source.stem}.i"
        assembly = intermediate / f"{source.stem}.s"

        cpp_arguments = [
            preprocessor,
            "-lang-c",
            "-undef",
            "-nostdinc",
        ]
        for directory in include_dirs:
            cpp_arguments.extend(("-I", str(directory.resolve())))
        for define in defines:
            cpp_arguments.append(f"-D{define}")
        cpp_arguments.append(str(source))
        preprocessed.write_bytes(_run(cpp_arguments))

        compiler_arguments = [
            cc1,
            "-quiet",
            f"-{optimization}",
            f"-G{small_data}",
            *cc1_flags,
        ]
        assembler_arguments = [
            maspsx,
            f"--aspsx-version={aspsx_version}",
            f"-G{small_data}",
            *maspsx_flags,
            "--run-assembler",
            "--force-stdin",
            "-march=r3000",
            "-mabi=32",
            *GNU_AS_SECTION_FLAGS,
        ]
        trace_environment = None
        if trace_path is not None:
            trace_path = trace_path.resolve()
            trace_path.parent.mkdir(parents=True, exist_ok=True)
            if trace_path.exists():
                raise ValueError(f"refusing to overwrite compiler trace: {trace_path}")
            trace_environment = {
                "KF_GCC257_TRACE": str(trace_path),
                "KF_GCC257_SOURCE_SHA256": hashlib.sha256(source.read_bytes()).hexdigest(),
            }
            if trace_function is not None:
                trace_environment["KF_GCC257_TRACE_FUNCTION"] = trace_function
        _run([*compiler_arguments, str(preprocessed), '-o', str(assembly)],
             trace_environment=trace_environment)
        exported_requests = compiler_requests(assembly.read_text())
        if trace_path is not None and (not trace_path.is_file() or not trace_path.stat().st_size):
            raise RuntimeError("instrumented compiler produced no trace")
        data_claims = scan_data_claims(source)
        source_sizes: dict[str, int] = {}
        if data_claims:
            # GCC 2.5.7 prints no `.size` for data, so objdiff would infer the
            # last datum's size from the remaining section. Retail DATA sizes
            # must not be copied here: they are the expectation, not source
            # evidence. Query sizeof with this compiler in a separate object.
            source_sizes = _source_data_sizes(
                preprocessed, tuple(claim.name for claim in data_claims),
                compiler_arguments, assembler_arguments, intermediate)
            with assembly.open("a", encoding="utf-8") as stream:
                for claim in data_claims:
                    stream.write(f"\t.type\t{claim.name},@object\n")
                    stream.write(f"\t.size\t{claim.name},{source_sizes[claim.name]}\n")
        # maspsx currently flattens COMMON into BSS. Preserve the native request
        # class/extent separately so placement cannot mistake it for fixed BSS.
        with assembly.open('a', encoding='utf-8') as stream:
            stream.write(assembly_note(exported_requests))
        _run([*assembler_arguments, '-o', str(staged)], input_data=assembly.read_bytes())
        metadata.update({
            "language": "c",
            "compiler": compiler_label,
            "compiler_probe": compiler,
            **({"cc1_override": str(cc1_override.resolve())}
               if cc1_override is not None else {}),
            **({"trace": str(trace_path),
                "source_sha256": trace_environment["KF_GCC257_SOURCE_SHA256"],
                "preprocessed_sha256": hashlib.sha256(preprocessed.read_bytes()).hexdigest()}
               if trace_path is not None else {}),
            "optimization": optimization,
            "small_data": small_data,
            "cc1_flags": list(cc1_flags),
            "maspsx_flags": list(maspsx_flags),
            "data_claims": [
                {"name": claim.name, "va": f"{claim.va:#x}", "size": claim.size}
                for claim in data_claims
            ],
            "data_symbol_sizes": {
                "method": "pinned-compiler-sizeof-probe",
                "sizes": source_sizes,
            },
            "assembler_model": f"maspsx ASPSX {aspsx_version} -> GNU mipsel as",
            "attribution": "candidate probe; exact retail compiler/profile unproven",
        })
    else:
        raise ValueError(f"unsupported source suffix {source.suffix!r}")

    object_data = staged.read_bytes()
    if suffix == '.c':
        object_data, metadata['section_alignment'] = aspsx_section_alignment(
            object_data, aspsx_version)
    _validate_mips_elf(object_data, output)
    _write_bytes_if_changed(output, object_data)
    metadata_path = output.with_suffix(output.suffix + ".json")
    _write_text_if_changed(metadata_path, json.dumps(metadata, indent=2) + "\n")
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
    parser.add_argument("--maspsx-flag", action="append", default=[])
    parser.add_argument("--define", action="append", default=[])
    args = parser.parse_args()

    key = image_key(args.image)
    output = args.output or (
        Path("build/objdiff") / key / "base" / f"{args.source.stem}.o"
    )
    include_dirs = list(args.include)
    project_include = Path("include")
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
        tuple(args.maspsx_flag),
        defines=tuple(args.define),
    )
    print(result)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
