"""Run in nix develop under xvfb-run; uses a private Wine prefix and save directory.

Optional --disc IMAGE compares Japanese and English extraction with the native
build, then runs the existing six-entry gameplay/save/language observer in Wine.
"""

import argparse
import os
import shutil
from pathlib import Path
import subprocess
import tempfile
import time

from runtime_scenarios import build_observer

ROOT = Path(__file__).resolve().parents[1]


def windows_path(path):
    return "Z:" + str(Path(path).resolve()).replace("/", "\\")


def contents(root):
    return {path.relative_to(root).as_posix(): path.read_bytes()
            for path in root.rglob("*") if path.is_file()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--disc", type=Path)
    args = parser.parse_args()
    subprocess.run(["cmake", "--build", "--preset", "windows", "--parallel", "8",
                    "--target", "kings-field", "kf-windows-regressions"], cwd=ROOT, check=True)
    with tempfile.TemporaryDirectory(prefix="kf-windows-", dir=ROOT / "build") as temporary:
        root = Path(temporary)
        environment = dict(os.environ, WINEPREFIX=str(root / "wine"), WINEDEBUG="-all",
                           WINEDLLOVERRIDES="mscoree,mshtml=", SDL_AUDIO_DRIVER="dummy")
        environment.pop("SDL_VIDEODRIVER", None)

        def run(executable, *options, success=True):
            result = subprocess.run(["wine", str(executable), *map(str, options)], env=environment,
                                    capture_output=True, text=True, timeout=120)
            if result.returncode != (0 if success else 1):
                raise RuntimeError(result.stdout + result.stderr)
            return result

        try:
            print(run(ROOT / "build/windows/kf-windows-regressions.exe").stdout)
            game = ROOT / "build/windows/kings-field.exe"
            assert "Usage:" in run(game, "--help").stdout
            run(game, "--language", "invalid", success=False)
            if not args.disc:
                return
            subprocess.run(["cmake", "--build", "--preset", "linux", "--parallel", "8"], cwd=ROOT, check=True)
            japanese, english = root / "日本語 resources", root / "English resources"
            native = ROOT / "build/linux/kings-field"
            for language, destination in [("ja", japanese), ("en", english)]:
                reference = root / (language + "-reference")
                subprocess.run([native, "--disc", args.disc.resolve(), "--language", language,
                                "--extract-to", reference, "--extract-only"], check=True)
                run(game, "--disc", windows_path(args.disc), "--language", language,
                    "--extract-to", windows_path(destination), "--extract-only")
                assert contents(destination) == contents(reference)
            prepared = root / "Prepared English"
            run(game, "--data", windows_path(japanese), "--extract-to", windows_path(prepared), "--extract-only")
            assert contents(prepared) == contents(english)
            run(game, "--data", windows_path(japanese), "--extract-to", windows_path(prepared),
                "--extract-only", success=False)
            assert contents(prepared) == contents(english)
            damaged = japanese / "KF/COM/COM.DAT"
            original = damaged.read_bytes()
            damaged.write_bytes(original[:-1] + bytes([original[-1] ^ 1]))
            rejected = root / "rejected"
            run(game, "--data", windows_path(japanese), "--extract-to", windows_path(rejected),
                "--extract-only", success=False)
            assert not rejected.exists()
            damaged.write_bytes(original)
            # CUE resolution must handle a Windows backslash path and a spaced BIN name.
            cue = root / "disc.cue"
            image = root / "Japanese disc.bin"
            shutil.copyfile(args.disc.resolve(), image)
            cue.write_text('FILE "Japanese disc.bin" BINARY\n TRACK 01 MODE2/2352\n INDEX 01 00:00:00\n')
            from_cue = root / "cue-output"
            run(game, "--disc", windows_path(cue), "--language", "ja", "--extract-to",
                windows_path(from_cue), "--extract-only")
            assert contents(from_cue) == contents(japanese)
            print("Windows BIN/CUE extraction and English generation match all native resource bytes.")
            output = root / "observer"
            output.mkdir()
            saves = output / "saves"
            saves.mkdir()
            build_observer(ROOT, ROOT / "build/windows", output, False)
            environment.update(KF_AUDIT_OUTPUT=windows_path(output), KF_AUDIT_LANGUAGE_SWITCH="1")
            # Direct disc startup also checks automatic temporary Japanese/English trees.
            with (output / "runtime.log").open("w") as stream:
                process = subprocess.Popen(["wine", str(output / "client.exe"), "--disc", windows_path(args.disc),
                                            "--saves", windows_path(saves)], env=environment,
                                           stdout=stream, stderr=subprocess.STDOUT)
                try:
                    deadline = time.monotonic() + 180
                    focused = False
                    while process.poll() is None and time.monotonic() < deadline:
                        if not focused:
                            found = subprocess.run(["xdotool", "search", "--name", "^King.s Field$"],
                                                   capture_output=True, text=True)
                            if found.returncode == 0:
                                subprocess.run(["xdotool", "windowfocus", found.stdout.splitlines()[0]], check=True)
                                focused = True
                        time.sleep(0.2)
                    if process.poll() is None:
                        process.terminate()
                        process.wait(timeout=10)
                        raise RuntimeError("Windows gameplay observer timed out")
                    log = (output / "runtime.log").read_text()
                    print(log)
                    assert process.returncode == 0 and "AUDIT complete:" in log
                    assert log.count("AUDIT language round trip") == 6
                finally:
                    if process.poll() is None:
                        process.kill()
                        process.wait()
            assert not list((root / "wine/drive_c/users").rglob("kings-field-language-*"))
            assert not list((root / "wine/drive_c/users").rglob("kings-field-disc-*"))
            print("Windows English startup, six gameplay/save/language round trips and temporary cleanup passed.")
        finally:
            subprocess.run(["wineserver", "-k"], env=environment, check=False)
            subprocess.run(["wineserver", "-w"], env=environment, check=False)


if __name__ == "__main__":
    main()
