"""Bounded floor/save/re-entry comparisons using user-supplied Japanese resources.

Run under xvfb-run in the development shell. Original code is compiled through
observer wrappers; production sources and the normal executable are not edited.
"""

import argparse
import os
from pathlib import Path
import shlex
import signal
import subprocess
import time


def build_observer(root, build, output, movement):
    subprocess.run(["cmake", "--build", str(build)], cwd=root, check=True)
    commands = subprocess.check_output(
        ["ninja", "-t", "commands", "kings-field"], cwd=build, text=True
    ).splitlines()
    link = shlex.split(commands[-1])
    if link[:2] == [":", "&&"]:
        link = link[2:]
    if link[-2:] == ["&&", ":"]:
        link = link[:-2]
    fixtures = [("src/game/game.cpp", "GAME"), ("src/open/opening_helpers.cpp", "OPENING")]
    if movement:
        fixtures.append(("src/game/player_update.cpp", "INPUT"))
    for source, mode in fixtures:
        wrapper = next(
            path for path in (build / "original").rglob("*.cpp")
            if str(root / source) in path.read_text()
        )
        command = next(shlex.split(line) for line in commands if str(wrapper) in line)
        original = command[command.index("-o") + 1]
        replacement = str(output / f"{mode.lower()}.o")
        command[command.index("-o") + 1] = replacement
        command[command.index(str(wrapper))] = str(Path(__file__).with_suffix(".cpp").resolve())
        command.extend([f"-DKF_AUDIT_{mode}", f'-DKF_AUDIT_SOURCE="{root / source}"'])
        subprocess.run(command, cwd=build, check=True)
        link[link.index(original)] = replacement
    link[link.index("-o") + 1] = str(output / "client")
    subprocess.run(link, cwd=build, check=True)


def run_observer(output, data, movement):
    saves = output / "saves"
    saves.mkdir(exist_ok=True)
    environment = dict(
        os.environ, SDL_VIDEODRIVER="x11", SDL_AUDIO_DRIVER="dummy",
        GDK_BACKEND="x11", GSK_RENDERER="cairo", KF_AUDIT_OUTPUT=str(output),
        ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1",
    )
    environment.pop("KF_AUDIT_MOVEMENT", None)
    if movement:
        environment["KF_AUDIT_MOVEMENT"] = "1"
    log = output / "runtime.log"
    with log.open("w") as stream:
        process = subprocess.Popen(
            [str(output / "client"), "--data", str(data), "--skip-intro", "--saves", str(saves)],
            stdout=stream, stderr=subprocess.STDOUT, env=environment, start_new_session=True,
        )
        start, focused = time.monotonic(), False
        try:
            while process.poll() is None and time.monotonic() - start < 180:
                if not focused:
                    result = subprocess.run(
                        ["xdotool", "search", "--pid", str(process.pid), "--name", "^King.s Field$"],
                        capture_output=True, text=True,
                    )
                    if result.returncode == 0:
                        subprocess.run(["xdotool", "windowfocus", result.stdout.splitlines()[0]], check=True)
                        focused = True
                if "Audit:" in log.read_text() or "runtime error:" in log.read_text():
                    break
                time.sleep(0.2)
            if process.poll() is None:
                raise RuntimeError(f"Observer stalled or reported an error; see {log}")
            if process.returncode != 0:
                raise RuntimeError(f"Observer exited {process.returncode}; see {log}")
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
                process.wait(timeout=5)
    contents = log.read_text()
    print(contents)
    if "AUDIT complete:" not in contents or contents.count("AUDIT entry=") != 6:
        raise RuntimeError("Observer did not finish all scenarios")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--build", choices=["linux", "sanitize"], default="sanitize")
    parser.add_argument("--data", type=Path, required=True)
    parser.add_argument("--movement", action="store_true")
    parser.add_argument("--baseline", type=Path, help="Compare all six saves with a previous output directory")
    args = parser.parse_args()
    root = args.source_root.resolve()
    output = root / "build" / ("cleanup-audit-movement" if args.movement else "cleanup-audit")
    output.mkdir(exist_ok=True)
    build_observer(root, root / "build" / args.build, output, args.movement)
    run_observer(output, args.data.resolve(), args.movement)
    if args.baseline:
        for entry in range(1, 7):
            name = f"entry-{entry}.kfs"
            if (output / name).read_bytes() != (args.baseline / name).read_bytes():
                raise RuntimeError(f"Save differs from reference: {name}")
        print("All six saves match the reference byte for byte.")


if __name__ == "__main__":
    main()
