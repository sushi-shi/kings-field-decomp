"""Run native static analysis or configure a sanitizer build; see docs/analysis.md."""

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SANITIZERS = {
    "address": "address,undefined,float-divide-by-zero",
    "thread": "thread",
}
CODEC_SOURCES = ["src/audio/codec.cpp", "src/renderer/tim.cpp", "src/lib/resource_decode.cpp"]


def run(command, **kwargs):
    return subprocess.run(command, check=True, **kwargs)


def configure(build, mode, jobs):
    flags = "-D_GLIBCXX_ASSERTIONS -fno-omit-frame-pointer"
    if mode in SANITIZERS:
        flags += " -fsanitize=" + SANITIZERS[mode]
    run(["cmake", "-S", str(ROOT), "-B", str(build), "-G", "Ninja",
         "-DCMAKE_C_COMPILER=clang", "-DCMAKE_CXX_COMPILER=clang++",
         "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_CXX_FLAGS_DEBUG=-g -O1",
         "-DCMAKE_C_FLAGS_DEBUG=-g -O1", "-DCMAKE_C_FLAGS=" + flags,
         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DKF_SANITIZE=OFF",
         "-DCMAKE_CXX_FLAGS=" + flags])
    run(["cmake", "--build", str(build), "--parallel", str(jobs)])


def compile_database(build, output):
    entries = json.loads((build / "compile_commands.json").read_text())
    # clang-tidy bypasses Nix's compiler wrapper, which supplies standard headers.
    includes = {}
    for entry in entries:
        arguments = entry.get("arguments") or shlex.split(entry["command"])
        language = "c" if Path(entry["file"]).suffix == ".c" else "c++"
        key = (arguments[0], language)
        if key not in includes:
            probe = run([arguments[0], "-E", "-v", "-x", language, "-"],
                        input="", capture_output=True, text=True)
            paths = probe.stderr.split("#include <...> search starts here:\n", 1)[1]
            paths = paths.split("End of search list.", 1)[0].splitlines()
            includes[key] = [arg for path in paths for arg in ("-isystem", path.strip())]
        entry["arguments"] = arguments + includes[key]
        entry.pop("command", None)
    (output / "compile_commands.json").write_text(json.dumps(entries, indent=2))
    return entries


def analysis_command(entry, report):
    command, skip = [], False
    for argument in entry["arguments"]:
        if skip:
            skip = False
        elif argument in ("-o", "-MF", "-MT", "-MQ"):
            skip = True
        elif argument not in ("-c", "-MD", "-MMD"):
            command.append(argument)
    return [*command, "-c", "--analyze", "-Wno-unused-command-line-argument",
            "-Xclang", "-analyzer-output=plist-multi-file",
            "-Xclang", "-analyzer-checker=core,cplusplus,deadcode,security,unix,"
            "optin.cplusplus.UninitializedObject,optin.cplusplus.VirtualCall",
            "-o", str(report)]


def static_analysis(args, build, output):
    configure(build, "plain", args.jobs)
    entries = compile_database(build, output)
    if args.file:
        entries = [entry for entry in entries if args.file in entry["file"]]
        if not entries:
            raise RuntimeError(f"No compilation entries match {args.file!r}")
    tools = ("clang", "tidy", "cppcheck") if args.tool == "all" else (args.tool,)
    results = []
    for tool in tools:
        if tool == "cppcheck":
            # Use the filtered compilation database without changing other tools' input.
            database = output / "cppcheck-commands.json"
            database.write_text(json.dumps(entries))
            cache = output / "cppcheck-cache"
            cache.mkdir(exist_ok=True)
            with (output / "cppcheck.xml").open("w") as errors, (output / "cppcheck.log").open("w") as log:
                process = subprocess.run([
                    "cppcheck", "--project=" + str(database), "--enable=warning,performance,portability",
                    "--check-level=exhaustive", "--inconclusive", "--xml", "--xml-version=2",
                    "--cppcheck-build-dir=" + str(cache), "-j", str(args.jobs),
                ], stdout=log, stderr=errors)
            results.append({"tool": tool, "exit": process.returncode})
            continue

        def inspect(entry):
            source = Path(entry["file"]).resolve()
            relative = source.relative_to(ROOT) if source.is_relative_to(ROOT) else source.relative_to(source.anchor)
            report = output / tool / relative
            report.parent.mkdir(parents=True, exist_ok=True)
            if tool == "clang":
                command = analysis_command(entry, report.with_name(report.name + ".plist"))
            else:
                command = ["clang-tidy", "-p", str(output), str(source),
                           "--checks=-*,bugprone-*,performance-*,portability-*",
                           "--header-filter=" + str(ROOT / "include") + "/.*",
                           "--export-fixes=" + str(report.with_name(report.name + ".yaml"))]
            with report.with_name(report.name + ".log").open("w") as log:
                process = subprocess.run(command, cwd=entry["directory"], stdout=log,
                                         stderr=subprocess.STDOUT, timeout=args.timeout)
            print(f"{tool}: {relative}: exit {process.returncode}", flush=True)
            return {"tool": tool, "source": str(relative), "exit": process.returncode}

        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            results.extend(pool.map(inspect, entries))
    (output / "summary.json").write_text(json.dumps(results, indent=2) + "\n")
    print(f"Reports: {output}. Tool exit codes do not count or dismiss findings.")
    return int(any(result["exit"] for result in results))


