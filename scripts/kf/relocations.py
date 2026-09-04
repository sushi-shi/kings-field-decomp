"""Pure validation and encoding rules for inferred PS-X MIPS relocations.

The linked executables do not contain a relocation directory.  Both the
delinker and semantic navigator therefore use this module to distinguish a
byte-validated relocation candidate from weaker investigation evidence.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass

from scripts.kf.retail import parse_int


SIGNED_LOW_OPCODES = {
    0x08,  # addi
    0x09,  # addiu
    0x20,  # lb
    0x21,  # lh
    0x23,  # lw
    0x24,  # lbu
    0x25,  # lhu
    0x28,  # sb
    0x29,  # sh
    0x2B,  # sw
}


@dataclass(frozen=True)
class RelocationValidation:
    """Decoded fields shared by relocation consumers after validation."""

    site: int
    target: int
    offset: int
    paired_site: int | None = None
    paired_offset: int | None = None
    word: int = 0
    paired_word: int | None = None


def decode_mips26_target(site_va: int, word: int) -> int:
    """Decode a J/JAL target at its linked virtual address."""
    return ((site_va + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def encode_mips26_addend(word: int, addend: int) -> int:
    if addend & 3:
        raise ValueError(f"unaligned MIPS26 addend {addend:#x}")
    return (word & 0xFC000000) | ((addend >> 2) & 0x03FFFFFF)


def decode_hi_lo_target(hi_word: int, lo_word: int) -> int:
    if hi_word >> 26 != 0x0F:
        raise ValueError("HI site is not LUI")
    hi_register = (hi_word >> 16) & 0x1F
    low_opcode = lo_word >> 26
    if (lo_word >> 21) & 0x1F != hi_register:
        raise ValueError("LO instruction does not consume the LUI register")
    low = lo_word & 0xFFFF
    if low_opcode == 0x0D:  # ori
        return ((hi_word & 0xFFFF) << 16) | low
    if low_opcode not in SIGNED_LOW_OPCODES:
        raise ValueError(f"unsupported LO opcode {low_opcode:#x}")
    signed_low = low if low < 0x8000 else low - 0x10000
    return (((hi_word & 0xFFFF) << 16) + signed_low) & 0xFFFFFFFF


def encode_hi_lo_addend(hi_word: int, lo_word: int, addend: int) -> tuple[int, int]:
    """Encode the implicit addend used by an ELF HI16/LO16 REL pair."""
    low_opcode = lo_word >> 26
    if low_opcode not in SIGNED_LOW_OPCODES:
        raise ValueError("ELF HI16/LO16 requires a sign-extending low instruction")
    high = ((addend + 0x8000) >> 16) & 0xFFFF
    low = addend & 0xFFFF
    return (hi_word & 0xFFFF0000) | high, (lo_word & 0xFFFF0000) | low


def policy_reason(row: dict[str, str], policy: str) -> str | None:
    if row["status"] == "rejected":
        return "candidate-rejected"
    if policy == "reviewed" and row["status"] != "reviewed":
        return "not-manually-reviewed"
    if (
        policy in {"safe", "reviewed"}
        and row["channel"] != "reachable-code"
        and row["status"] != "reviewed"
    ):
        return "non-reachable-code-channel"
    return None


def _word(blob: bytes | bytearray, offset: int) -> int:
    if offset < 0 or offset + 4 > len(blob):
        raise ValueError("site-outside-function-extent")
    return struct.unpack_from("<I", blob, offset)[0]


def validate_relocation(
    blob: bytes | bytearray,
    function,
    row: dict[str, str],
    catalog,
    policy: str,
) -> RelocationValidation:
    """Validate one row against linked bytes without rewriting those bytes.

    ``function`` and ``catalog`` intentionally use structural typing so this
    evidence primitive remains below the delinker's object model.
    """
    reason = policy_reason(row, policy)
    if reason:
        raise ValueError(reason)

    site = parse_int(row["site_va"])
    target = parse_int(row["target_va"])
    offset = site - function.va
    word = _word(blob, offset)

    if row["kind"] == "mips26":
        expected_opcode = {"j": 2, "jal": 3}.get(row["opcode"])
        if expected_opcode is None or word >> 26 != expected_opcode:
            raise ValueError("instruction-opcode-mismatch")
        if decode_mips26_target(site, word) != target:
            raise ValueError("decoded-target-mismatch")
        target_function = catalog.function_starts[function.image].get(target)
        if not function.contains(target) and target_function is None and policy != "all":
            raise ValueError("target-is-not-a-function-start")
        return RelocationValidation(site, target, offset, word=word)

    if row["kind"] == "mips_hi16_lo16":
        if (
            row["target_region"] != "load"
            and row["status"] != "reviewed"
            and policy != "all"
        ):
            raise ValueError("target-outside-load-image")
        if not row["paired_site_va"]:
            raise ValueError("missing-paired-site")
        paired = parse_int(row["paired_site_va"])
        if not function.contains(paired, 4):
            raise ValueError("pair-crosses-function-extent")
        paired_offset = paired - function.va
        paired_word = _word(blob, paired_offset)
        try:
            decoded = decode_hi_lo_target(word, paired_word)
        except ValueError as error:
            raise ValueError(f"instruction-pair-mismatch:{error}") from error
        if decoded != target:
            raise ValueError("decoded-target-mismatch")
        try:
            encode_hi_lo_addend(word, paired_word, 0)
        except ValueError as error:
            raise ValueError("unsupported-unsigned-low-pair") from error
        return RelocationValidation(
            site,
            target,
            offset,
            paired,
            paired_offset,
            word,
            paired_word,
        )

    if row["kind"] == "mips32_candidate" and (
        policy == "all" or row["status"] == "reviewed"
    ):
        if word != target:
            raise ValueError("decoded-target-mismatch")
        return RelocationValidation(site, target, offset, word=word)

    raise ValueError("unsupported-relocation-kind")
