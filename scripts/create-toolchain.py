#!/usr/bin/env python3
"""Build the hash-pinned King's Field Psy-Q candidate toolchain.

The exact compiler/linker tuple used for King's Field is not proven.  This
builder therefore preserves the complete Release 2.5 host-tool installation,
both compiler frontends found beside it (GCC 2.4.1 and GCC 2.6.0), and the
independent GNU C 2.60 disk as separate, named candidates.

Entry point:

    nix develop

`flake.nix` supplies the source media and invokes this verifier/stager directly.
"""

from __future__ import annotations

import argparse
import gzip
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path


PROJECT_DIR = Path(
    os.environ.get("KINGS_FIELD_DIR", Path(__file__).resolve().parent.parent)
).resolve()

# King's Field (Japan) release date, used to normalize staged metadata.
RELEASE_EPOCH = 787536000


@dataclass(frozen=True)
class Medium:
    env: str
    filename: str
    sha256: str
    url: str


MEDIA = {
    "release-2.5": Medium(
        env="PSYQ_FLOPPIES_RAR",
        filename="Floppies.rar",
        sha256="49a2f3cebca3a8421c94f1de43b9d20e73750b042b0831dd2b1732d28f947783",
        url="https://archive.org/download/ps1_sdks/Floppies.rar",
    ),
    "gcc-2.60-disk-1": Medium(
        env="PSYQ_GCC260_IMG",
        filename="GNU C Compiler Version 2.60 (World) (Disk 1).img",
        sha256="07af9fecd148cd423411ceb9127ed3f3e9149a1151a1a254df3e2b8f0c33bae7",
        url=(
            "https://archive.org/download/ps1_sdks/"
            "GNU%20C%20Compiler%20Version%202.60%20%28World%29%20%28Disk%201%29.img"
        ),
    ),
}

# These gates bind the staged candidate to the exact files used by the
# attribution investigation.  They are not claims that King's Field used every
# file, nor that the surrounding Release 2.5 distribution is the exact SDK.
EXPECTED_RELEASE25_FILES = {
    "isa board/PSXBIN/BIN/CCPSX.EXE": (
        "50a85bebf4bb2e1672ce5fb33d1943e35657fc76ad093e03310e048adc12065b"
    ),
    "isa board/PSXBIN/BIN/ASPSX.EXE": (
        "06abb0eb4a1483c30a28c0ef89d8dd52585229963875d4b9cb19c1cb202e955b"
    ),
    "isa board/PSXBIN/BIN/PSYLINK.EXE": (
        "b98e7180fc33b4c94adf41dcf9cc73f469864d23aabb3f3fcedf5e9f2c1ca040"
    ),
    "isa board/PSXBIN/BIN/PSYLIB.EXE": (
        "fdae3dea7cfd0891a4862e845398a7b028a51cede0d0f0f852d269e8a877bd98"
    ),
    "isa board/PSXBIN/BIN/ASMPSX.EXE": (
        "ccd1e6d80c5115a4472d82e142c2ea98726712a798a93c3756bc8a238111027f"
    ),
    "compiler/CC1PSX": (
        "d164b281bd6c815abb06a38ca21f1a4f75d5c6a63c2b3a41128def58afee9548"
    ),
    "compiler/CPPPSX": (
        "38687a01d58dcd3c373d7487e50d1c634cd3cf6c3137f3aa92304aa05bdf17e1"
    ),
    "compiler/CC1PSX.EXE": (
        "e65635ec539f2b9c6db6f2b350c1fd300a26fdd0a264b3f2defefa9c2d0d6dd3"
    ),
    "compiler/CPPPSX.EXE": (
        "97b42264dffafcf15ca3e6348d49a537f13d958d2d66292eeb1a1c155fe0c217"
    ),
    "isa board/PSXLIB/LIB/LIBCD.LIB": (
        "d05f3b1d730f4b8b63ba7231abaa7fb4289121902581f8649d411cdcbd854de3"
    ),
    "isa board/PSXLIB/LIB/LIBSPU.LIB": (
        "24a6abb704a07f9e97f9c5c227c118d0beff642924edcbef43ff23bc565864d6"
    ),
    "isa board/PSXLIB/LIB/LIBSND.LIB": (
        "8b012e133b62047a3223888201b968423bb7c5868707da2c3d41529e8fea2c9f"
    ),
    "isa board/PSXLIB/LIB/LIBGTE.LIB": (
        "3e0fbd64b24c6b12133708f032a45c4a3888dc04bd6ff433ea9421e0cce2d790"
    ),
    "isa board/PSXLIB/LIB/LIBGPU.LIB": (
        "66c22a78268ba8cb72a7e7703dafa07b44aa686e9708df00e9f6d3f8a9e52a01"
    ),
    "isa board/PSXLIB/LIB/LIBSN.LIB": (
        "2f85afd0ea6dbd55c889ceda2cf6bd821ddad06246c20d7a55902d4dad7d791d"
    ),
}

