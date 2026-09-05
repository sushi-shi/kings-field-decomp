"""Compile one reconstructed source into its per-image objdiff base object."""

from __future__ import annotations

import argparse
import json
import os
import shutil
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

# Emit the assembly's complete explicit extents, not GNU-as end-of-section
# padding. This does not lower alignment requirements or remove explicit zeros,
# alignment directives, COMMON allocation rounding, or MIPS delay slots.
GNU_AS_SECTION_FLAGS = ("-no-pad-sections",)


def _tool(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        raise RuntimeError(f"{name} not found; enter nix develop")
    return path


def _run(arguments: list[str], *, input_data: bytes | None = None) -> bytes:
    process = subprocess.run(
        arguments,
        input=input_data,
        capture_output=True,
        check=False,
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
) -> Path:
    source = source.resolve()
    if not source.is_file():
        raise ValueError(f"{source}: source is missing")
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
        cc1 = _tool(cc1_name)
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
            str(preprocessed),
            "-o",
            str(assembly),
        ]
        _run(compiler_arguments)
        data_claims = scan_data_claims(source)
        if data_claims:
            # GCC 2.5.7 prints no `.size` for data, so objdiff would infer the
            # last claimed datum's extent from the remaining section bytes.
            # The claim states the curated size; annotating the symbol with it
            # only fixes the comparison extent and never changes code or bytes.
            with assembly.open("a", encoding="utf-8") as stream:
                for claim in data_claims:
                    stream.write(f"\t.type\t{claim.name},@object\n")
                    stream.write(f"\t.size\t{claim.name},{claim.size}\n")
        assembler_arguments = [
            maspsx,
            f"--aspsx-version={aspsx_version}",
            *maspsx_flags,
            "--run-assembler",
            "--force-stdin",
            "-march=r3000",
            "-mabi=32",
            *GNU_AS_SECTION_FLAGS,
            "-o",
            str(staged),
        ]
        _run(assembler_arguments, input_data=assembly.read_bytes())
        metadata.update({
            "language": "c",
            "compiler": compiler_label,
            "compiler_probe": compiler,
            "optimization": optimization,
            "small_data": small_data,
            "cc1_flags": list(cc1_flags),
            "maspsx_flags": list(maspsx_flags),
            "data_claims": [
                {"name": claim.name, "va": f"{claim.va:#x}", "size": claim.size}
                for claim in data_claims
            ],
            "assembler_model": f"maspsx ASPSX {aspsx_version} -> GNU mipsel as",
            "attribution": "candidate probe; exact retail compiler/profile unproven",
        })
    else:
        raise ValueError(f"unsupported source suffix {source.suffix!r}")

    object_data = staged.read_bytes()
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
