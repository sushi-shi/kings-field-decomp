"""Batch-export Ghidra prototype/decompiler candidates for identity review.

Ghidra is deliberately a proposal lane.  This command labels known starts in a
per-image build project and writes JSON under ``build/ghidra-inventory``.  It
never edits the curated TSVs, and failures/conflicting auto-carves remain rows
in the output rather than silently changing the structural census.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from scripts.kf.delink import image_key
from scripts.kf.inventory import load_function_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv


def _function_rows(image: str) -> list[dict[str, object]]:
    identities = load_function_identities(RETAIL_CONFIG, required=True)
    _, functions = read_tsv(RETAIL_CONFIG / "functions.tsv")
    _, vendors = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
    vendor_map = {
        (row["image"], parse_int(row["va"])): row for row in vendors
    }
    rows = []
    for row in functions:
        if row["image"] != image or int(row["fragments"]) != 1:
            continue
        va = parse_int(row["va"])
        vendor = vendor_map.get((image, va))
        identity = identities.get((image, va))
        rows.append({
            "image": image,
            "va": va,
            "body_size": parse_int(row["body_size"]),
            "name": (
                vendor["name"] if vendor and vendor["name"]
                else identity.name if identity is not None
                else row["name"] or f"func_{va:08x}"
            ),
            "decomp": identity is not None,
            "semantic": bool(
                vendor and vendor["name"]
                or identity is not None and not identity.unresolved
            ),
        })
    return rows


def _write_if_changed(path: Path, document: object) -> None:
    content = json.dumps(document, indent=2, sort_keys=True) + "\n"
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.tmp")
    temporary.write_text(content, encoding="utf-8")
    temporary.replace(path)


def _data_references(program, listing, function) -> list[dict[str, object]]:
    """Collect Ghidra's non-flow memory references as review candidates."""
    references: dict[tuple[int, int, str], dict[str, object]] = {}
    for instruction in listing.getInstructions(function.getBody(), True):
        for reference in instruction.getReferencesFrom():
            target = reference.getToAddress()
            reference_type = reference.getReferenceType()
            if (
                target is None
                or not target.isMemoryAddress()
                or reference_type.isFlow()
            ):
                continue
            key = (
                instruction.getAddress().getOffset(),
                target.getOffset(),
                str(reference_type),
            )
            symbol = program.getSymbolTable().getPrimarySymbol(target)
            datum = listing.getDefinedDataContaining(target)
            references[key] = {
                "site": key[0],
                "target": key[1],
                "type": key[2],
                "mnemonic": str(instruction.getMnemonicString()),
                "target_name": (
                    str(symbol.getName(True)) if symbol is not None else None
                ),
                "data_start": (
                    datum.getAddress().getOffset() if datum is not None else None
                ),
                "data_size": datum.getLength() if datum is not None else None,
                "data_type": str(datum.getDataType()) if datum is not None else None,
            }
    return [references[key] for key in sorted(references)]


