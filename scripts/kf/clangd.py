"""Generate Clang editor commands from the reconstruction manifest."""

from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import tempfile

from scripts.kf.manifest import Manifest, Unit, load as load_manifest
from scripts.kf.paths import REPO


IMAGES = ("psx", "game", "open")
MODES = {
    "modern": ("-x", "c++", "-std=gnu++20"),
    "retail": ("-x", "c", "-std=gnu89"),
}
FLAGS = (
    "--target=mipsel-none-elf", "-march=mips1", "-mabi=32",
    "-ffreestanding", "-fno-builtin", "-undef", "-nostdinc",
)


def environment() -> tuple[str, Path]:
    compiler = shutil.which("clang")
    sdk_value = os.environ.get("PSYQ_INCLUDE")
    if compiler is None or not sdk_value or not Path(sdk_value).is_dir():
        raise ValueError("Clang checks require Clang and PSYQ_INCLUDE; enter nix develop")
    return compiler, Path(sdk_value).resolve()


def selected_mode(mode: str | None = None, *, repo: Path = REPO) -> str:
    context = repo / "build/clangd/mode"
    if mode is None:
        mode = context.read_text(encoding="utf-8").strip() if context.is_file() else "modern"
    if mode not in MODES:
        raise ValueError(f"invalid clangd mode {mode!r}; use modern or retail")
    return mode


def unit_arguments(
    unit: Unit, repo: Path, compiler: str, sdk: Path, *, mode: str = "modern",
) -> list[str]:
    if mode not in MODES:
        raise ValueError(f"invalid Clang mode {mode!r}; use modern or retail")
    return [
        compiler, *MODES[mode], *FLAGS,
        "-I", str(repo / "include"), "-isystem", str(sdk),
        *(f"-D{define}" for define in unit.defines),
        "-c", str(repo / unit.source),
    ]


def commands(
    manifest: Manifest, repo: Path, compiler: str, sdk: Path, image: str,
    *, mode: str = "modern",
) -> list[dict]:
    """Select one command per C source, preferring the requested overlay."""
    selected: dict[str, Unit] = {}
    for unit in manifest.units:
        if manifest.profiles[unit.profile].language != "c":
            continue
        previous = selected.get(unit.source)
        if previous is None or unit.image_key == image:
            selected[unit.source] = unit
    result = []
    for source, unit in sorted(selected.items()):
        path = str(repo / source)
        result.append({
            "directory": str(repo),
            "file": path,
            "arguments": unit_arguments(unit, repo, compiler, sdk, mode=mode),
        })
    return result


def _write_if_changed(path: Path, content: str) -> None:
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent,
                                     prefix=f".{path.name}.", delete=False) as stream:
        temporary = Path(stream.name)
        stream.write(content)
    try:
        temporary.replace(path)
    finally:
        temporary.unlink(missing_ok=True)


def generate(
    manifest: Manifest | None = None, *, image: str | None = None,
    mode: str | None = None, repo: Path = REPO,
) -> tuple[int, str]:
    repo = repo.resolve()
    context = repo / "build/clangd/image"
    if image is None:
        image = context.read_text(encoding="utf-8").strip() if context.is_file() else "game"
    if image not in IMAGES:
        raise ValueError(f"invalid clangd image {image!r}; use psx, game, or open")
    mode = selected_mode(mode, repo=repo)
    compiler, sdk = environment()
    entries = commands(manifest or load_manifest(), repo, compiler, sdk, image, mode=mode)
    _write_if_changed(repo / "compile_commands.json", json.dumps(entries, indent=2) + "\n")
    _write_if_changed(context, image + "\n")
    _write_if_changed(repo / "build/clangd/mode", mode + "\n")
    return len(entries), image
