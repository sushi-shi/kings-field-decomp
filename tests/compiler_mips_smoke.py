"""End-to-end native GCC 2.6.0/2.5.7 PSX -> maspsx -> GNU-as smoke test."""

from __future__ import annotations

import json
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.kf.compile import compile_source
from scripts.kf.retail import write_tsv


ORDER_SOURCE = """\
static __inline__ int inline_candidate(int value) { return value - 7; }
int third(int value) { return inline_candidate(value * 3 + 8); }
int first(int value) { return third(value) + 5; }
int second(int value) { return first(value) - 2; }
"""


PROBE_257_SOURCE = """\
extern void callee(int value);
int ratio(int value, int span) { return (value << 6) / (span + 1) + 1; }
void framed(int value) { callee(value); callee(value + 1); }
"""

DATA_CLAIM_SOURCE = """\
#define DATA(va, size)
typedef unsigned long u32;
DATA(0x80057b0c, 0x4)
static u32 counter = 0;
DATA(0x80057b10, 0x10)
u32 table[4] = {1, 2, 3, 4};
void tick(void) { counter += table[1]; }
"""

TENTATIVE_SOURCE = """\
#define DATA(va, size)
DATA(0x80060000, 0x4)
static int counter;
DATA(0x80060008, 0x4)
int public_word;
int tick(int value) { counter += value; public_word = counter; return counter; }
"""


def symbol_rows(path: Path) -> list[list[str]]:
    output = subprocess.run(
        ["mipsel-linux-gnu-readelf", "-sW", str(path)],
        capture_output=True, text=True, check=True,
    ).stdout
    return [parts for line in output.splitlines()
            if len(parts := line.split()) >= 8 and parts[0].endswith(":")
            and parts[0][:-1].isdigit()]


def tentative_controls(root: Path) -> None:
    for version in ("257", "260"):
        outputs = []
        for unit in ("a", "b"):
            source = root / f"allocation-{version}-{unit}.c"
            output = source.with_suffix(".o")
            source.write_text(TENTATIVE_SOURCE.replace("public_word", f"public_{unit}")
                              .replace("tick", f"tick_{unit}"), encoding="utf-8")
            compile_source(source, "GAME.EXE", output, root / "delink",
                           optimization="O2", compiler=f"gcc{version}-native")
            objects = object_symbols(output)
            if objects.get("counter", ("", 0, ""))[:2] != ("LOCAL", 4):
                raise RuntimeError(f"GCC {version} private tentative datum exported: {objects}")
            if objects.get(f"public_{unit}", ("", 0, ""))[:2] != ("GLOBAL", 4):
                raise RuntimeError(f"GCC {version} public tentative datum hidden: {objects}")
            if objects["counter"][2] != objects[f"public_{unit}"][2]:
                raise RuntimeError("G0 tentative allocations no longer share .bss")
            offsets = {row[7]: int(row[1], 16) for row in symbol_rows(output)}
            # Raw probes emit .lcomm/.comm sizes 8 (2.5.7) versus 4 (2.6.0).
            allocation = 8 if version == "257" else 4
            if offsets["counter"] != 0 or offsets[f"public_{unit}"] != allocation:
                raise RuntimeError(f"GCC {version} tentative allocation offsets changed: {offsets}")
            outputs.append(output)

        # A third TU must resolve the exported object, but cannot see either
        # private counter. Two same-named statics must coexist without merging.
        consumer = root / f"consumer-{version}.o"
        subprocess.run([
            "mipsel-linux-gnu-as", "-march=r3000", "-mabi=32", "-G0",
            "-o", str(consumer),
        ], input=".data\n.word counter\n.word public_a\n", text=True, check=True)
        linked = root / f"linked-{version}.o"
        subprocess.run(["mipsel-linux-gnu-ld", "-r", "-o", str(linked),
                        *(str(path) for path in outputs), str(consumer)],
                       capture_output=True, text=True, check=True)
        rows = symbol_rows(linked)
        private = [row for row in rows if row[7] == "counter" and row[4] == "LOCAL"]
        if len(private) != 2 or private[0][1] == private[1][1]:
            raise RuntimeError(f"linker merged or lost private allocations: {private}")
        undefined = {row[7] for row in rows if row[6] == "UND"}
        if undefined != {"counter"}:
            raise RuntimeError(f"private/exported cross-TU resolution is wrong: {undefined}")


