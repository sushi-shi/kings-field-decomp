"""Bounded GAME parser execution for retail and reconstructed MIPS code.

The runner deliberately is not a PlayStation boot harness.  It maps the
verified GAME.EXE load image into the R3000 KSEG0 alias, relocates an explicit
closure of reconstructed functions to private code slots, and executes one
O32 call.  Any attempted candidate-code fetch outside the declared closure is
an error.  Calls into unchanged retail providers and Python service hooks must
also be declared explicitly.

Unicorn does not promise R3000 load-delay behavior.  The default executed-path
audit therefore rejects a load whose destination is read by the immediately
following instruction instead of silently accepting host-emulator semantics.

CPU stores are guarded before execution and their actual bytes checked at the
next instruction boundary. Unicorn's memory-write hook corrupts MIPS delayed
control transfers, so it must not be installed by this runner.
"""

from __future__ import annotations

import struct
from collections import defaultdict
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Callable, Mapping, Sequence

from elftools.elf.elffile import ELFFile

from scripts.kf.delink import load_catalog
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.sema.image import RetailImage


RAM_BASE = 0x80000000
RAM_SIZE = 0x200000
CODE_BASE = 0x80100000
CODE_END = 0x80140000
RODATA_BASE = 0x80140000
RODATA_END = 0x80170000
HOOK_BASE = 0x80170000
HOOK_END = 0x8017F000
UNPATCHED_CODE_VA = 0x8017F000
RETURN_VA = 0x8017FFF0
DEFAULT_STACK_VA = 0x801FF000
DEFAULT_INSTRUCTION_LIMIT = 20_000_000

R_MIPS_32 = 2
R_MIPS_26 = 4
R_MIPS_HI16 = 5
R_MIPS_LO16 = 6


class ParserMachineError(RuntimeError):
    """Base error for link and execution failures."""


class CandidateLinkError(ParserMachineError):
    """A candidate object cannot be safely placed into the explicit closure."""


class ParserMachineFault(ParserMachineError):
    """A bounded parser call faulted or violated an execution invariant."""

    def __init__(self, message: str, *, pc: int, instructions: int) -> None:
        super().__init__(
            f"{message} at {pc:#010x} after {instructions} instructions"
        )
        self.pc = pc
        self.instructions = instructions


@dataclass(frozen=True)
class MemoryInput:
    """Bytes copied into emulated RAM before a call."""

    address: int
    data: bytes


@dataclass(frozen=True)
class MemoryRange:
    """Named memory range captured after a call or admitted for writes."""

    name: str
    address: int
    size: int


@dataclass(frozen=True)
class MemorySnapshot:
    name: str
    address: int
    data: bytes


@dataclass(frozen=True)
class WriteRecord:
    pc: int
    address: int
    size: int
    value: int


@dataclass(frozen=True)
class CallRecord:
    index: int
    name: str
    kind: str
    address: int
    args: tuple[int, int, int, int]
    stack_args: tuple[int, ...]


@dataclass(frozen=True)
class ExecutionResult:
    v0: int
    v1: int
    memory: tuple[MemorySnapshot, ...]
    trace: tuple[CallRecord, ...]
    writes: tuple[WriteRecord, ...]
    instructions: int
    pc: int

    def memory_by_name(self) -> dict[str, bytes]:
        return {item.name: item.data for item in self.memory}


@dataclass(frozen=True)
class HookReturn:
    """Optional register values returned by an :class:`ExternalHook`."""

    v0: int | None = None
    v1: int | None = None


HookHandler = Callable[["HookContext"], HookReturn | int | tuple[int, int] | None]


@dataclass(frozen=True)
class ExternalHook:
    """Deterministic replacement for one I/O or library service.

    ``retail_va`` is intercepted when retail code or an admitted retail
    provider calls the service.  Candidate relocations use a private hook slot
    so an unresolved candidate call can never fall through into GAME.EXE.
    """

    name: str
    handler: HookHandler
    retail_va: int | None = None
    stack_words: int = 0


@dataclass(frozen=True)
class CandidateFunction:
    name: str
    object_path: Path
    stack_words: int = 0


@dataclass(frozen=True)
class RetailProvider:
    """Explicit unchanged GAME function admitted to a candidate closure."""

    name: str
    address: int | None = None
    size: int | None = None
    stack_words: int = 0


@dataclass(frozen=True)
class LinkedFunction:
    name: str
    address: int
    size: int
    kind: str
    stack_words: int = 0

    @property
    def end(self) -> int:
        return self.address + self.size

    def contains(self, address: int, size: int = 1) -> bool:
        return self.address <= address and address + size <= self.end


@dataclass(frozen=True)
class MemoryPatch:
    name: str
    address: int
    data: bytes


@dataclass(frozen=True)
class _HookBinding:
    address: int
    hook: ExternalHook


@dataclass(frozen=True)
class LinkedProgram:
    functions: tuple[LinkedFunction, ...]
    patches: tuple[MemoryPatch, ...] = ()
    hooks: tuple[_HookBinding, ...] = ()
    label: str = "program"
    data_binding: str = "sections"

    def address_of(self, name: str) -> int:
        matches = [item.address for item in self.functions if item.name == name]
        if len(matches) != 1:
            raise KeyError(f"{self.label}: expected one linked function {name!r}")
        return matches[0]