def codec_analysis(args, output):
    output.mkdir(parents=True, exist_ok=True)
    if args.mode == "fuzz":
        tests, flags = ["fuzz_codecs"], ["-fsanitize=fuzzer,address,undefined"]
    else:
        tests = ["resource_codecs", "codec_records"]
        flags = [] if args.mode == "valgrind" else ["-fsanitize=" + (
            "memory" if args.mode == "memory" else SANITIZERS[args.mode])]
        if args.mode == "memory":
            flags += ["-fsanitize-memory-track-origins=2", "-fPIE", "-pie"]
    environment = dict(os.environ)
    environment.setdefault("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1")
    environment.setdefault("MSAN_OPTIONS", "halt_on_error=1:exit_code=1")
    environment.setdefault("TSAN_OPTIONS", "halt_on_error=1")
    for test in tests:
        binary = output / test
        run(["clang++", "-std=c++20", "-O1", "-g", "-fno-omit-frame-pointer", *flags,
             "-I", str(ROOT / "include"), str(ROOT / "tests" / (test + ".cpp")),
             *[str(ROOT / source) for source in CODEC_SOURCES], "-o", str(binary)])
        command = [str(binary)]
        if args.mode == "valgrind":
            command = ["valgrind", "--track-origins=yes", "--leak-check=full",
                       "--errors-for-leak-kinds=definite,indirect", "--error-exitcode=1", *command]
        if args.mode == "fuzz":
            corpus = (args.corpus or output / "corpus").resolve()
            corpus.mkdir(parents=True, exist_ok=True)
            command += [str(corpus), "-max_total_time=" + str(args.seconds),
                        "-max_len=65536", "-timeout=5", "-artifact_prefix=" + str(output) + "/"]
        log_path = output / (test + ".log")
        print(f"Running {test}: {log_path}", flush=True)
        with log_path.open("w") as log:
            run(command, env=environment, stdout=log, stderr=subprocess.STDOUT,
                timeout=args.seconds + 60)
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="action", required=True)
    build_parser = subparsers.add_parser("build", help="Build the full native game with instrumentation")
    build_parser.add_argument("mode", choices=["address", "thread", "valgrind"])
    static_parser = subparsers.add_parser("static", help="Analyze the current compilation database")
    static_parser.add_argument("--tool", choices=["clang", "tidy", "cppcheck", "all"], default="all")
    static_parser.add_argument("--file", help="Only analyze source paths containing this text")
    static_parser.add_argument("--timeout", type=int, default=240, help="Seconds per Clang translation unit")
    static_parser.add_argument("--output", type=Path, default=ROOT / "build/analysis/reports")
    codec_parser = subparsers.add_parser("codecs", help="Check isolated codecs or fuzz their input")
    codec_parser.add_argument("mode", choices=["address", "thread", "memory", "valgrind", "fuzz"])
    codec_parser.add_argument("--seconds", type=int, default=60, help="Fuzz budget or test timeout")
    codec_parser.add_argument("--corpus", type=Path, help="Local seed directory for fuzzing")
    codec_parser.add_argument("--output", type=Path)
    for command in (build_parser, static_parser):
        command.add_argument("--build-dir", type=Path, help="Dedicated analysis build directory")
        command.add_argument("-j", "--jobs", type=int, default=min(os.cpu_count() or 1, 4))
    args = parser.parse_args()
    if args.action == "codecs":
        if args.seconds < 1:
            parser.error("seconds must be positive")
        return codec_analysis(args, (args.output or ROOT / "build/analysis" / ("codecs-" + args.mode)).resolve())
    if args.jobs < 1 or (args.action == "static" and args.timeout < 1):
        parser.error("jobs and timeout must be positive")
    mode = args.mode if args.action == "build" else "static"
    build = (args.build_dir or ROOT / "build/analysis" / mode).resolve()
    if args.action == "build":
        configure(build, mode, args.jobs)
        print(f"Executable: {build / 'kings-field'}")
        return 0
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    return static_analysis(args, build, output)


if __name__ == "__main__":
    raise SystemExit(main())