def assembler_binding_controls(root: Path) -> None:
    # Exercise both allocation classes and the adapter's opt-in COMMON paths.
    # G8 here tests the adapter, not the project's G0 compiler/profile choice.
    source = ".comm exported,8\n.lcomm private,8\n"
    source += ".type exported,@object\n.type private,@object\n"
    for limit in (0, 8):
        for mode, flags in enumerate(((), ("--use-comm-section",),
                                     ("--use-comm-section", "--use-comm-for-lcomm"))):
            output = root / f"binding-{limit}-{mode}.o"
            subprocess.run([
                "maspsx", "--aspsx-version=1.07", *flags,
                "--run-assembler", "--force-stdin", "-march=r3000", "-mabi=32",
                f"-G{limit}", "-o", str(output),
            ], input=source, capture_output=True, text=True, check=True)
            objects = object_symbols(output)
            if objects.get("private", ("", 0, ""))[0] != "LOCAL":
                raise RuntimeError(f"G{limit} {flags}: .lcomm exported: {objects}")
            if objects.get("exported", ("", 0, ""))[0] != "GLOBAL":
                raise RuntimeError(f"G{limit} {flags}: .comm hidden: {objects}")
            if (objects["exported"][2] == "COM") != bool(flags):
                raise RuntimeError(f"COMMON option changed allocation class: {objects}")
            if objects["private"][2] in {"COM", "UND"}:
                raise RuntimeError(f"private datum is not locally allocated: {objects}")
            if not flags:
                table = subprocess.run(["mipsel-linux-gnu-objdump", "-t", str(output)],
                                       capture_output=True, text=True, check=True).stdout
                rows = {parts[-1]: parts for line in table.splitlines()
                        if len(parts := line.split()) == 6}
                section = ".sbss" if limit else ".bss"
                for name, offset in (("exported", 0), ("private", 8)):
                    if rows[name][3] != section or int(rows[name][0], 16) != offset:
                        raise RuntimeError(f"G{limit} allocation layout changed: {rows[name]}")


def elf_sections(path: Path) -> dict[str, tuple[int, int, bytes]]:
    """Section size, alignment and payload from the controlled ELF32-LE output."""
    blob = path.read_bytes()
    offset = struct.unpack_from('<I', blob, 32)[0]
    stride, count, names_index = struct.unpack_from('<HHH', blob, 46)
    rows = [struct.unpack_from('<10I', blob, offset + i * stride) for i in range(count)]
    names_row = rows[names_index]
    names = blob[names_row[4]:names_row[4] + names_row[5]]
    return {names[r[0]:names.index(b'\0', r[0])].decode():
            (r[5], r[8], b'' if r[1] == 8 else blob[r[4]:r[4] + r[5]]) for r in rows}


def section_extent_controls(root: Path) -> None:
    source = root / 'section-native.c'
    source.write_text('unsigned char payload[9] = {1, 0, 2, 0, 3, 0, 0, 0, 0};\n'
                      'const unsigned char message[5] = {120, 121, 0, 0, 0};\n'
                      'unsigned char tentative[9];\n'
                      'int first(void) { return payload[0]; }\n')
    for version in ('257', '260'):
        output = root / f'section-{version}.o'
        compile_source(source, 'GAME.EXE', output, root / 'delink',
                       optimization='O2', compiler=f'gcc{version}-native')
        sections = elf_sections(output)
        for name, payload in (('.data', bytes((1, 0, 2, 0, 3, 0, 0, 0, 0))),
                              ('.rodata', b'xy\0\0\0')):
            if sections[name][0] != len(payload) or sections[name][2] != payload:
                raise RuntimeError(f'GCC {version}: automatic tail or lost explicit zeros: '
                                   f'{name}: {sections[name]}')
        # Compiler-specified COMMON rounding is real input, not a GAS tail.
        allocation = 16 if version == '257' else 9
        if sections['.bss'][:2] != (allocation, 4):
            raise RuntimeError(f'GCC {version}: tentative allocation/alignment changed: {sections}')
        if sections['.data'][1] != 4:
            raise RuntimeError('C analysis object must preserve the native ASPSX section constraint')
        metadata = json.loads(output.with_suffix('.o.json').read_text())
        if metadata.get('gnu_as_section_flags') != ['-no-pad-sections']:
            raise RuntimeError('object metadata does not identify its section-padding contract')
        if metadata.get('section_alignment', {}).get('method') != 'aspsx-1.07-section-declarations':
            raise RuntimeError('object metadata does not identify its native section constraint')

    # Diagnostic assembly, not a game reconstruction: retain an explicit final
    # NOP, .space bytes, interior alignment padding and a relocated addend.
    assembly = root / 'section-asm.s'
    assembly.write_text('.text\n.set noreorder\njr $31\nnop\nnop\n'
                        '.data\n.byte 127\n.balign 8\n.word 0\n'
                        '.bss\n.space 9\n'
                        '.section .rodata\n.balign 8\n.word external+4\n.byte 0\n')
    output = root / 'section-asm.o'
    compile_source(assembly, 'GAME.EXE', output, root / 'delink')
    sections = elf_sections(output)
    expected = {'.text': (12, 16, struct.pack('<3I', 0x03E00008, 0, 0)),
                '.data': (12, 16, b'\x7f' + bytes(11)),
                '.bss': (9, 16, b''), '.rodata': (5, 8, struct.pack('<I', 4) + b'\0')}
    for name, value in expected.items():
        if sections[name] != value:
            raise RuntimeError(f'explicit section contents/alignment changed: {name}: {sections[name]}')
    if len(sections['.rel.rodata'][2]) != 8:
        raise RuntimeError('no-tail-padding lost the R_MIPS_32 relocation')
    offset, info = struct.unpack('<II', sections['.rel.rodata'][2])
    if (offset, info & 0xFF) != (0, 2):
        raise RuntimeError('no-tail-padding changed the data relocation offset/type')
    symbol = sections['.symtab'][2][(info >> 8) * 16:((info >> 8) + 1) * 16]
    name_offset = struct.unpack_from('<I', symbol)[0]
    name = sections['.strtab'][2][name_offset:].split(b'\0', 1)[0]
    if name != b'external' or struct.unpack_from('<H', symbol, 14)[0] != 0:
        raise RuntimeError('no-tail-padding changed the unresolved data referent')