EXPECTED_GCC260_DISK_FILES = {
    "CC1PSX": "6587ed37f9f8795f8e47601d3a71167e93c832959f2f314e08d11c53be04661e",
    "CPPPSX": "69e0b24ef219119200298ed450d5c8342529ee153f498a25c2166fea663c20d1",
}

TOOL_VERSION_MARKERS = {
    "CCPSX.EXE": "CCPSX version 1.02",
    # Spelling/case follows the strings embedded in the original binaries.
    "PSYLINK.EXE": "PSYLINK version 1.17",
    "PSYLIB.EXE": "PsyLib version 1.04",
    "ASMPSX.EXE": "Psy-Q PSX version 1.07",
}


def log(message: str) -> None:
    print(f"[toolchain] {message}", flush=True)


def run(command: list[str], **kwargs: object) -> subprocess.CompletedProcess[bytes]:
    log("+ " + " ".join(command))
    return subprocess.run(command, check=True, **kwargs)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def verify_hash(path: Path, expected: str, label: str) -> None:
    actual = sha256_file(path)
    if actual != expected:
        raise RuntimeError(
            f"{label} hash mismatch: expected {expected}, got {actual} ({path})"
        )


def resolve_casefold(root: Path, relative: str) -> Path:
    """Resolve an archive member without assuming host case semantics."""

    current = root
    for part in Path(relative).parts:
        if not current.is_dir():
            raise FileNotFoundError(relative)
        candidates = [
            child for child in current.iterdir() if child.name.casefold() == part.casefold()
        ]
        if len(candidates) != 1:
            raise FileNotFoundError(
                f"expected one case-insensitive match for {part!r} below {current}, "
                f"found {len(candidates)}"
            )
        current = candidates[0]
    return current


def find_release_root(extracted: Path) -> Path:
    """Find the directory containing both `compiler` and `isa board`."""

    candidates = [extracted, *sorted(p for p in extracted.rglob("*") if p.is_dir())]
    for candidate in candidates:
        children = {p.name.casefold() for p in candidate.iterdir()} if candidate.is_dir() else set()
        if "compiler" in children and "isa board" in children:
            return candidate
    raise RuntimeError("Release 2.5 root (compiler + isa board) was not found")


def extract_archive(source: Path, destination: Path) -> None:
    destination.mkdir(parents=True, exist_ok=True)
    if source.suffix.casefold() == ".rar":
        run(
            [
                "unar",
                "-quiet",
                "-force-overwrite",
                "-output-directory",
                str(destination),
                str(source),
            ]
        )
        return
    run(
        ["7z", "x", "-y", f"-o{destination}", str(source)],
        stdout=subprocess.DEVNULL,
    )


def extract_gzip(source: Path, destination: Path) -> None:
    with gzip.open(source, "rb") as compressed, destination.open("wb") as output:
        shutil.copyfileobj(compressed, output)


def copy_tree(source: Path, destination: Path) -> None:
    if not source.is_dir():
        raise RuntimeError(f"required directory is absent: {source}")
    shutil.copytree(source, destination, dirs_exist_ok=True)


