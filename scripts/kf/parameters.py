"""Inventory parameter names and enum/record domains for semantic review.

Both modern and retail views are parsed: retail macros erase enum identities.
Spelling differences are review leads, never automatic rename decisions.
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import sys

from pylibclang import cindex

from scripts.kf.check_types import select_units
from scripts.kf.clangd import environment, unit_arguments
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import REPO


CALLABLES = {
    cindex.CursorKind.CXCursor_FunctionDecl,
    cindex.CursorKind.CXCursor_FunctionTemplate,
    cindex.CursorKind.CXCursor_CXXMethod,
    cindex.CursorKind.CXCursor_Constructor,
    cindex.CursorKind.CXCursor_ConversionFunction,
}
CALLBACK_OWNERS = {
    cindex.CursorKind.CXCursor_ParmDecl,
    cindex.CursorKind.CXCursor_FieldDecl,
    cindex.CursorKind.CXCursor_VarDecl,
    cindex.CursorKind.CXCursor_TypedefDecl,
    cindex.CursorKind.CXCursor_TypeAliasDecl,
}
INDIRECT = {
    cindex.TypeKind.CXType_Pointer,
    cindex.TypeKind.CXType_LValueReference,
    cindex.TypeKind.CXType_RValueReference,
}
ARRAYS = {
    cindex.TypeKind.CXType_ConstantArray,
    cindex.TypeKind.CXType_IncompleteArray,
    cindex.TypeKind.CXType_VariableArray,
    cindex.TypeKind.CXType_DependentSizedArray,
}


def _location(location, root: Path) -> dict:
    path = Path(location.file.name).resolve() if location.file else None
    return {
        "file": str(path.relative_to(root)) if path and path.is_relative_to(root)
        else str(path or ""),
        "line": location.line, "column": location.column, "offset": location.offset,
    }


def _qualified_name(cursor) -> str:
    names = [cursor.spelling or "<unnamed>"]
    parent = cursor.semantic_parent
    while parent and parent.kind != cindex.CursorKind.CXCursor_TranslationUnit:
        if parent.spelling:
            names.append(parent.spelling)
        parent = parent.semantic_parent
    return "::".join(reversed(names))


def domains(type_, root: Path, *, route: str = "value") -> list[dict]:
    """Follow typedefs, indirection, arrays and callback signatures in the AST."""
    type_ = type_.get_canonical()
    if type_.kind in INDIRECT:
        return domains(type_.get_pointee(), root, route=route + "/" + type_.kind.name)
    if type_.kind in ARRAYS:
        return domains(type_.get_array_element_type(), root, route=route + "/array")
    if type_.kind == cindex.TypeKind.CXType_FunctionProto:
        result = domains(type_.get_result(), root, route=route + "/return")
        for index, argument in enumerate(type_.argument_types()):
            result.extend(domains(argument, root, route=f"{route}/argument:{index}"))
        return result
    if type_.kind not in (cindex.TypeKind.CXType_Enum, cindex.TypeKind.CXType_Record):
        return []
    declaration = type_.get_declaration()
    # The checked view carries the enum domain in this ABI-storage wrapper.
    if declaration.spelling == "KfEnumStorage" and type_.get_num_template_arguments() > 0:
        enum_domains = domains(type_.get_template_argument_type(0), root,
                               route=route + "/enum_storage")
        if enum_domains:
            return enum_domains
    kind = "enum" if type_.kind == cindex.TypeKind.CXType_Enum else "struct"
    if declaration.kind == cindex.CursorKind.CXCursor_UnionDecl:
        kind = "union"
    if declaration.kind == cindex.CursorKind.CXCursor_ClassDecl:
        kind = "class"
    location = _location(declaration.location, root)
    return [{"kind": kind, "name": declaration.spelling or type_.spelling,
             "canonical_type": type_.spelling, "route": route,
             "identity": declaration.get_usr(), **location}]


def scan_file(path: Path, arguments: list[str], *, root: Path) -> dict:
    root = root.resolve()
    tu = cindex.Index.create().parse(str(path.resolve()), args=arguments)
    errors = [str(d) for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if errors:
        raise ValueError(f"{path}: parsing failed:\n" + "\n".join(errors))
    parameters = []
    handled_parameters = set()
    unowned_parameters = []
    source_bytes = {}
    files = {str(path.resolve().relative_to(root))}
    for included in tu.get_includes():
        filename = Path(included.include.name).resolve()
        if filename.is_relative_to(root):
            files.add(str(filename.relative_to(root)))

    def walk(cursor):
        location = cursor.location
        project = location.file and Path(location.file.name).resolve().is_relative_to(root)
        if project and cursor.kind in CALLABLES | CALLBACK_OWNERS:
            # FunctionTemplate.get_arguments() is empty in libclang; its direct
            # ParmDecl children retain dependent types and their source sites.
            arguments = [child for child in cursor.get_children()
                         if child.kind == cindex.CursorKind.CXCursor_ParmDecl]
            function_location = _location(location, root)
            for index, parameter in enumerate(arguments):
                handled_parameters.add(parameter.hash)
                extent = parameter.extent
                extent_file = Path(extent.start.file.name).resolve() if extent.start.file else None
                written = b""
                if extent_file and extent.end.file and extent.end.file.name == extent.start.file.name:
                    if extent_file not in source_bytes:
                        source_bytes[extent_file] = extent_file.read_bytes()
                    written = source_bytes[extent_file][extent.start.offset:extent.end.offset]
                parameters.append({
                    **_location(parameter.location, root),
                    "function": (_qualified_name(cursor) if cursor.kind in CALLBACK_OWNERS
                                 else cursor.spelling),
                    "function_usr": cursor.get_usr(),
                    "function_kind": cursor.kind.name,
                    "callable_category": ("callback_signature" if cursor.kind in CALLBACK_OWNERS
                                          else "function"),
                    "function_location": function_location,
                    "linkage": cursor.linkage.name,
                    "definition": cursor.is_definition(),
                    "index": index, "parameter_count": len(arguments),
                    "name": parameter.spelling,
                    "type": parameter.type.spelling,
                    "canonical_type": parameter.type.get_canonical().spelling,
                    "domains": domains(parameter.type, root),
                    "extent": {"start": extent.start.offset, "end": extent.end.offset},
                    # get_tokens() can span unrelated text for macro expansions.
                    # Keep the literal expansion range and expose empty ranges.
                    "source_excerpt": written[:2048].decode("utf-8", errors="replace"),
                    "source_excerpt_complete": len(written) <= 2048,
                })
        if (project and cursor.kind == cindex.CursorKind.CXCursor_ParmDecl
                and cursor.hash not in handled_parameters):
            unowned_parameters.append({**_location(location, root), "name": cursor.spelling,
                                       "type": cursor.type.spelling})
        for child in cursor.get_children():
            if not child.location.is_in_system_header:
                walk(child)

    walk(tu.cursor)
    return {"parameters": parameters, "included_files": sorted(files),
            "unowned_parameters": unowned_parameters,
            "diagnostics": [str(d) for d in tu.diagnostics]}


def collect(units, compiler: str, sdk: Path, *, root: Path = REPO) -> dict:
    root = root.resolve()
    rows = {}
    scans = []
    included = set()
    for unit in units:
        for mode in ("modern", "retail"):
            arguments = unit_arguments(unit, root, compiler, sdk, mode=mode)[1:-2]
            scan = scan_file(root / unit.source, arguments, root=root)
            context = {"image": unit.image, "unit": unit.unit,
                       "source": unit.source, "defines": list(unit.defines), "mode": mode}
            scans.append({**context, "parameters": len(scan["parameters"]),
                          "included_files": scan["included_files"],
                          "unowned_parameters": scan["unowned_parameters"],
                          "diagnostics": scan["diagnostics"]})
            included.update(scan["included_files"])
            occurrences = {}
            for parameter in scan["parameters"]:
                # Macro-generated overloads can share every expansion offset.
                site = (parameter["file"], parameter["function_location"]["offset"],
                        parameter["function"], parameter["parameter_count"],
                        parameter["index"], parameter["offset"], parameter["name"])
                occurrence = occurrences.get(site, 0)
                occurrences[site] = occurrence + 1
                key = (*site, occurrence)
                row = rows.setdefault(key, {
                    key: parameter[key] for key in (
                        "file", "line", "column", "offset", "function", "function_location",
                        "function_kind", "index", "parameter_count", "name", "definition",
                        "callable_category",
                    )
                })
                row["expansion_occurrence"] = occurrence
                observations = row.setdefault("observations", [])
                observation = {**context, **{key: parameter[key] for key in (
                    "function_usr", "linkage", "type", "canonical_type", "domains",
                    "extent", "source_excerpt", "source_excerpt_complete",
                )}}
                if observation not in observations:
                    observations.append(observation)
    parameters = []
    for key in sorted(rows):
        row = rows[key]
        row["id"] = f"P{len(parameters) + 1:05d}"
        row["selected"] = any(o["domains"] for o in row["observations"])
        row["modes"] = sorted({o["mode"] for o in row["observations"]})
        row["origin"] = "vendored" if row["file"].startswith("vendor/") else "project"
        parameters.append(row)
    headers = {str(path.relative_to(root)) for base in ("include", "vendor/include")
               for path in (root / base).rglob("*.h")}
    sources = {str(path.relative_to(root)) for base in ("src", "vendor/src")
               for path in (root / base).rglob("*.c")}
    unowned = {(row["file"], row["offset"], row["name"], row["type"]): row
               for scan in scans for row in scan["unowned_parameters"]}
    return {"schema_version": 1, "variants": len(units), "scans": scans,
            "parameters": parameters, "groups": group_parameters(parameters),
            "by_type": group_types(parameters),
            "coverage": {
                "included_files": sorted(included), "unseen_headers": sorted(headers - included),
                "unselected_sources": sorted(sources - {unit.source for unit in units}),
                "unowned_parameters": [unowned[key] for key in sorted(unowned)],
                "limitations": [
                    "Only active declarations in selected manifest C variants and included "
                    "project headers are visible; unseen headers and inactive preprocessor "
                    "branches are not certified.",
                    "SDK declarations are excluded; SDK types on project parameters are included.",
                    "Modern-only helpers and dependent template parameters are retained; an "
                    "uninstantiated dependent type has no concrete enum/record identity.",
                    "Parameter IDs identify this sorted snapshot, not stable cross-edit keys.",
                    "Macro-generated callables retain expansion occurrence ordinals and AST "
                    "identities; empty source excerpts cannot identify a written parameter token.",
                    "Spelling differences require individual semantic review; equal spellings "
                    "do not prove equal roles.",
                ],
            }}


def group_parameters(parameters: list[dict]) -> list[dict]:
    """Reconcile declarations by image, callable identity and parameter index."""
    def scope(row, observation):
        return (observation["image"], row["function"],
                observation["unit"] if observation["linkage"] == "CXLinkage_Internal" else "")

    modern_identities = {}
    for row in parameters:
        for observation in row["observations"]:
            if observation["mode"] == "modern":
                key = scope(row, observation)
                modern_identities.setdefault(key, set()).add(observation["function_usr"])
    groups = {}
    for row in parameters:
        for observation in row["observations"]:
            function = row["function"]
            overloaded = len(modern_identities.get(scope(row, observation), ())) > 1
            if overloaded:
                function = (observation["function_usr"] if observation["mode"] == "modern"
                            else function + "[retail ABI]")
            if observation["linkage"] == "CXLinkage_Internal":
                function = f"{observation['unit']}::{function}"
            if (row["function_kind"] != "CXCursor_FunctionDecl"
                    or row["function"].startswith("operator")):
                function = observation["function_usr"]
                if row["callable_category"] == "callback_signature":
                    owner = row["function_location"]
                    function = f"{function}@{owner['file']}:{owner['offset']}"
                if observation["linkage"] == "CXLinkage_Internal":
                    function = f"{observation['unit']}::{function}"
            key = (observation["image"], function, row["index"])
            group = groups.setdefault(key, {"image": key[0], "function_identity": function,
                                           "function": row["function"], "index": row["index"],
                                           "overloaded_family": overloaded,
                                           "sites": set(), "names": set(), "domains": set()})
            group["sites"].add(row["id"])
            group["names"].add(row["name"])
            group["domains"].update(domain["name"] for domain in observation["domains"])
    result = []
    for key in sorted(groups):
        group = groups[key]
        for field in ("sites", "names", "domains"):
            group[field] = sorted(group[field])
        group["spelling_difference"] = len(group["names"]) > 1
        result.append(group)
    return result


def group_types(parameters: list[dict]) -> list[dict]:
    """Index a canonical domain's names across functions without judging roles.

    Declaration location disambiguates equal Clang USRs from unrelated TUs;
    an image remains part of the domain context even for a shared header.
    """
    groups = {}
    for row in parameters:
        for observation in row["observations"]:
            for domain in observation["domains"]:
                key = (observation["image"], domain["identity"], domain["file"], domain["offset"])
                group = groups.setdefault(key, {
                    "image": observation["image"], "type_identity": domain["identity"],
                    "type": domain["name"], "kind": domain["kind"],
                    "declaration": {field: domain[field] for field in ("file", "line", "offset")},
                    "sites": set(), "name_buckets": {}, "uses": {},
                })
                group["sites"].add(row["id"])
                group["name_buckets"].setdefault(row["name"], set()).add(row["id"])
                use = group["uses"].setdefault(row["id"], {
                    "site": row["id"], "name": row["name"], "function": row["function"],
                    "index": row["index"], "file": row["file"], "line": row["line"],
                    "routes": set(), "types": set(), "contexts": [],
                })
                use["routes"].add(domain["route"])
                use["types"].add(observation["type"])
                context = {field: observation[field] for field in ("image", "unit", "defines", "mode")}
                if context not in use["contexts"]:
                    use["contexts"].append(context)
    result = []
    for key in sorted(groups):
        group = groups[key]
        group["sites"] = sorted(group["sites"])
        group["names"] = sorted(group["name_buckets"])
        group["name_buckets"] = {name: sorted(group["name_buckets"][name]) for name in group["names"]}
        uses = []
        for site in group["sites"]:
            use = group["uses"][site]
            use["routes"] = sorted(use["routes"])
            use["types"] = sorted(use["types"])
            uses.append(use)
        group["uses"] = uses
        group["spelling_difference"] = len(group["names"]) > 1
        result.append(group)
    return result


def write_tsv(report: dict, *, all_parameters: bool = False, by_type: bool = False) -> None:
    writer = csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
    if by_type:
        writer.writerow(("image", "type", "kind", "declaration_file", "declaration_line",
                         "name", "sites", "functions"))
        for group in report["by_type"]:
            for name, sites in group["name_buckets"].items():
                functions = sorted({use["function"] for use in group["uses"] if use["name"] == name})
                writer.writerow((group["image"], group["type"], group["kind"],
                                 group["declaration"]["file"], group["declaration"]["line"],
                                 name, ";".join(sites), ";".join(functions)))
        return
    writer.writerow(("id", "file", "line", "function", "index", "name", "domains", "types",
                     "definition", "modes", "images", "units", "selected"))
    for row in report["parameters"]:
        if not all_parameters and not row["selected"]:
            continue
        observations = row["observations"]
        writer.writerow((
            row["id"], row["file"], row["line"], row["function"], row["index"], row["name"],
            ";".join(sorted({d["name"] for o in observations for d in o["domains"]})),
            ";".join(sorted({o["mode"] + ":" + o["type"] for o in observations})),
            int(row["definition"]), ";".join(row["modes"]),
            ";".join(sorted({o["image"] for o in observations})),
            ";".join(sorted({o["unit"] for o in observations})), int(row["selected"]),
        ))


def add_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--image", action="append", choices=("psx", "game", "open"), default=[])
    parser.add_argument("--unit", action="append", default=[])
    parser.add_argument("--json", action="store_true", help="full inventory, observations and groups")
    parser.add_argument("--all", action="store_true", help="include scalar parameters in TSV")
    parser.add_argument("--by-type", action="store_true",
                        help="TSV index of domain types and parameter names across functions")


def run(args: argparse.Namespace) -> int:
    try:
        units = select_units(load_manifest(),
                             images=tuple(f"{image.upper()}.EXE" for image in args.image),
                             names=tuple(args.unit))
        compiler, sdk = environment()
        report = collect(units, compiler, sdk)
    except (ValueError, RuntimeError, OSError, cindex.TranslationUnitLoadError) as error:
        print(f"parameters: {error}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        write_tsv(report, all_parameters=args.all, by_type=args.by_type)
    selected = sum(row["selected"] for row in report["parameters"])
    print(f"{selected} enum/record parameter sites; {len(report['parameters'])} total; "
          f"{report['variants']} variants in modern and retail views; "
          f"{len(report['coverage']['unseen_headers'])} unseen headers; "
          f"{len(report['coverage']['unowned_parameters'])} unowned parameter sites", file=sys.stderr)
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    add_arguments(parser)
    return run(parser.parse_args(argv))


if __name__ == "__main__":
    raise SystemExit(main())
