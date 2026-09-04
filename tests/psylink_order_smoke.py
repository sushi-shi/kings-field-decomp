"""Run controlled object/archive order probes through Psy-Q PSYLINK 1.17."""

from __future__ import annotations

import os
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path


ORIGIN = 0x80010000


def run(arguments: list[str], *, cwd: Path, env: dict[str, str] | None = None) -> None:
    process = subprocess.run(
        arguments,
        cwd=cwd,
        env=env,
        capture_output=True,
        text=True,
        check=False,
        timeout=30,
    )
    if process.returncode:
        raise RuntimeError(
            f"command failed ({process.returncode}): {' '.join(arguments)}\n"
            f"{process.stderr or process.stdout}"
        )


def symbol_address(path: Path, name: str) -> int:
    data = path.read_bytes()
    encoded = name.upper().encode("ascii")
    marker = bytes((2, len(encoded))) + encoded
    offsets = []
    cursor = 0
    while (offset := data.find(marker, cursor)) >= 0:
        if offset >= 4:
            offsets.append(struct.unpack_from("<I", data, offset - 4)[0])
        cursor = offset + len(marker)
    if len(offsets) != 1:
        raise RuntimeError(f"{path}: expected one {name} symbol, found {offsets}")
    return offsets[0]


def assert_link_succeeded(path: Path) -> None:
    text = path.read_text(encoding="ascii", errors="replace")
    if "Linking completed." not in text or "0 error(s)" not in text:
        raise RuntimeError(f"{path}: PSYLINK did not succeed:\n{text}")


def main() -> int:
    psyq_bin = Path(os.environ["PSYQ_BIN"])
    psyq_lib = Path(os.environ["PSYQ_LIB"])
    psyk = shutil.which("psyk")
    emulator = os.environ.get("PSYLINK_DOSBOX") or shutil.which("dosbox-x")
    emulator = emulator or shutil.which("dosbox")
    if psyk is None or emulator is None:
        raise RuntimeError("psyk or a DOSBox emulator is missing")
    dosbox_x = Path(emulator).name == "dosbox-x"

    with tempfile.TemporaryDirectory(prefix="kf-psylink-order-") as directory:
        root = Path(directory)
        api = root / "api"
        gs = root / "gs"
        api.mkdir()
        gs.mkdir()
        run([psyk, "extract", str(psyq_lib / "LIBAPI.LIB")], cwd=api)
        run([psyk, "extract", str(psyq_lib / "LIBGS.LIB")], cwd=gs)
        shutil.copy2(psyq_bin / "PSYLINK.EXE", root / "PSYLINK.EXE")
        for name in ("A02.OBJ", "A03.OBJ", "A04.OBJ"):
            shutil.copy2(api / name, root / name)
        shutil.copy2(gs / "GS_007.OBJ", root / "G7.OBJ")
        shutil.copy2(gs / "GS_008.OBJ", root / "G8.OBJ")

        run(
            [psyk, "create", "T123.LIB", "A02.OBJ", "A03.OBJ", "A04.OBJ"],
            cwd=root,
        )
        run(
            [psyk, "create", "T213.LIB", "A03.OBJ", "A02.OBJ", "A04.OBJ"],
            cwd=root,
        )

        dosbox_env = os.environ.copy()
        dosbox_env.update({
            "HOME": str(root / "home"),
            "XDG_CONFIG_HOME": str(root / "config"),
            "SDL_AUDIODRIVER": "dummy",
        })
        if dosbox_x:
            dosbox_env["SDL_VIDEODRIVER"] = "dummy"
        elif "DISPLAY" not in dosbox_env:
            dosbox_env["SDL_VIDEODRIVER"] = "offscreen"
        (root / "home").mkdir()
        (root / "config").mkdir()
        commands = (
            "psylink /o$80010000 a02.obj+a03.obj,d12.cpe,d12.sym,d12.map > d12.txt",
            "psylink /o$80010000 a03.obj+a02.obj,d21.cpe,d21.sym,d21.map > d21.txt",
            "psylink /o$80010000 g7.obj+g8.obj,a123.cpe,a123.sym,a123.map,t123.lib > a123.txt",
            "psylink /o$80010000 g7.obj+g8.obj,a213.cpe,a213.sym,a213.map,t213.lib > a213.txt",
            "psylink /o$80010000 g8.obj+g7.obj,r87.cpe,r87.sym,r87.map,t123.lib > r87.txt",
        )
        if dosbox_x:
            dosbox = [
                emulator,
                "-silent",
                "-fastlaunch",
                "-set",
                "sdl output=surface",
                "-c",
                f"mount c {root}",
                "-c",
                "c:",
            ]
        else:
            dosbox = [
                emulator,
                "--exit",
                "-noconsole",
                "-set",
                "output=texture",
                "-c",
                f"mount c {root}",
                "-c",
                "c:",
            ]
        for command in commands:
            dosbox.extend(("-c", command))
        if dosbox_x:
            dosbox.append("-exit")
        run(dosbox, cwd=root, env=dosbox_env)
        for name in ("D12.TXT", "D21.TXT", "A123.TXT", "A213.TXT", "R87.TXT"):
            assert_link_succeeded(root / name)
        for name in ("D12.CPE", "D21.CPE", "A123.CPE", "A213.CPE", "R87.CPE"):
            if (root / name).read_bytes()[:6] != b"CPE\x01\x08\x00":
                raise RuntimeError(f"{name}: missing CPE v1/select-unit-0 prefix")

        if (
            symbol_address(root / "D12.SYM", "SetRCnt"),
            symbol_address(root / "D12.SYM", "GetRCnt"),
        ) != (ORIGIN, ORIGIN + 0x10):
            raise RuntimeError("PSYLINK did not retain A02+A03 direct-input order")
        if (
            symbol_address(root / "D21.SYM", "GetRCnt"),
            symbol_address(root / "D21.SYM", "SetRCnt"),
        ) != (ORIGIN, ORIGIN + 0x10):
            raise RuntimeError("PSYLINK did not retain A03+A02 direct-input order")

        archive_symbols = {
            "GetRCnt": ORIGIN + 0x5C,
            "SetRCnt": ORIGIN + 0x6C,
            "StartRCnt": ORIGIN + 0x7C,
        }
        for output in ("A123.SYM", "A213.SYM", "R87.SYM"):
            for name, expected in archive_symbols.items():
                actual = symbol_address(root / output, name)
                if actual != expected:
                    raise RuntimeError(
                        f"{output}: {name}={actual:#x}, expected {expected:#x}"
                    )
        if (root / "A123.CPE").read_bytes() != (root / "A213.CPE").read_bytes():
            raise RuntimeError("archive member order changed the linked CPE")
        if (root / "A123.SYM").read_bytes() != (root / "A213.SYM").read_bytes():
            raise RuntimeError("archive member order changed the linked symbols")

    print(
        "PSYLINK 1.17 order calibration: direct inputs follow command order; "
        "reversed archive storage and root order retain extracted library order"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