def copy_file(source: Path, destination: Path) -> None:
    if not source.is_file():
        raise RuntimeError(f"required file is absent: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source, destination)


def verify_release25(root: Path) -> None:
    for relative, digest in EXPECTED_RELEASE25_FILES.items():
        verify_hash(resolve_casefold(root, relative), digest, relative)

    bin_dir = resolve_casefold(root, "isa board/PSXBIN/BIN")
    for filename, marker in TOOL_VERSION_MARKERS.items():
        data = resolve_casefold(bin_dir, filename).read_bytes()
        if marker.encode("ascii") not in data:
            raise RuntimeError(f"{filename} does not contain version marker {marker!r}")

    for relative, marker in (
        ("compiler/CC1PSX", b"2.4.1"),
        ("compiler/CPPPSX", b"2.4.1"),
        ("compiler/CC1PSX.EXE", b"2.6.0"),
        ("compiler/CPPPSX.EXE", b"2.6.0"),
    ):
        if marker not in resolve_casefold(root, relative).read_bytes():
            raise RuntimeError(f"{relative} does not contain compiler marker {marker!r}")


def stage_release25(root: Path, stage: Path) -> None:
    log("staging the Release 2.5 Psy-Q host tools, headers, and libraries")
    copy_tree(resolve_casefold(root, "isa board/PSXBIN/BIN"), stage / "psyq" / "bin")
    copy_tree(resolve_casefold(root, "isa board/PSXLIB/INCLUDE"), stage / "psyq" / "include")
    copy_tree(resolve_casefold(root, "isa board/PSXLIB/LIB"), stage / "psyq" / "lib")

    compiler = resolve_casefold(root, "compiler")
    copy_file(
        resolve_casefold(compiler, "CC1PSX"),
        stage / "compilers" / "gcc-2.4.1" / "CC1PSX",
    )
    copy_file(
        resolve_casefold(compiler, "CPPPSX"),
        stage / "compilers" / "gcc-2.4.1" / "CPPPSX",
    )
    copy_file(
        resolve_casefold(compiler, "CC1PSX.EXE"),
        stage / "compilers" / "gcc-2.6.0-release-2.5" / "CC1PSX.EXE",
    )
    copy_file(
        resolve_casefold(compiler, "CPPPSX.EXE"),
        stage / "compilers" / "gcc-2.6.0-release-2.5" / "CPPPSX.EXE",
    )

    docs_dir = stage / "compilers" / "documentation"
    for name in (
        "COPYING.txt",
        "CPP.TEX",
        "EXTEND.TEX",
        "GCC.TEX",
        "INVOKE.TEX",
        "Install.txt",
    ):
        candidate = resolve_casefold(compiler, name)
        copy_file(candidate, docs_dir / candidate.name)


def stage_gcc260_disk(extracted: Path, stage: Path) -> None:
    log("decompressing and staging the independent GNU C 2.60 disk")
    destination = stage / "compilers" / "gcc-2.6.0-disk-1"
    destination.mkdir(parents=True, exist_ok=True)

    for compressed_name, output_name in (
        ("CC1PSX.EXZ", "CC1PSX"),
        ("CPPPSX.EXZ", "CPPPSX"),
    ):
        source = resolve_casefold(extracted, compressed_name)
        output = destination / output_name
        extract_gzip(source, output)
        verify_hash(output, EXPECTED_GCC260_DISK_FILES[output_name], output_name)
        if b"2.6.0" not in output.read_bytes():
            raise RuntimeError(f"{output_name} from compiler disk lacks GCC 2.6.0 marker")

    for name in ("COPYING", "README.TXT"):
        source = resolve_casefold(extracted, name)
        copy_file(source, destination / source.name)


def write_attribution(stage: Path) -> None:
    text = """# King's Field Psy-Q candidate toolchain

This directory is a reproducible research input, not a final compiler
attribution.  Relocation-aware library matching identifies the contemporary
Sony Psy-Q family strongly, but the exact compiler, assembler, linker build,
and optimization flags used for King's Field remain unresolved.

Preserved candidates:

- `psyq/`: host tools, headers, and libraries from the 1994-12-29 Release 2.5
  floppy collection. Embedded strings identify CCPSX 1.02, PSYLINK 1.17,
  PSYLIB 1.04, and ASMPSX 1.07. ASPSX is present but key-protected and does not
  expose a version through the same static string gate.
- `compilers/gcc-2.4.1/`: extensionless GCC 2.4.1 frontend pair from that
  collection.
- `compilers/gcc-2.6.0-release-2.5/`: DOS/COFF GCC 2.6.0 frontend pair found
  in the same collection.
- `compilers/gcc-2.6.0-disk-1/`: independently extracted GCC 2.6.0 frontend
  pair from the GNU C Compiler 2.60 disk. These files are not byte-identical to
  the Release 2.5 pair and are intentionally kept separate.

Do not collapse those alternatives until controlled compile/link probes match
the retail executables. The original binaries remain proprietary; this output
is generated from hash-pinned media and must not be committed to Git.
"""
    (stage / "ATTRIBUTION.md").write_text(text, encoding="utf-8", newline="\n")

    rows = ["id\tenvironment\tsha256\turl"]
    for identifier, medium in MEDIA.items():
        rows.append(f"{identifier}\t{medium.env}\t{medium.sha256}\t{medium.url}")
    (stage / "SOURCE_MEDIA.tsv").write_text("\n".join(rows) + "\n", encoding="utf-8")


def normalize_stage(stage: Path) -> None:
    for path in sorted(stage.rglob("*")):
        if path.is_symlink():
            raise RuntimeError(f"staged toolchain must not contain symlinks: {path}")
        mode = 0o755 if path.is_dir() else 0o644
        path.chmod(mode)
        os.utime(path, (RELEASE_EPOCH, RELEASE_EPOCH), follow_symlinks=False)
    stage.chmod(0o755)
    os.utime(stage, (RELEASE_EPOCH, RELEASE_EPOCH), follow_symlinks=False)


def write_manifest(stage: Path) -> None:
    manifest = stage / "MANIFEST.tsv"
    rows = ["path\tbytes\tsha256"]
    for path in sorted(p for p in stage.rglob("*") if p.is_file() and p != manifest):
        relative = path.relative_to(stage).as_posix()
        rows.append(f"{relative}\t{path.stat().st_size}\t{sha256_file(path)}")
    manifest.write_text("\n".join(rows) + "\n", encoding="utf-8", newline="\n")


def medium_from_environment(medium: Medium) -> Path:
    value = os.environ.get(medium.env)
    if not value:
        raise RuntimeError(
            f"{medium.env} is unset; use `nix develop` or set the path to the "
            "pinned source medium"
        )
    path = Path(value)
    if not path.is_file():
        raise RuntimeError(f"{medium.env} does not name a file: {path}")
    verify_hash(path, medium.sha256, medium.filename)
    return path


def build(stage: Path, work: Path) -> None:
    floppies = medium_from_environment(MEDIA["release-2.5"])
    gcc260 = medium_from_environment(MEDIA["gcc-2.60-disk-1"])

    release_extract = work / "release-2.5"
    disk_extract = work / "gcc-2.60-disk-1"
    extract_archive(floppies, release_extract)
    extract_archive(gcc260, disk_extract)

    release_root = find_release_root(release_extract)
    verify_release25(release_root)
    stage_release25(release_root, stage)
    stage_gcc260_disk(disk_extract, stage)
    write_attribution(stage)
    write_manifest(stage)
    normalize_stage(stage)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--stage-dir",
        type=Path,
        default=PROJECT_DIR / "build" / "toolchain",
        help="stage directly here (default: build/toolchain)",
    )
    parser.add_argument("--work-dir", type=Path, help="temporary extraction directory")
    parser.add_argument("--keep-work", action="store_true", help="retain temporary extraction files")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    owned_work = args.work_dir is None
    work = (
        Path(tempfile.mkdtemp(prefix="kings-field-toolchain-"))
        if owned_work
        else args.work_dir.resolve()
    )
    stage = args.stage_dir.resolve()

    if stage.exists() and any(stage.iterdir()):
        raise RuntimeError(f"refusing to merge into non-empty stage directory: {stage}")
    stage.mkdir(parents=True, exist_ok=True)
    work.mkdir(parents=True, exist_ok=True)

    log(f"work:  {work}")
    log(f"stage: {stage}")
    try:
        build(stage, work)
        log("toolchain initialization complete")
    finally:
        if owned_work and not args.keep_work:
            shutil.rmtree(work, ignore_errors=True)
        elif args.keep_work:
            log(f"kept work directory: {work}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
