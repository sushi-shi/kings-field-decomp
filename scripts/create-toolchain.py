                      
"""Stage and verify the hash-pinned Psy-Q Release 2.5 SDK media."""

from __future__ import annotations

import argparse
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
}

                                                                
                                                                              
                                                                           
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
    "isa board/PSXLIB/LIB/CARD.OBJ": (
        "4ab0873cddbf26d99aded93f8b654c861f44409cee408b3bff7026fc59665387"
    ),
    "isa board/PSXLIB/LIB/LIBAPI.LIB": (
        "1f4afdb983d445d53c3632b882165e9c85e0db372734e19c263ee9ee7dd92dfb"
    ),
    "isa board/PSXLIB/LIB/LIBETC.LIB": (
        "571cfbbc00c34e3f1f0fd19eb54ba88ae1a15d1ba3bcb0d30e9d0fcfe42bc7be"
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
    "isa board/PSXLIB/LIB/LIBGS.LIB": (
        "e9d5f75c5e638300a513fabc6f55e36536f8e810b9e8312d429265eb709e13d7"
    ),
    "isa board/PSXLIB/LIB/LIBSN.LIB": (
        "2f85afd0ea6dbd55c889ceda2cf6bd821ddad06246c20d7a55902d4dad7d791d"
    ),
    "isa board/PSXLIB/LIB/LIBPRESS.LIB": (
        "1440e132a157faa009e0b2d62d9ef85929550c0700dd8184492ca5eec336fad7"
    ),
    "isa board/PSXLIB/LIB/MALLOC.OBJ": (
        "628e405fd0e3acfff2ce9d4a15d481f0aa36398c14e9eae0a82b7ff0a86a74c9"
    ),
    "H2000/LIB2000/2MBYTE.OBJ": (
        "b4ed8b2e76bc841f7a81132906d846877aefa8f6233fb738600c8b1ebd032a46"
    ),
    "H2000/LIB2000/8MBYTE.OBJ": (
        "0c0ccb91c6d059a88b5ec5ac8f34c65a717c636a911247c31faa0df4ae2e599e"
    ),
    "H2000/LIB2000/LIBAPI.LIB": (
        "4bc72902080ef75ffbbbe35820d89c0ea721e7d4d8187276bffd43ff341a3f5f"
    ),
    "H2000/LIB2000/NONE2.OBJ": (
        "38662381b57cdbebb8d083842fd23914bad1d4c66303beb871f2f5248c38774c"
    ),
    "demo/CARD/CARD.OBJ": (
        "4ab0873cddbf26d99aded93f8b654c861f44409cee408b3bff7026fc59665387"
    ),
}

TOOL_VERSION_MARKERS = {
    "CCPSX.EXE": "CCPSX version 1.02",
                                                                          
    "PSYLINK.EXE": "PSYLINK version 1.17",
    "PSYLIB.EXE": "PsyLib version 1.04",
    "ASMPSX.EXE": "Psy-Q PSX version 1.07",
}


def log(message: str) -> None:
    print(f"[sdk] {message}", flush=True)


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


def copy_tree(source: Path, destination: Path) -> None:
    if not source.is_dir():
        raise RuntimeError(f"required directory is absent: {source}")
    shutil.copytree(source, destination, dirs_exist_ok=True)


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
    log("staging the complete Release 2.5 SDK media tree")
    copy_tree(root, stage / "release-2.5")


def write_attribution(stage: Path) -> None:
    text = """# Psy-Q Release 2.5 SDK baseline

`release-2.5/` is the complete tree extracted from the one hash-pinned Release
2.5 floppy collection. It is preserved as a coherent SDK baseline rather than
being combined with tools or files from other distributions.

All 17 `.LIB`/`.OBJ` paths are present at their source-media locations. This
includes the general PSXLIB set, the distinct H2000 set, and the demo CARD
object. The demo CARD object is byte-identical to the general CARD object.

The medium contains CCPSX 1.02, PSYLINK 1.17, PSYLIB 1.04, ASMPSX 1.07 and its
compiler directory. The extensionless GCC 2.4.1 pair and `.EXE` GCC 2.6.0 pair
inside that directory are both files from this one medium, not separate SDKs.
Both supplied assembler executables require the original software key in the
current DOS environment. No assembler from another archive is substituted.

The build supplies its native C compiler and replacement assemblers separately.
The original SDK binaries remain proprietary; this output must not be committed
to Git.
"""
    (stage / "ATTRIBUTION.md").write_text(text, encoding="utf-8", newline="\n")

    rows = ["id\tenvironment\tsha256\turl"]
    for identifier, medium in MEDIA.items():
        rows.append(f"{identifier}\t{medium.env}\t{medium.sha256}\t{medium.url}")
    (stage / "SOURCE_MEDIA.tsv").write_text("\n".join(rows) + "\n", encoding="utf-8")


def normalize_stage(stage: Path) -> None:
    for path in sorted(stage.rglob("*")):
        if path.is_symlink():
            raise RuntimeError(f"staged SDK must not contain symlinks: {path}")
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

    release_extract = work / "release-2.5"
    extract_archive(floppies, release_extract)

    release_root = find_release_root(release_extract)
    verify_release25(release_root)
    stage_release25(release_root, stage)
    write_attribution(stage)
    write_manifest(stage)
    normalize_stage(stage)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--stage-dir",
        type=Path,
        default=PROJECT_DIR / "build" / "sdk",
        help="stage directly here (default: build/sdk)",
    )
    parser.add_argument("--work-dir", type=Path, help="temporary extraction directory")
    parser.add_argument("--keep-work", action="store_true", help="retain temporary extraction files")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    owned_work = args.work_dir is None
    work = (
        Path(tempfile.mkdtemp(prefix="kings-field-sdk-"))
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
        log("SDK initialization complete")
    finally:
        if owned_work and not args.keep_work:
            shutil.rmtree(work, ignore_errors=True)
        elif args.keep_work:
            log(f"kept work directory: {work}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