@dataclass(frozen=True)
class GameSymbols:
    """Curated GAME function and data identities used by candidate objects."""

    functions: Mapping[str, tuple[int, int]]
    data: Mapping[str, tuple[int, int]]

    @classmethod
    def load(cls, config_dir: Path = RETAIL_CONFIG) -> "GameSymbols":
        catalog = load_catalog(config_dir)
        functions: dict[str, tuple[int, int]] = {}
        data: dict[str, tuple[int, int]] = {}
        for item in catalog.functions["GAME.EXE"]:
            _insert_unique(functions, item.symbol, (item.va, item.size), "function")
        for item in catalog.data["GAME.EXE"]:
            _insert_unique(data, item.symbol, (item.va, item.size), "data")
        return cls(functions, data)

    def function(self, name: str) -> tuple[int, int]:
        try:
            return self.functions[name]
        except KeyError as error:
            raise CandidateLinkError(f"unknown curated GAME function {name!r}") from error

    def datum(self, name: str) -> tuple[int, int]:
        try:
            return self.data[name]
        except KeyError as error:
            raise CandidateLinkError(f"unknown curated GAME datum {name!r}") from error


def _insert_unique(
    result: dict[str, tuple[int, int]],
    name: str,
    value: tuple[int, int],
    kind: str,
) -> None:
    previous = result.setdefault(name, value)
    if previous != value:
        raise CandidateLinkError(
            f"ambiguous curated GAME {kind} identity {name!r}: {previous} and {value}"
        )


@dataclass(frozen=True)
class _ElfSymbol:
    index: int
    name: str
    section: str | None
    special_section: str | None
    value: int
    size: int
    kind: str


@dataclass(frozen=True)
class _ElfRelocation:
    section: str
    offset: int
    kind: int
    symbol_index: int


@dataclass(frozen=True)
class _ObjectFile:
    path: Path
    sections: Mapping[str, bytes]
    section_sizes: Mapping[str, int]
    symbols: tuple[_ElfSymbol, ...]
    relocations: tuple[_ElfRelocation, ...]

    def symbol(self, index: int) -> _ElfSymbol:
        try:
            return self.symbols[index]
        except IndexError as error:
            raise CandidateLinkError(
                f"{self.path}: relocation uses missing symbol index {index}"
            ) from error

    def named_symbol(self, name: str) -> _ElfSymbol:
        matches = [item for item in self.symbols if item.name == name]
        if len(matches) != 1:
            raise CandidateLinkError(
                f"{self.path}: expected one ELF symbol {name!r}, found {len(matches)}"
            )
        return matches[0]


def resolve_data_object(
    obj: _ObjectFile, symbol: _ElfSymbol, addend: int, addresses: Mapping[str, int]
) -> int:
    """Bind one named object for an isolated function test, not section placement.

    Section-relative references must identify one complete object. Padding,
    ambiguous one-past references and missing identities remain errors.
    """
    if symbol.kind == "STT_OBJECT" and symbol.name in addresses:
        target, offset = symbol, addend
    else:
        offset = symbol.value + addend
        candidates = [item for item in obj.symbols
                      if item.section == symbol.section and item.kind == "STT_OBJECT"
                      and item.size > 0 and item.value <= offset <= item.value + item.size]
        destinations = {addresses[item.name] + offset - item.value for item in candidates
                        if item.name in addresses}
        if not candidates or len(destinations) != 1 or any(item.name not in addresses
                                                          for item in candidates):
            raise CandidateLinkError(
                f"{obj.path}: {symbol.section}+{offset:#x} does not identify one data object")
        # A preceding object's one-past pointer and the next object's start
        # are equivalent only when both mappings produce the same address.
        target = candidates[0]
        offset -= target.value
    if target.name not in addresses or not 0 <= offset <= target.size:
        raise CandidateLinkError(
            f"{obj.path}: unmapped or out-of-bounds data object {target.name!r}+{offset:#x}")
    return (addresses[target.name] + offset) & 0xFFFFFFFF


def _load_object(path: Path) -> _ObjectFile:
    path = path.resolve()
    with path.open("rb") as stream:
        elf = ELFFile(stream)
        if elf["e_machine"] != "EM_MIPS" or not elf.little_endian:
            raise CandidateLinkError(f"{path}: expected little-endian MIPS ELF")
        sections: dict[str, bytes] = {}
        section_sizes: dict[str, int] = {}
        for section in elf.iter_sections():
            name = section.name
            section_sizes[name] = int(section["sh_size"])
            if section["sh_type"] != "SHT_NOBITS":
                sections[name] = section.data()
        symtab = elf.get_section_by_name(".symtab")
        if symtab is None:
            raise CandidateLinkError(f"{path}: missing .symtab")
        symbols: list[_ElfSymbol] = []
        for index, symbol in enumerate(symtab.iter_symbols()):
            section_index = symbol["st_shndx"]
            section_name = None
            special = None
            if isinstance(section_index, int):
                section_name = elf.get_section(section_index).name
            else:
                special = str(section_index)
            symbols.append(
                _ElfSymbol(
                    index,
                    symbol.name,
                    section_name,
                    special,
                    int(symbol["st_value"]),
                    int(symbol["st_size"]),
                    str(symbol["st_info"]["type"]),
                )
            )
        relocations: list[_ElfRelocation] = []
        for section in elf.iter_sections():
            if section["sh_type"] != "SHT_REL":
                continue
            target = elf.get_section(int(section["sh_info"])).name
            for item in section.iter_relocations():
                relocations.append(
                    _ElfRelocation(
                        target,
                        int(item["r_offset"]),
                        int(item["r_info_type"]),
                        int(item["r_info_sym"]),
                    )
                )
        return _ObjectFile(
            path,
            sections,
            section_sizes,
            tuple(symbols),
            tuple(relocations),
        )


def _align(value: int, alignment: int = 16) -> int:
    return (value + alignment - 1) & -alignment


@dataclass(frozen=True)
class _Selected:
    request: CandidateFunction
    obj: _ObjectFile
    symbol: _ElfSymbol
    address: int


