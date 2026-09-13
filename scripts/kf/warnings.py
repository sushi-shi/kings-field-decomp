"""Audit maximal diagnostics with Clang C++20/C89 and pinned historical GCC.

Reports retain SDK and language-compatibility warnings; no suppression or source
rewriting is performed. Counts deduplicate complete diagnostic lines per mode.
"""

from concurrent.futures import ThreadPoolExecutor
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import argparse
import csv

from scripts.kf.manifest import load
from scripts.kf.clangd import environment, unit_arguments
from scripts.kf.compile import C_COMPILERS
from scripts.kf.paths import BUILD, REPO


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=BUILD / "warning-audit")
    args = parser.parse_args(argv)
    repo = REPO
    out = args.output_dir.resolve()
    if not out.is_relative_to(BUILD.resolve()):
        raise ValueError("Warning reports must be written under build/")
    out.mkdir(parents=True, exist_ok=True)
    manifest = load()
    units = [u for u in manifest.units if manifest.profiles[u.profile].language == "c"]
    clang, sdk = environment()
    gcc_flags = [
        "-W",
        "-Waggregate-return",
        "-Wall",
        "-Wcast-align",
        "-Wcast-qual",
        "-Wchar-subscripts",
        "-Wcomment",
        "-Wconversion",
        "-Wformat",
        "-Wimplicit",
        "-Wimport",
        "-Winline",
        "-Wmissing-braces",
        "-Wmissing-prototypes",
        "-Wnested-externs",
        "-Wparentheses",
        "-Wpointer-arith",
        "-Wredundant-decls",
        "-Wreturn-type",
        "-Wshadow",
        "-Wstrict-prototypes",
        "-Wswitch",
        "-Wtraditional",
        "-Wtrigraphs",
        "-Wuninitialized",
        "-Wunused",
        "-Wwrite-strings",
        "-pedantic",
    ]
    clang_flags = [
        "-Weverything",
        "-Wsystem-headers",
        "-pedantic",
        "-ferror-limit=0",
        "-fdiagnostics-show-option",
        "-fno-color-diagnostics",
    ]
    cpp_flags = ["-Wall", "-Wtraditional", "-Wimport", "-pedantic"]
    inputs = {
        str(p): hashlib.sha256(p.read_bytes()).hexdigest()
        for folder in ("src", "include", "vendor")
        for p in (repo / folder).rglob("*")
        if p.is_file()
    }
    probe = out / "probe.i"
    probe.write_text("int probe(void) { return 0; }\n")
    compiler_probes = sorted({C_COMPILERS[manifest.profiles[u.profile].compiler][1] for u in units})
    for cc in compiler_probes:
        result = subprocess.run(
            [cc, "-quiet", "-O2", *gcc_flags, str(probe), "-o", os.devnull],
            text=True,
            capture_output=True,
        )
        assert not re.search("Invalid option|unrecognized|unknown.*option", result.stderr, re.I), (
            result.stderr
        )
        assert result.returncode == 0, result.stderr

    def run(task):
        mode, unit = task
        folder = out / mode
        folder.mkdir(exist_ok=True)
        commands = []
        if mode.startswith("clang"):
            arguments = unit_arguments(
                unit, repo, clang, sdk, mode="modern" if mode == "clang-cpp20" else "retail"
            )
            arguments[arguments.index("-c")] = "-fsyntax-only"
            arguments[1:1] = [*clang_flags, "-O2"]
            commands.append(arguments)
            result = subprocess.run(arguments, text=True, capture_output=True)
            log = result.stdout + result.stderr
        else:
            profile = manifest.profiles[unit.profile]
            cpp, cc, _ = C_COMPILERS[profile.compiler]
            intermediate = folder / f"{unit.unit}.i"
            args = [
                cpp,
                "-lang-c",
                "-undef",
                "-nostdinc",
                *cpp_flags,
                "-I",
                str(repo / "include"),
                "-I",
                str(repo / "vendor/include"),
                "-I",
                str(sdk),
                *(f"-D{d}" for d in unit.defines),
                str(repo / unit.source),
            ]
            commands.append(args)
            result = subprocess.run(args, capture_output=True)
            log = result.stderr.decode(errors="replace")
            if result.returncode == 0:
                intermediate.write_bytes(result.stdout)
                args = [
                    cc,
                    "-quiet",
                    f"-{profile.optimization}",
                    f"-G{profile.small_data}",
                    *profile.cc1_flags,
                    *gcc_flags,
                    str(intermediate),
                    "-o",
                    os.devnull,
                ]
                commands.append(args)
                result = subprocess.run(args, text=True, capture_output=True)
                log += result.stdout + result.stderr
        (folder / f"{unit.unit}.log").write_text(log)
        return {
            "mode": mode,
            "unit": unit.unit,
            "source": unit.source,
            "exit_code": result.returncode,
            "warnings": sum("warning:" in line for line in log.splitlines()),
            "commands": commands,
            "log": str((folder / f"{unit.unit}.log").relative_to(repo)),
        }

    with ThreadPoolExecutor(max_workers=4) as pool:
        rows = list(
            pool.map(
                run, [(m, u) for m in ("clang-cpp20", "clang-c89", "gcc-pinned") for u in units]
            )
        )
    assert all(hashlib.sha256(Path(p).read_bytes()).hexdigest() == h for p, h in inputs.items()), (
        "source changed during audit"
    )
    summary = {}
    diagnostics = []
    for mode in ("clang-cpp20", "clang-c89", "gcc-pinned"):
        selected = [r for r in rows if r["mode"] == mode]
        counts = Counter()
        unique = set()
        origins = Counter()
        examples = []
        for row in selected:
            for line in (repo / row["log"]).read_text().splitlines():
                if "warning:" not in line:
                    continue
                clean = line.replace(str(repo) + "/", "").replace(str(sdk) + "/", "SDK/")
                unique.add(clean)
                origins[
                    "sdk"
                    if clean.startswith("SDK/")
                    else "project"
                    if clean.startswith(("src/", "include/"))
                    else "other"
                ] += 1
                group = re.search(r"\[(-W[^]]+)\]", line)
                category = (
                    group[1]
                    if group
                    else re.sub(r"'[^']*'|`[^']*'", "<name>", line.split("warning:", 1)[1].strip())
                )
                counts[category] += 1
                if len(examples) < 10:
                    examples.append(clean)
        diagnostics.extend({"mode": mode, "diagnostic": line} for line in sorted(unique))
        summary[mode] = {
            "variants": len(selected),
            "variants_with_warnings": sum(r["warnings"] > 0 for r in selected),
            "failed_variants": sum(r["exit_code"] != 0 for r in selected),
            "warnings": sum(r["warnings"] for r in selected),
            "unique_diagnostic_lines": len(unique),
            "origins": dict(origins),
            "top_categories": counts.most_common(20),
            "examples": examples,
        }
    report = {
        "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
        "input_sha256": inputs,
        "clang_flags": clang_flags,
        "gcc_flags": gcc_flags,
        "cpp_flags": cpp_flags,
        "summary": summary,
        "runs": rows,
    }
    (out / "report.json").write_text(json.dumps(report, indent=2))
    with (out / "diagnostics.tsv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=["mode", "diagnostic"], delimiter="\t")
        writer.writeheader()
        writer.writerows(diagnostics)
    print(json.dumps(summary, indent=2))
    return int(any(row["exit_code"] for row in rows))


if __name__ == "__main__":
    raise SystemExit(main())