def text_words(path: Path) -> list[int]:
    data = subprocess.run(
        ["mipsel-linux-gnu-objcopy", "-O", "binary", "--only-section=.text",
         str(path), "/dev/stdout"],
        capture_output=True,
        check=True,
    ).stdout
    return list(struct.unpack(f"<{len(data) // 4}I", data[: len(data) // 4 * 4]))


def object_symbols(path: Path) -> dict[str, tuple[str, int, str]]:
    """name -> (binding, size, section) for OBJECT-typed symbols."""
    output = subprocess.run(
        ["mipsel-linux-gnu-readelf", "-s", "-W", str(path)],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    symbols = {}
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 8 and parts[3] == "OBJECT":
            symbols[parts[7]] = (parts[4], int(parts[2]), parts[6])
    return symbols


def function_symbols(path: Path) -> list[tuple[int, str]]:
    output = subprocess.run(
        ["mipsel-linux-gnu-nm", "-n", "--defined-only", str(path)],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    symbols = []
    for line in output.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in {"t", "T"}:
            symbols.append((int(parts[0], 16), parts[2]))
    return symbols


def main() -> int:
    for tool in (
        "cpppsx-260",
        "cc1psx-260",
        "cpppsx-257",
        "cc1psx-257",
        "maspsx",
        "mipsel-linux-gnu-as",
        "mipsel-linux-gnu-nm",
        "mipsel-linux-gnu-readelf",
        "mipsel-linux-gnu-objdump",
        "mipsel-linux-gnu-ld",
    ):
        if shutil.which(tool) is None:
            raise RuntimeError(f"{tool} is missing; run inside nix develop")

    with tempfile.TemporaryDirectory(prefix="kf-compiler-mips-") as directory:
        root = Path(directory)
        source = root / "simple.c"
        output = root / "simple.o"
        order_source = root / "order.c"
        order_o0 = root / "order-o0.o"
        order_o2 = root / "order-o2.o"
        probe_source = root / "probe257.c"
        probe_257 = root / "probe257.o"
        claim_source = root / "claims.c"
        claim_object = root / "claims.o"
        manifest = root / "delink/game/objects.tsv"
        source.write_text(
            "int add(int left, int right) { return left + right; }\n",
            encoding="utf-8",
        )
        write_tsv(
            manifest,
            (
                "image", "va", "size", "body_size", "name", "scope",
                "provider", "library", "object", "relocations", "confidence",
                "provenance",
            ),
            tuple({
                "image": "GAME.EXE",
                "va": f"{0x80010000 + index * 4:#x}",
                "size": "0x4",
                "body_size": "0x4",
                "name": Path(name).stem,
                "scope": "decomp",
                "provider": "",
                "library": "",
                "object": f"objects/{name}",
                "relocations": 0,
                "confidence": "test",
                "provenance": "test",
            } for index, name in enumerate((
                "simple.o", "order-o0.o", "order-o2.o", "probe257.o", "claims.o",
                "allocation-257-a.o", "allocation-257-b.o",
                "allocation-260-a.o", "allocation-260-b.o",
                "section-257.o", "section-260.o", "section-asm.o",
            ))),
            (),
        )
        compile_source(
            source,
            "GAME.EXE",
            output,
            root / "delink",
            optimization="O2",
        )
        data = output.read_bytes()
        if data[:7] != b"\x7fELF\x01\x01\x01":
            raise RuntimeError("compiler pipeline did not emit ELF32 little-endian")
        if struct.unpack_from("<H", data, 18)[0] != 8:
            raise RuntimeError("compiler pipeline did not emit EM_MIPS")
        symbols = subprocess.run(
            ["mipsel-linux-gnu-nm", str(output)],
            capture_output=True,
            text=True,
            check=True,
        ).stdout
        if " add" not in symbols:
            raise RuntimeError(f"compiled add symbol is missing: {symbols}")

        order_source.write_text(ORDER_SOURCE, encoding="utf-8")
        compile_source(
            order_source,
            "GAME.EXE",
            order_o0,
            root / "delink",
            optimization="O0",
        )
        compile_source(
            order_source,
            "GAME.EXE",
            order_o2,
            root / "delink",
            optimization="O2",
        )
        o0_symbols = function_symbols(order_o0)
        o2_symbols = function_symbols(order_o2)
        expected_order = ["third", "first", "second"]
        probe_names = {"inline_candidate", *expected_order}
        o0_names = [name for _address, name in o0_symbols if name in probe_names]
        o2_names = [name for _address, name in o2_symbols if name in probe_names]
        if "inline_candidate" not in o0_names:
            raise RuntimeError(f"GCC 2.6.0 -O0 omitted the inline body: {o0_symbols}")
        if [name for name in o0_names if name != "inline_candidate"] != expected_order:
            raise RuntimeError(f"GCC 2.6.0 -O0 changed source function order: {o0_symbols}")
        if o2_names != expected_order:
            raise RuntimeError(
                "GCC 2.6.0 -O2 did not preserve exported source order or omit the "
                f"inlined static body: {o2_symbols}"
            )
        o0_offsets = {name: address for address, name in o0_symbols}
        o2_offsets = {name: address for address, name in o2_symbols}
        if all(o0_offsets[name] == o2_offsets[name] for name in expected_order[1:]):
            raise RuntimeError(
                "optimization did not move either downstream function offset; "
                "the shift calibration is ineffective"
            )

        # GCC 2.5.7 probe calibration against two retail-observed forms: every
        # framed GAME.EXE epilogue restores $sp in the `jr $ra` delay slot, and
        # signed division carries the checked ASPSX expansion (`break 7`).
        probe_source.write_text(PROBE_257_SOURCE, encoding="utf-8")
        compile_source(
            probe_source,
            "GAME.EXE",
            probe_257,
            root / "delink",
            optimization="O2",
            compiler="gcc257-native",
            maspsx_flags=("--expand-div",),
        )
        words = text_words(probe_257)
        jr_ra = 0x03E00008
        returns = [index for index, word in enumerate(words) if word == jr_ra]
        if not returns:
            raise RuntimeError("GCC 2.5.7 probe object has no jr $ra")
        framed = [
            index for index in returns
            if index + 1 < len(words) and words[index + 1] >> 16 == 0x27BD
        ]
        if not framed:
            raise RuntimeError(
                "GCC 2.5.7 probe did not restore $sp in the jr $ra delay slot"
            )
        if 0x0007000D not in words:
            raise RuntimeError("--expand-div did not emit the break 7 divide check")

        # DATA() claims: the compiler prints no data sizes, so kf-compile
        # annotates claimed symbols with the claimed size and object type.
        claim_source.write_text(DATA_CLAIM_SOURCE, encoding="utf-8")
        compile_source(
            claim_source,
            "GAME.EXE",
            claim_object,
            root / "delink",
            optimization="O2",
            compiler="gcc257-native",
            maspsx_flags=("--expand-div",),
        )
        objects = object_symbols(claim_object)
        if objects.get("counter") != ("LOCAL", 4, "3"):
            raise RuntimeError(f"claimed static datum is not a sized local object: {objects}")
        if objects.get("table", ("", 0, ""))[:2] != ("GLOBAL", 16):
            raise RuntimeError(f"claimed initialized table is not a sized global: {objects}")
        if objects["table"][2] != objects["counter"][2]:
            raise RuntimeError("claimed data did not land in one .data section")
        tentative_controls(root)
        assembler_binding_controls(root)
        section_extent_controls(root)
    print(
        "GCC 2.6.0 PSX C calibration: MIPS ELF, source-order emission at O0/O2, "
        "O2 inlined-static omission, and downstream offset shifts; "
        "GCC 2.5.7 probe: delay-slot $sp restore, checked div expansion, "
        "compiler-sized DATA() symbols; both compilers: private tentative storage "
        "and cross-TU linkage; six BSS/small-BSS/COMMON adapter binding controls; "
        "explicit section extents, zero tails, relocation addends and retained ELF alignments"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
