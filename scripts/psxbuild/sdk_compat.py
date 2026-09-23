"""Compatibility corrections to pinned SDK linker inputs."""

import hashlib


LIBETC_SHA256 = '571cfbbc00c34e3f1f0fd19eb54ba88ae1a15d1ba3bcb0d30e9d0fcfe42bc7be'
IRQ_REPEAT_OFFSET = 0x8b5
IRQ_REPEAT_BRANCH = bytes.fromhex('ccff4014')


def library_input(name: str, data: bytes) -> tuple[bytes, list[dict]]:
    """Let the BIOS service new interrupts after each SDK dispatch pass.

    Release 2.5 INTR.OBJ can consume every VBlank during repeated GPU DMA,
    starving BIOS controller, sound and event handlers. Return after one
    pass, as the game's original interrupt dispatcher does. A pending IRQ
    then enters the BIOS normally. The original SDK archive stays intact.
    """
    if name != 'LIBETC.LIB':
        return data, []
    if hashlib.sha256(data).hexdigest() != LIBETC_SHA256:
        raise ValueError('LIBETC interrupt correction requires the pinned Release 2.5 archive')
    if data[IRQ_REPEAT_OFFSET:IRQ_REPEAT_OFFSET + 4] != IRQ_REPEAT_BRANCH:
        raise ValueError('LIBETC interrupt repeat branch differs from the verified input')
    patched = data[:IRQ_REPEAT_OFFSET] + b'\0' * 4 + data[IRQ_REPEAT_OFFSET + 4:]
    return patched, [{
        'name': 'bios-interrupt-return',
        'member': 'INTR.OBJ',
        'archive_offset': IRQ_REPEAT_OFFSET,
        'before': IRQ_REPEAT_BRANCH.hex(),
        'after': '00000000',
        'reason': 'Return to BIOS after one dispatch pass so new VBlank events reach input and audio',
    }]