class CandidateProgram:
    """Relocate an explicit reconstructed parser/helper closure."""

    @classmethod
    def link(
        cls,
        symbols: GameSymbols,
        functions: Sequence[CandidateFunction],
        *,
        hooks: Sequence[ExternalHook] = (),
        providers: Sequence[RetailProvider] = (),
        symbol_overrides: Mapping[str, int] | None = None,
        section_bases: Mapping[tuple[Path, str], int] | None = None,
        bind_data_objects: bool = False,
    ) -> LinkedProgram:
        if not functions:
            raise CandidateLinkError("candidate closure is empty")
        if bind_data_objects and section_bases:
            raise CandidateLinkError("object bindings cannot be combined with section placement")
        overrides = dict(symbol_overrides or {})
        explicit_section_bases = {
            (Path(path).resolve(), section): address
            for (path, section), address in (section_bases or {}).items()
        }
        objects = {item.object_path.resolve(): _load_object(item.object_path) for item in functions}

        selected: list[_Selected] = []
        selected_by_name: dict[str, _Selected] = {}
        code_cursor = CODE_BASE
        for request in functions:
            obj = objects[request.object_path.resolve()]
            symbol = obj.named_symbol(request.name)
            if symbol.section != ".text" or symbol.kind != "STT_FUNC" or symbol.size <= 0:
                raise CandidateLinkError(
                    f"{obj.path}: {request.name!r} is not an extended .text function"
                )
            if request.name in selected_by_name:
                raise CandidateLinkError(f"duplicate candidate function {request.name!r}")
            code_cursor = _align(code_cursor)
            if code_cursor + symbol.size > CODE_END:
                raise CandidateLinkError("candidate code arena exhausted")
            item = _Selected(request, obj, symbol, code_cursor)
            selected.append(item)
            selected_by_name[request.name] = item
            code_cursor += _align(symbol.size)

        provider_functions: list[LinkedFunction] = []
        providers_by_name: dict[str, LinkedFunction] = {}
        for provider in providers:
            if provider.name in selected_by_name or provider.name in providers_by_name:
                raise CandidateLinkError(f"duplicate linked identity {provider.name!r}")
            curated_address, curated_size = symbols.function(provider.name)
            address = curated_address if provider.address is None else provider.address
            size = curated_size if provider.size is None else provider.size
            if address != curated_address:
                raise CandidateLinkError(
                    f"provider {provider.name!r} address {address:#x} disagrees with "
                    f"curated GAME address {curated_address:#x}"
                )
            if size <= 0 or size > curated_size:
                raise CandidateLinkError(
                    f"provider {provider.name!r} size {size:#x} exceeds curated {curated_size:#x}"
                )
            linked = LinkedFunction(
                provider.name, address, size, "provider", provider.stack_words
            )
            provider_functions.append(linked)
            providers_by_name[provider.name] = linked

        private_hook_addresses: dict[str, int] = {}
        hook_bindings: list[_HookBinding] = []
        hook_cursor = HOOK_BASE
        for hook in hooks:
            if hook.name in private_hook_addresses:
                raise CandidateLinkError(f"duplicate external hook {hook.name!r}")
            private_hook_addresses[hook.name] = hook_cursor
            hook_bindings.append(_HookBinding(hook_cursor, hook))
            retail_hook_va = hook.retail_va
            if retail_hook_va is None and hook.name in symbols.functions:
                retail_hook_va = symbols.functions[hook.name][0]
            if retail_hook_va is not None:
                hook_bindings.append(_HookBinding(retail_hook_va, hook))
            hook_cursor += 12
            if hook_cursor > HOOK_END:
                raise CandidateLinkError("candidate hook arena exhausted")
        hook_addresses: dict[int, str] = {}
        for binding in hook_bindings:
            previous = hook_addresses.setdefault(binding.address, binding.hook.name)
            if previous != binding.hook.name:
                raise CandidateLinkError(
                    f"external hooks {previous!r} and {binding.hook.name!r} share "
                    f"entry {binding.address:#x}"
                )

        rodata_bases: dict[Path, int] = {}
        rodata_cursor = RODATA_BASE
        for path, obj in objects.items():
            size = obj.section_sizes.get(".rodata", 0)
            if not size:
                continue
            rodata_cursor = _align(rodata_cursor)
            if rodata_cursor + size > RODATA_END:
                raise CandidateLinkError("candidate rodata arena exhausted")
            rodata_bases[path] = rodata_cursor
            rodata_cursor += _align(size)

        data_bases: dict[tuple[Path, str], int] = dict(explicit_section_bases)
        known_data = {name: address for name, (address, _size) in symbols.data.items()}
        known_data.update(overrides)
        for path, obj in objects.items():
            for elf_symbol in obj.symbols:
                if elf_symbol.section not in {".data", ".bss", ".sdata", ".sbss"}:
                    continue
                if elf_symbol.name not in known_data:
                    continue
                if bind_data_objects:
                    if elf_symbol.kind != "STT_OBJECT" or elf_symbol.size <= 0:
                        raise CandidateLinkError(f"{path}: missing extent for {elf_symbol.name!r}")
                    if (elf_symbol.name in symbols.data
                            and elf_symbol.size != symbols.data[elf_symbol.name][1]):
                        raise CandidateLinkError(f"{path}: data extent differs for {elf_symbol.name!r}")
                    continue
                key = path, elf_symbol.section
                inferred = (known_data[elf_symbol.name] - elf_symbol.value) & 0xFFFFFFFF
                previous = data_bases.setdefault(key, inferred)
                if previous != inferred:
                    raise CandidateLinkError(
                        f"{path}: inconsistent {elf_symbol.section} base inferred from "
                        f"{elf_symbol.name!r}"
                    )

        def map_text_offset(obj: _ObjectFile, offset: int, *, soft: bool) -> int:
            for target in selected:
                if target.obj.path != obj.path:
                    continue
                start = target.symbol.value
                if start <= offset < start + target.symbol.size:
                    return target.address + offset - start
            # GCC sometimes expresses a same-unit function call through the
            # `.text` section symbol plus the callee's section offset.  Recover
            # the named identity before resolving it to an explicit hook or
            # unchanged provider; do not assume candidate/retail extents agree.
            named = next(
                (
                    item
                    for item in obj.symbols
                    if item.section == ".text"
                    and item.kind == "STT_FUNC"
                    and item.value == offset
                    and item.name
                ),
                None,
            )
            if named is not None and named.name in private_hook_addresses:
                return private_hook_addresses[named.name]
            if named is not None and named.name in providers_by_name:
                return providers_by_name[named.name].address
            if soft:
                return UNPATCHED_CODE_VA
            raise CandidateLinkError(
                f"{obj.path}: reference to unloaded .text offset {offset:#x}; "
                "add its function to the candidate closure or declare a provider/hook"
            )

        def resolve(
            owner: _Selected,
            symbol: _ElfSymbol,
            addend: int,
            *,
            soft_text: bool,
        ) -> int:
            if symbol.special_section == "SHN_UNDEF":
                name = symbol.name
                if name in selected_by_name:
                    return (selected_by_name[name].address + addend) & 0xFFFFFFFF
                if name in private_hook_addresses:
                    return (private_hook_addresses[name] + addend) & 0xFFFFFFFF
                if name in providers_by_name:
                    return (providers_by_name[name].address + addend) & 0xFFFFFFFF
                if name in overrides:
                    return (overrides[name] + addend) & 0xFFFFFFFF
                if name in known_data:
                    return (known_data[name] + addend) & 0xFFFFFFFF
                if name in symbols.functions:
                    raise CandidateLinkError(
                        f"{owner.obj.path}: candidate call/reference to GAME function {name!r} "
                        "is not in the candidate closure, providers, or hooks"
                    )
                raise CandidateLinkError(
                    f"{owner.obj.path}: unresolved candidate symbol {name!r}"
                )
            if symbol.special_section == "SHN_ABS":
                return (symbol.value + addend) & 0xFFFFFFFF
            if symbol.special_section == "SHN_COMMON":
                if not bind_data_objects:
                    raise CandidateLinkError(
                        f"{owner.obj.path}: COMMON {symbol.name!r} has no section placement; "
                        "isolated object binding must be explicit"
                    )
                return resolve_data_object(owner.obj, symbol, addend, known_data)
            if symbol.section == ".text":
                if symbol.name in selected_by_name and symbol.kind == "STT_FUNC":
                    target = selected_by_name[symbol.name]
                    return (target.address + addend) & 0xFFFFFFFF
                return map_text_offset(
                    owner.obj, symbol.value + addend, soft=soft_text
                )
            if symbol.section == ".rodata":
                try:
                    base = rodata_bases[owner.obj.path]
                except KeyError as error:
                    raise CandidateLinkError(
                        f"{owner.obj.path}: relocation names absent .rodata"
                    ) from error
                return (base + symbol.value + addend) & 0xFFFFFFFF
            if symbol.section in {".data", ".bss", ".sdata", ".sbss"}:
                if bind_data_objects:
                    return resolve_data_object(owner.obj, symbol, addend, known_data)
                key = owner.obj.path, symbol.section
                try:
                    base = data_bases[key]
                except KeyError as error:
                    raise CandidateLinkError(
                        f"{owner.obj.path}: no mapped base for owned section "
                        f"{symbol.section!r}; provide a curated symbol identity or section_bases"
                    ) from error
                return (base + symbol.value + addend) & 0xFFFFFFFF
            raise CandidateLinkError(
                f"{owner.obj.path}: unsupported symbol section {symbol.section!r} "
                f"for {symbol.name!r}"
            )

        def relocate(owner: _Selected, section_name: str, data: bytes) -> bytes:
            result = bytearray(data)
            relocs = sorted(
                (
                    item
                    for item in owner.obj.relocations
                    if item.section == section_name
                    and (
                        section_name != ".text"
                        or owner.symbol.value
                        <= item.offset
                        < owner.symbol.value + owner.symbol.size
                    )
                ),
                key=lambda item: item.offset,
            )
            pending_hi: dict[int, list[_ElfRelocation]] = defaultdict(list)
            for item in relocs:
                if item.offset < 0 or item.offset + 4 > len(result):
                    raise CandidateLinkError(
                        f"{owner.obj.path}: relocation {item.offset:#x} exceeds {section_name}"
                    )
                symbol = owner.obj.symbol(item.symbol_index)
                word = struct.unpack_from("<I", result, item.offset)[0]
                soft_text = section_name == ".rodata"
                if item.kind == R_MIPS_32:
                    target = resolve(owner, symbol, word, soft_text=soft_text)
                    struct.pack_into("<I", result, item.offset, target)
                elif item.kind == R_MIPS_26:
                    addend = (word & 0x03FFFFFF) << 2
                    target = resolve(owner, symbol, addend, soft_text=soft_text)
                    site = (
                        owner.address + item.offset - owner.symbol.value
                        if section_name == ".text"
                        else rodata_bases[owner.obj.path] + item.offset
                    )
                    if ((site + 4) ^ target) & 0xF0000000:
                        raise CandidateLinkError(
                            f"{owner.obj.path}: R_MIPS_26 at {item.offset:#x} cannot reach "
                            f"{target:#x} from {site:#x}"
                        )
                    struct.pack_into(
                        "<I",
                        result,
                        item.offset,
                        (word & 0xFC000000) | ((target >> 2) & 0x03FFFFFF),
                    )
                elif item.kind == R_MIPS_HI16:
                    pending_hi[item.symbol_index].append(item)
                elif item.kind == R_MIPS_LO16:
                    highs = pending_hi.pop(item.symbol_index, [])
                    if not highs:
                        raise CandidateLinkError(
                            f"{owner.obj.path}: unpaired R_MIPS_LO16 for {symbol.name!r}"
                        )
                    encoded_low: int | None = None
                    for high in highs:
                        high_word = struct.unpack_from("<I", result, high.offset)[0]
                        try:
                            addend = decode_hi_lo_target(high_word, word)
                            target = resolve(owner, symbol, addend, soft_text=soft_text)
                            new_high, new_low = encode_hi_lo_addend(high_word, word, target)
                        except ValueError as error:
                            raise CandidateLinkError(
                                f"{owner.obj.path}: invalid HI16/LO16 pair for "
                                f"{symbol.name!r}: {error}"
                            ) from error
                        if encoded_low is not None and encoded_low != new_low:
                            raise CandidateLinkError(
                                f"{owner.obj.path}: shared LO16 for {symbol.name!r} has "
                                "incompatible implicit addends"
                            )
                        struct.pack_into("<I", result, high.offset, new_high)
                        encoded_low = new_low
                    assert encoded_low is not None
                    struct.pack_into("<I", result, item.offset, encoded_low)
                else:
                    raise CandidateLinkError(
                        f"{owner.obj.path}: unsupported MIPS relocation {item.kind} "
                        f"for {symbol.name!r}"
                    )
            unpaired = [owner.obj.symbol(index).name for index, rows in pending_hi.items() if rows]
            if unpaired:
                raise CandidateLinkError(
                    f"{owner.obj.path}: unpaired R_MIPS_HI16 symbols {unpaired!r}"
                )
            return bytes(result)

        patches: list[MemoryPatch] = []
        for item in selected:
            original = item.obj.sections.get(".text")
            if original is None:
                raise CandidateLinkError(f"{item.obj.path}: missing .text")
            relocated = relocate(item, ".text", original)
            start = item.symbol.value
            patches.append(
                MemoryPatch(
                    f"candidate:{item.request.name}",
                    item.address,
                    relocated[start:start + item.symbol.size],
                )
            )

        # One relocated read-only clone per object is shared by its selected
        # functions.  The first selected owner is sufficient because text
        # offsets are mapped through the complete selected-function table.
        for path, base in rodata_bases.items():
            obj = objects[path]
            owner = next(item for item in selected if item.obj.path == path)
            patches.append(
                MemoryPatch(
                    f"candidate:{path.name}:.rodata",
                    base,
                    relocate(owner, ".rodata", obj.sections[".rodata"]),
                )
            )

        seen_data_patches: dict[int, bytes] = {}
        for path, obj in objects.items():
            owner = next(item for item in selected if item.obj.path == path)
            for section_name in (".data", ".sdata"):
                data = obj.sections.get(section_name)
                if not data:
                    continue
                if bind_data_objects:
                    relocated = relocate(owner, section_name, data)
                    for symbol in obj.symbols:
                        if symbol.section != section_name or symbol.kind != "STT_OBJECT":
                            continue
                        if symbol.size <= 0 or symbol.value + symbol.size > len(relocated):
                            raise CandidateLinkError(f"{path}: invalid initialized extent for {symbol.name!r}")
                        address = resolve_data_object(obj, symbol, 0, known_data)
                        payload = relocated[symbol.value:symbol.value + symbol.size]
                        for previous_address, previous in seen_data_patches.items():
                            if (address < previous_address + len(previous)
                                    and previous_address < address + len(payload)
                                    and (address != previous_address or payload != previous)):
                                raise CandidateLinkError("candidate initialized data objects overlap")
                        if address not in seen_data_patches:
                            patches.append(MemoryPatch(f"candidate:{path.name}:{symbol.name}",
                                                       address, payload))
                            seen_data_patches[address] = payload
                    continue
                key = path, section_name
                if key not in data_bases:
                    raise CandidateLinkError(
                        f"{path}: no mapped base for owned initialized section {section_name!r}"
                    )
                address = data_bases[key]
                relocated = relocate(owner, section_name, data)
                previous = seen_data_patches.setdefault(address, relocated)
                if previous != relocated:
                    raise CandidateLinkError(
                        f"candidate initialized-data patches disagree at {address:#x}"
                    )
                if previous is relocated:
                    patches.append(
                        MemoryPatch(f"candidate:{path.name}:{section_name}", address, relocated)
                    )

        linked_candidates = tuple(
            LinkedFunction(
                item.request.name,
                item.address,
                item.symbol.size,
                "candidate",
                item.request.stack_words,
            )
            for item in selected
        )
        return LinkedProgram(
            linked_candidates + tuple(provider_functions),
            tuple(patches),
            tuple(hook_bindings),
            "candidate",
            "objects" if bind_data_objects else "sections",
        )


