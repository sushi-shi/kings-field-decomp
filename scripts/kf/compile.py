"""Compile one reconstructed source into its per-image objdiff base object."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
from pathlib import Path

from scripts.kf.delink import image_key
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv


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


def _target_names(delink_dir: Path, image: str) -> set[str]:
    manifest = delink_dir / image_key(image) / "objects.tsv"
    if not manifest.is_file():
        raise ValueError(f"{manifest}: run kf-delink first")
    _, rows = read_tsv(manifest)
    return {Path(row["object"]).name for row in rows}


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
) -> Path:
    source = source.resolve()
    if not source.is_file():
        raise ValueError(f"{source}: source is missing")
    object_name = output.name
    if object_name not in _target_names(delink_dir, image):
        raise ValueError(
            f"{object_name}: no carved target with this filename for {image}"
        )
    output.parent.mkdir(parents=True, exist_ok=True)
    suffix = source.suffix.lower()
    metadata: dict[str, object] = {
        "image": image,
        "source": os.path.relpath(source, Path.cwd()),
        "output": os.path.relpath(output, Path.cwd()),
    }

    if suffix in {".s", ".asm"}:
        assembler = _tool("mipsel-linux-gnu-as")
        _run([
            assembler,
            "-march=r3000",
            "-mabi=32",
            "-G0",
            "-o",
            str(output),
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
        preprocessor = _tool("cpppsx-260")
        compiler = _tool("cc1psx-260")
        maspsx = _tool("maspsx")
        intermediate = output.parent / ".intermediates"
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
        cpp_arguments.append(str(source))
        preprocessed.write_bytes(_run(cpp_arguments))

        compiler_arguments = [
            compiler,
            "-quiet",
            f"-{optimization}",
            f"-G{small_data}",
            *cc1_flags,
            str(preprocessed),
            "-o",
            str(assembly),
        ]
        _run(compiler_arguments)
        assembler_arguments = [
            maspsx,
            f"--aspsx-version={aspsx_version}",
            "--run-assembler",
            "--force-stdin",
            "-march=r3000",
            "-mabi=32",
            "-o",
            str(output),
        ]
        _run(assembler_arguments, input_data=assembly.read_bytes())
        metadata.update({
            "language": "c",
            "compiler": "Decompals old-gcc 0.17 GCC 2.6.0 PSX rebuild",
            "optimization": optimization,
            "small_data": small_data,
            "cc1_flags": list(cc1_flags),
            "assembler_model": f"maspsx ASPSX {aspsx_version} -> GNU mipsel as",
            "attribution": "candidate probe; exact retail compiler/profile unproven",
        })
    else:
        raise ValueError(f"unsupported source suffix {source.suffix!r}")

    metadata_path = output.with_suffix(output.suffix + ".json")
    metadata_path.write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
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
    )
    print(result)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
