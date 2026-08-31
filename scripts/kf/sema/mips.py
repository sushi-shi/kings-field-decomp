"""Small MIPS-I control decoder used for CFG and xref truth.

GNU objdump remains the display disassembler.  This module deliberately owns
control semantics so basic-block boundaries do not depend on parsing prose and
so every transfer keeps its architectural delay slot.
"""

from __future__ import annotations

from dataclasses import dataclass

from scripts.kf.relocations import decode_mips26_target


@dataclass(frozen=True)
class Control:
    kind: str
    mnemonic: str
    target: int | None
    delay_slot: bool
    conditional: bool = False
    call: bool = False
    annulled: bool = False

    @property
    def terminates_block(self) -> bool:
        return self.conditional or self.kind in {
            "branch",
            "jump",
            "return",
            "indirect-jump",
            "trap",
        }


@dataclass(frozen=True)
class Instruction:
    va: int
    word: int
    control: Control | None


def sign_extend_16(value: int) -> int:
    return value if value < 0x8000 else value - 0x10000


def branch_target(va: int, word: int) -> int:
    return (va + 4 + (sign_extend_16(word & 0xFFFF) << 2)) & 0xFFFFFFFF


def _branch(va: int, word: int, mnemonic: str, *, call: bool = False,
            annulled: bool = False) -> Control:
    return Control(
        "call" if call else "branch",
        mnemonic,
        branch_target(va, word),
        True,
        conditional=True,
        call=call,
        annulled=annulled,
    )


def decode_control(va: int, word: int) -> Control | None:
    opcode = word >> 26
    rs = (word >> 21) & 0x1F
    rt = (word >> 16) & 0x1F
    rd = (word >> 11) & 0x1F
    funct = word & 0x3F

    if opcode == 0:
        if funct == 0x08:  # jr
            if rs == 31:
                return Control("return", "jr", None, True)
            return Control("indirect-jump", "jr", None, True)
        if funct == 0x09:  # jalr
            if rd == 0:
                return Control("indirect-jump", "jalr", None, True)
            return Control("indirect-call", "jalr", None, True, call=True)
        if funct in {0x0C, 0x0D}:
            return Control("trap", "syscall" if funct == 0x0C else "break", None, False)
        return None

    if opcode == 0x02:
        return Control("jump", "j", decode_mips26_target(va, word), True)
    if opcode == 0x03:
        return Control(
            "call", "jal", decode_mips26_target(va, word), True, call=True
        )

    branches = {
        0x04: "beq",
        0x05: "bne",
        0x06: "blez",
        0x07: "bgtz",
        0x14: "beql",
        0x15: "bnel",
        0x16: "blezl",
        0x17: "bgtzl",
    }
    if opcode in branches:
        return _branch(va, word, branches[opcode], annulled=opcode >= 0x14)

    if opcode == 0x01:
        regimm = {
            0x00: ("bltz", False, False),
            0x01: ("bgez", False, False),
            0x02: ("bltzl", False, True),
            0x03: ("bgezl", False, True),
            0x10: ("bltzal", True, False),
            0x11: ("bgezal", True, False),
            0x12: ("bltzall", True, True),
            0x13: ("bgezall", True, True),
        }.get(rt)
        if regimm is not None:
            mnemonic, call, annulled = regimm
            return _branch(va, word, mnemonic, call=call, annulled=annulled)

    # COPz branch: opcode identifies the coprocessor and rs=8 is BCz.
    if opcode in {0x10, 0x11, 0x12, 0x13} and rs == 0x08:
        cop = opcode - 0x10
        truth = "t" if rt & 1 else "f"
        annulled = bool(rt & 2)
        mnemonic = f"bc{cop}{truth}" + ("l" if annulled else "")
        return _branch(va, word, mnemonic, annulled=annulled)
    return None


def decode_words(va: int, payload: bytes) -> tuple[Instruction, ...]:
    size = len(payload) - (len(payload) % 4)
    return tuple(
        Instruction(
            va + offset,
            int.from_bytes(payload[offset:offset + 4], "little"),
            None,
        )
        for offset in range(0, size, 4)
    )


def load_delay_register(word: int) -> int | None:
    """Destination whose loaded value is unavailable to the next instruction."""
    opcode = word >> 26
    # LWCz opcodes load coprocessor registers, not a delayed GPR result.
    if opcode in {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26}:
        return (word >> 16) & 0x1F
    return None


def with_controls(instructions: tuple[Instruction, ...]) -> tuple[Instruction, ...]:
    return tuple(
        Instruction(row.va, row.word, decode_control(row.va, row.word))
        for row in instructions
    )