class RetailProgram:
    """Declare the retail parser/helper closure admitted for one run."""

    @classmethod
    def link(
        cls,
        symbols: GameSymbols,
        functions: Sequence[str],
        *,
        hooks: Sequence[ExternalHook] = (),
        providers: Sequence[RetailProvider] = (),
        stack_words: Mapping[str, int] | None = None,
    ) -> LinkedProgram:
        stack = dict(stack_words or {})
        linked: list[LinkedFunction] = []
        seen: set[str] = set()
        for name, kind, explicit in (
            *((name, "retail", None) for name in functions),
            *((item.name, "provider", item) for item in providers),
        ):
            if name in seen:
                raise CandidateLinkError(f"duplicate retail closure identity {name!r}")
            seen.add(name)
            curated_address, curated_size = symbols.function(name)
            address = curated_address if explicit is None or explicit.address is None else explicit.address
            size = curated_size if explicit is None or explicit.size is None else explicit.size
            if address != curated_address or size <= 0 or size > curated_size:
                raise CandidateLinkError(f"invalid explicit retail extent for {name!r}")
            words = stack.get(name, 0) if explicit is None else explicit.stack_words
            linked.append(LinkedFunction(name, address, size, kind, words))
        bindings: list[_HookBinding] = []
        for hook in hooks:
            address = hook.retail_va
            if address is None:
                address, _size = symbols.function(hook.name)
            bindings.append(_HookBinding(address, hook))
        return LinkedProgram(tuple(linked), (), tuple(bindings), "retail")


