"""Link a headless regression against an existing native Ninja game build.

Example: nix develop -c python3 tests/run_game_regression.py tests/effect_storage.cpp
No disc or graphical session is required.
"""

import argparse
import os
from pathlib import Path
import shlex
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("test", type=Path)
    parser.add_argument("--build-dir", type=Path, default=Path("build/sanitize"))
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    test = (root / args.test).resolve()
    build = (root / args.build_dir).resolve()
    output = build / "regressions" / test.stem
    output.mkdir(parents=True, exist_ok=True)
    subprocess.run(["cmake", "--build", str(build)], cwd=root, check=True)
    commands = [shlex.split(line) for line in subprocess.check_output(
        ["ninja", "-t", "commands", "kings-field"], cwd=build, text=True
    ).splitlines()]
    source = str(root / "src/platform/entry.cpp")
    compile_command = next(command.copy() for command in commands if source in command)
    original = compile_command[compile_command.index("-o") + 1]
    replacement = str(output / "entry.o")
    compile_command[compile_command.index("-o") + 1] = replacement
    compile_command[compile_command.index(source)] = str(test)
    if "-MF" in compile_command:
        compile_command[compile_command.index("-MF") + 1] = str(output / "entry.d")
    compile_command.append("-UNDEBUG")
    subprocess.run(compile_command, cwd=build, check=True)
    link = commands[-1]
    if link[:2] == [":", "&&"]:
        link = link[2:]
    if link[-2:] == ["&&", ":"]:
        link = link[:-2]
    link[link.index(original)] = replacement
    binary = output / test.stem
    link[link.index("-o") + 1] = str(binary)
    subprocess.run(link, cwd=build, check=True)
    environment = os.environ.copy()
    environment.setdefault("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1")
    subprocess.run([str(binary)], env=environment, check=True, timeout=30)


if __name__ == "__main__":
    main()