def export_image(image: str, *, timeout: int = 15) -> Path:
    import pyghidra

    rows = _function_rows(image)
    key = image_key(image)
    root = BUILD / "ghidra-inventory" / key
    root.mkdir(parents=True, exist_ok=True)
    executable = configured_retail_dir() / image
    project = root / "project"
    project.mkdir(parents=True, exist_ok=True)

    with pyghidra.open_program(
        executable,
        project_location=project,
        project_name=f"kf-{key}",
        analyze=True,
    ) as api:
        from ghidra.app.cmd.disassemble import DisassembleCommand
        from ghidra.app.decompiler import DecompInterface
        from ghidra.program.model.address import AddressSet
        from ghidra.program.model.symbol import SourceType
        from ghidra.util.task import TaskMonitor

        program = api.getCurrentProgram()
        listing = program.getListing()
        functions = program.getFunctionManager()
        space = program.getAddressFactory().getDefaultAddressSpace()
        monitor = TaskMonitor.DUMMY

        transaction = program.startTransaction("kf inventory starts and labels")
        try:
            for row in rows:
                start = space.getAddress(row["va"])
                if listing.getInstructionAt(start) is None:
                    DisassembleCommand(AddressSet(start, start), None, True).applyTo(
                        program, monitor
                    )
                function = functions.getFunctionAt(start)
                if function is None and functions.getFunctionContaining(start) is None:
                    end = start.add(max(0, row["body_size"] - 1))
                    try:
                        function = functions.createFunction(
                            None,
                            start,
                            AddressSet(start, end),
                            SourceType.ANALYSIS,
                        )
                    except Exception:
                        function = None
                if function is not None and row["semantic"]:
                    try:
                        function.setName(row["name"], SourceType.IMPORTED)
                    except Exception:
                        pass
        finally:
            program.endTransaction(transaction, True)

        decompiler = DecompInterface()
        decompiler.toggleCCode(True)
        decompiler.toggleSyntaxTree(True)
        if not decompiler.openProgram(program):
            raise RuntimeError(f"Ghidra decompiler could not open {image}")
        output = []
        decompilable = sum(row["decomp"] for row in rows)
        try:
            for position, row in enumerate(item for item in rows if item["decomp"]):
                start = space.getAddress(row["va"])
                function = functions.getFunctionAt(start)
                record = {
                    "image": image,
                    "va": row["va"],
                    "inventory_name": row["name"],
                    "body_size": row["body_size"],
                    "status": "missing-function" if function is None else "available",
                    "ghidra_name": None,
                    "prototype": None,
                    "return_type": None,
                    "parameters": [],
                    "data_references": [],
                    "decompiled_c": None,
                    "error": None,
                }
                if function is None:
                    owner = functions.getFunctionContaining(start)
                    if owner is not None:
                        record["status"] = "interior-conflict"
                        record["error"] = str(owner.getName(True))
                    output.append(record)
                    continue
                record["ghidra_name"] = str(function.getName(True))
                record["prototype"] = str(function.getPrototypeString(False, False))
                record["return_type"] = str(function.getReturnType())
                record["parameters"] = [
                    {
                        "ordinal": parameter.getOrdinal(),
                        "name": str(parameter.getName()),
                        "datatype": str(parameter.getDataType()),
                        "storage": str(parameter.getVariableStorage()),
                    }
                    for parameter in function.getParameters()
                ]
                record["data_references"] = _data_references(program, listing, function)
                result = decompiler.decompileFunction(function, timeout, monitor)
                if result.decompileCompleted():
                    record["status"] = "decompiled"
                    decompiled = result.getDecompiledFunction()
                    if decompiled is not None:
                        record["prototype"] = str(decompiled.getSignature())
                        record["decompiled_c"] = str(decompiled.getC())
                    high_function = result.getHighFunction()
                    if high_function is not None:
                        prototype = high_function.getFunctionPrototype()
                        record["return_type"] = str(prototype.getReturnType())
                        record["parameters"] = [
                            {
                                "ordinal": ordinal,
                                "name": str(parameter.getName()),
                                "datatype": str(parameter.getDataType()),
                                "storage": str(parameter.getStorage()),
                            }
                            for ordinal in range(prototype.getNumParams())
                            if (parameter := prototype.getParam(ordinal)) is not None
                        ]
                else:
                    record["status"] = "decompile-failed"
                    record["error"] = str(result.getErrorMessage())
                output.append(record)
                if position and position % 100 == 0:
                    print(f"[ghidra-inventory] {image}: {position}/{decompilable}")
        finally:
            decompiler.dispose()

    result_path = root / "candidates.json"
    _write_if_changed(result_path, {
        "schema": "kf-ghidra-inventory-v1",
        "image": image,
        "source": str(executable),
        "rows": output,
        "counts": {
            status: sum(row["status"] == status for row in output)
            for status in sorted({row["status"] for row in output})
        },
    })
    return result_path


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", action="append", choices=("psx", "game", "open"))
    parser.add_argument("--timeout", type=int, default=15)
    args = parser.parse_args(argv)
    aliases = {image_key(image): image for image in IMAGE_LAYOUTS}
    selected = [aliases[key] for key in args.image] if args.image else list(IMAGE_LAYOUTS)
    for image in selected:
        print(export_image(image, timeout=args.timeout))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