class HookContext:
    """Controlled register and memory access offered to an external hook."""

    def __init__(self, machine: "ParserMachine", call: CallRecord) -> None:
        self._machine = machine
        self.call = call

    @property
    def args(self) -> tuple[int, int, int, int]:
        return self.call.args

    @property
    def stack_args(self) -> tuple[int, ...]:
        return self.call.stack_args

    def read(self, address: int, size: int) -> bytes:
        return self._machine._read(address, size)

    def write(self, address: int, data: bytes) -> None:
        self._machine._hook_write(address, data)

    def read_u32(self, address: int) -> int:
        return struct.unpack("<I", self.read(address, 4))[0]

    def write_u32(self, address: int, value: int) -> None:
        self.write(address, struct.pack("<I", value & 0xFFFFFFFF))

    def register(self, index: int) -> int:
        return self._machine._read_gpr(index)

    def set_register(self, index: int, value: int) -> None:
        self._machine._write_gpr(index, value)


@lru_cache(maxsize=1)
def _unicorn_api():
    try:
        from unicorn import (
            UC_ARCH_MIPS,
            UC_HOOK_CODE,
            UC_MODE_LITTLE_ENDIAN,
            UC_MODE_MIPS32,
            Uc,
            UcError,
        )
        from unicorn import mips_const
    except ImportError as error:
        raise RuntimeError(
            "Unicorn is unavailable; run parser-machine tests inside `nix develop`"
        ) from error
    return (
        Uc,
        UcError,
        UC_ARCH_MIPS,
        UC_MODE_MIPS32 | UC_MODE_LITTLE_ENDIAN,
        UC_HOOK_CODE,
        mips_const,
    )


def _physical(address: int, size: int = 1) -> int:
    physical = address & 0x1FFFFFFF if address & 0x80000000 else address
    if physical < 0 or size < 0 or physical + size > RAM_SIZE:
        raise ParserMachineError(
            f"RAM access {address:#010x}+{size:#x} is outside mapped PSX RAM"
        )
    return physical


def _canonical(address: int) -> int:
    return RAM_BASE | (address & 0x1FFFFFFF)


def _gpr_reads(word: int) -> set[int]:
    """Return GPRs read by one MIPS-I instruction (zero omitted)."""
    opcode = word >> 26
    rs = (word >> 21) & 31
    rt = (word >> 16) & 31
    funct = word & 63
    reads: set[int] = set()
    if opcode == 0:
        if funct in {0x00, 0x02, 0x03}:  # fixed shifts
            reads.add(rt)
        elif funct in {0x04, 0x06, 0x07}:
            reads.update((rs, rt))
        elif funct in {0x08, 0x09, 0x11, 0x13}:  # jr/jalr/mthi/mtlo
            reads.add(rs)
        elif funct not in {0x0C, 0x0D, 0x10, 0x12}:  # ALU, mult/div
            reads.update((rs, rt))
    elif opcode == 1:
        reads.add(rs)
    elif opcode in {2, 3, 0x0F}:  # j, jal, lui
        pass
    elif opcode in {4, 5}:
        reads.update((rs, rt))
    elif opcode in {6, 7}:
        reads.add(rs)
    elif opcode in {0x28, 0x29, 0x2A, 0x2B, 0x2E}:  # stores
        reads.update((rs, rt))
    elif opcode in {0x10, 0x12}:  # coprocessor moves/branches
        cop_rs = rs
        if cop_rs in {4, 6}:  # mtc/ctc consumes rt
            reads.add(rt)
    else:  # immediates and loads consume their base/source register
        reads.add(rs)
    reads.discard(0)
    return reads


def _loaded_gpr(word: int) -> int | None:
    opcode = word >> 26
    rt = (word >> 16) & 31
    if opcode in {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26}:
        return rt or None
    if opcode in {0x10, 0x12} and ((word >> 21) & 31) in {0, 2}:  # mfc/cfc
        return rt or None
    return None


def _store_record(
    word: int, pc: int, read_gpr: Callable[[int], int]
) -> WriteRecord | None:
    """Describe one MIPS-I little-endian integer store, without executing it."""
    opcode = word >> 26
    if opcode in {0x2C, 0x2D, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F}:
        raise ParserMachineError(
            f"unsupported coprocessor/non-MIPS-I store {word:#010x} at {pc:#010x}"
        )
    if opcode not in {0x28, 0x29, 0x2A, 0x2B, 0x2E}:
        return None
    rs = (word >> 21) & 31
    rt = (word >> 16) & 31
    immediate = word & 0xFFFF
    displacement = immediate if immediate < 0x8000 else immediate - 0x10000
    address = (read_gpr(rs) + displacement) & 0xFFFFFFFF
    value = read_gpr(rt)
    if opcode == 0x2A:  # SWL: high register bytes to low bytes of aligned word
        size = (address & 3) + 1
        address &= ~3
        value >>= (4 - size) * 8
    elif opcode == 0x2E:  # SWR: low register bytes to high bytes of aligned word
        size = 4 - (address & 3)
    else:
        size = {0x28: 1, 0x29: 2, 0x2B: 4}[opcode]
        if address & (size - 1):
            raise ParserMachineError(f"unaligned integer store at {pc:#010x}: {address:#010x}")
    _physical(address, size)
    return WriteRecord(pc, _canonical(address), size, value & ((1 << (size * 8)) - 1))


class ParserMachine:
    """Execute one linked retail or candidate GAME parser call at a time."""

    def __init__(
        self,
        retail: RetailImage,
        program: LinkedProgram,
        *,
        stack_va: int = DEFAULT_STACK_VA,
        audit_load_delays: bool = True,
    ) -> None:
        if retail.image != "GAME.EXE":
            raise ValueError(f"parser machine requires GAME.EXE, got {retail.image}")
        self.retail = retail
        self.program = program
        self.stack_va = stack_va
        self.audit_load_delays = audit_load_delays
        self._uc = None
        self._regs = None
        self._writes: list[WriteRecord] = []
        self._trace: list[CallRecord] = []
        self._allowed_writes: tuple[MemoryRange, ...] | None = None
        self._current_pc = 0

    def call(
        self,
        entry_name: str,
        args: Sequence[int] = (),
        *,
        memory: Sequence[MemoryInput] = (),
        capture: Sequence[MemoryRange] = (),
        allowed_writes: Sequence[MemoryRange] | None = None,
        initial_gprs: Mapping[int, int] | None = None,
        instruction_limit: int = DEFAULT_INSTRUCTION_LIMIT,
    ) -> ExecutionResult:
        if instruction_limit <= 0:
            raise ValueError("instruction_limit must be positive")
        self._validate_memory_inputs(memory)
        Uc, UcError, architecture, mode, hook_code, regs = _unicorn_api()
        self._regs = regs
        self._uc = Uc(architecture, mode)
        # CPU accesses KSEG0 virtual addresses.  This Unicorn build aliases
        # them to physical RAM, while its host mapping/read/write API does not.
        self._uc.mem_map(0, RAM_SIZE)
        load_image = self.retail.data[0x800:0x800 + self.retail.layout.load_size]
        self._write(self.retail.layout.load_address, load_image)
        for patch in self.program.patches:
            self._write(patch.address, patch.data)
        # The hook runs at the first NOP; the return and its delay slot then
        # execute normally, without stopping/restarting a delayed transfer.
        hook_stub = struct.pack("<3I", 0, 0x03E00008, 0)
        for binding in self.program.hooks:
            self._write(binding.address, hook_stub)
        for item in memory:
            self._write(item.address, item.data)

        self._writes = []
        self._trace = []
        self._allowed_writes = None if allowed_writes is None else tuple(allowed_writes)
        self._current_pc = 0
        functions_by_entry = {item.address: item for item in self.program.functions}
        hooks_by_entry = {item.address: item.hook for item in self.program.hooks}
        hook_delay_addresses = {
            address
            for item in self.program.hooks
            for address in (item.address + 4, item.address + 8)
        }
        if len(functions_by_entry) != len(self.program.functions):
            raise ParserMachineError(f"{self.program.label}: duplicate function entry address")
        if len(hooks_by_entry) != len(self.program.hooks):
            raise ParserMachineError(f"{self.program.label}: duplicate hook entry address")
        entry = self.program.address_of(entry_name)

        for index, value in enumerate(args[:4], start=4):
            self._write_gpr(index, value)
        self._write_gpr(29, self.stack_va)
        self._write_gpr(31, RETURN_VA)
        for index, value in (initial_gprs or {}).items():
            if not 0 <= index < 32:
                raise ValueError(f"initial GPR index {index} is outside 0..31")
            if index in {0, 4, 5, 6, 7, 29, 31}:
                raise ValueError(
                    f"initial GPR r{index} conflicts with zero/O32 args/stack/return setup"
                )
            self._write_gpr(index, value)
        for index, value in enumerate(args[4:]):
            self._write(
                self.stack_va + 16 + index * 4,
                struct.pack("<I", value & 0xFFFFFFFF),
            )

        instructions = 0
        pending_load: int | None = None
        pending_store: WriteRecord | None = None
        hook_error: Exception | None = None
        fault: str | None = None

        def read_call(linked_name: str, kind: str, address: int, stack_words: int) -> CallRecord:
            call_args = tuple(self._read_gpr(index) for index in range(4, 8))
            sp = self._read_gpr(29)
            stack_args = tuple(
                struct.unpack("<I", self._read(sp + 16 + index * 4, 4))[0]
                for index in range(stack_words)
            )
            return CallRecord(
                len(self._trace),
                linked_name,
                kind,
                address,
                call_args,  # type: ignore[arg-type]
                stack_args,
            )

        def verify_store() -> None:
            nonlocal pending_store
            if pending_store is None:
                return
            record = pending_store
            actual = self._read(record.address, record.size)
            if actual != record.value.to_bytes(record.size, "little"):
                raise ParserMachineFault(
                    f"store result differs from decoded instruction at {record.address:#010x}",
                    pc=record.pc,
                    instructions=instructions,
                )
            self._writes.append(record)
            pending_store = None

        def on_code(_uc, address, size, _user_data) -> None:
            nonlocal instructions, pending_load, pending_store, hook_error, fault
            verify_store()
            address = _canonical(address)
            self._current_pc = address
            if address == RETURN_VA:
                self._uc.emu_stop()
                return
            hook = hooks_by_entry.get(address)
            if hook is not None:
                if pending_load in {4, 5, 6, 7}:
                    fault = f"R3000 load-delay hazard into hook argument r{pending_load}"
                    self._uc.emu_stop()
                    return
                call_record = read_call(hook.name, "hook", address, hook.stack_words)
                self._trace.append(call_record)
                try:
                    returned = hook.handler(HookContext(self, call_record))
                    if isinstance(returned, int):
                        returned = HookReturn(v0=returned)
                    elif isinstance(returned, tuple):
                        returned = HookReturn(*returned)
                    if returned is not None:
                        if returned.v0 is not None:
                            self._write_gpr(2, returned.v0)
                        if returned.v1 is not None:
                            self._write_gpr(3, returned.v1)
                except Exception as error:
                    hook_error = error
                    self._uc.emu_stop()
                    return
                pending_load = None
                instructions += 1
                return
            linked = functions_by_entry.get(address)
            if linked is not None:
                self._trace.append(
                    read_call(linked.name, linked.kind, address, linked.stack_words)
                )
            if (
                address not in hook_delay_addresses
                and not any(item.contains(address, size) for item in self.program.functions)
            ):
                fault = "instruction fetch outside declared parser/provider closure"
                self._uc.emu_stop()
                return
            word = struct.unpack("<I", self._read(address, 4))[0]
            if self.audit_load_delays and pending_load in _gpr_reads(word):
                fault = f"R3000 load-delay hazard reading r{pending_load}"
                self._uc.emu_stop()
                return
            pending_load = _loaded_gpr(word)
            pending_store = _store_record(word, address, self._read_gpr)
            if pending_store is not None:
                self._check_write(pending_store.address, pending_store.size)
            instructions += 1

        self._uc.hook_add(hook_code, on_code)
        self._write_pc(entry)
        while True:
            pc = self._read_pc()
            if pc == RETURN_VA:
                break
            if instructions >= instruction_limit:
                raise ParserMachineFault(
                    f"instruction limit {instruction_limit} reached",
                    pc=pc,
                    instructions=instructions,
                )
            hook_error = None
            fault = None
            try:
                self._uc.emu_start(pc, RETURN_VA, count=instruction_limit - instructions)
            except UcError as error:
                pc = self._read_pc()
                raise ParserMachineFault(
                    f"Unicorn fault: {error}", pc=pc, instructions=instructions
                ) from error
            pc = self._read_pc()
            if fault is not None:
                raise ParserMachineFault(fault, pc=self._current_pc, instructions=instructions)
            if hook_error is not None:
                raise hook_error
            verify_store()
            if pc == RETURN_VA:
                break
            if instructions >= instruction_limit:
                continue
            raise ParserMachineFault(
                "emulation stopped without return or hook",
                pc=pc,
                instructions=instructions,
            )

        snapshots = tuple(
            MemorySnapshot(item.name, item.address, self._read(item.address, item.size))
            for item in capture
        )
        return ExecutionResult(
            self._read_gpr(2),
            self._read_gpr(3),
            snapshots,
            tuple(self._trace),
            tuple(self._writes),
            instructions,
            self._read_pc(),
        )

    def _validate_memory_inputs(self, memory: Sequence[MemoryInput]) -> None:
        """Keep caller data from replacing the declared executable closure.

        Addresses are compared after KSEG translation because Unicorn maps
        GAME's high virtual addresses and low physical aliases to the same PSX
        RAM. Only this program's functions, relocation patches, and hook stubs
        are protected; unrelated bytes in the retail image remain valid parser
        inputs.
        """

        protected = [
            (f"executable function {item.name!r}", item.address, item.size)
            for item in self.program.functions
        ]
        protected.extend(
            (f"linked patch {item.name!r}", item.address, len(item.data))
            for item in self.program.patches
        )
        protected.extend(
            (f"hook stub {item.hook.name!r}", item.address, 12)
            for item in self.program.hooks
        )
        for value in memory:
            input_size = len(value.data)
            input_start = _physical(value.address, input_size)
            input_end = input_start + input_size
            for label, address, size in protected:
                if input_size == 0 or size == 0:
                    continue
                protected_start = _physical(address, size)
                protected_end = protected_start + size
                if input_start < protected_end and protected_start < input_end:
                    raise ParserMachineError(
                        f"memory input {value.address:#010x}+{input_size:#x} overlaps "
                        f"{label} at {address:#010x}+{size:#x}"
                    )

    def _read(self, address: int, size: int) -> bytes:
        return bytes(self._uc.mem_read(_physical(address, size), size))

    def _write(self, address: int, data: bytes) -> None:
        self._uc.mem_write(_physical(address, len(data)), data)

    def _read_gpr(self, index: int) -> int:
        register = self._regs.UC_MIPS_REG_0 + index
        return int(self._uc.reg_read(register)) & 0xFFFFFFFF

    def _write_gpr(self, index: int, value: int) -> None:
        register = self._regs.UC_MIPS_REG_0 + index
        self._uc.reg_write(register, value & 0xFFFFFFFF)

    def _read_pc(self) -> int:
        return int(self._uc.reg_read(self._regs.UC_MIPS_REG_PC)) & 0xFFFFFFFF

    def _write_pc(self, value: int) -> None:
        self._uc.reg_write(self._regs.UC_MIPS_REG_PC, value & 0xFFFFFFFF)

    def _hook_write(self, address: int, data: bytes) -> None:
        for offset, value in enumerate(data):
            self._check_write(address + offset, 1)
            self._writes.append(
                WriteRecord(self._current_pc, address + offset, 1, value)
            )
        self._write(address, data)

    def _check_write(self, address: int, size: int) -> None:
        if self._allowed_writes is None:
            return
        stack_allowed = self.stack_va - 0x10000 <= address and address + size <= RAM_BASE + RAM_SIZE
        explicitly_allowed = any(
            item.address <= address and address + size <= item.address + item.size
            for item in self._allowed_writes
        )
        if not stack_allowed and not explicitly_allowed:
            raise ParserMachineFault(
                f"write outside allowed ranges: {address:#010x}+{size:#x}",
                pc=self._current_pc,
                instructions=len(self._writes),
            )
