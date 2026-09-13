# Native COMMON allocation depends on symbol names

The semantic naming campaign changed `game_exit_code` to
`game_next_overlay_mode`, reflecting the GAME result consumed by the overlay
loader. The pinned native linker allocated that COMMON symbol at a different
address. This changed the linked GAME executable despite identical compiled
instructions and ordered relocation referents before linking.

This is an observed name dependence in the current native build. No hash-table,
sorting, or allocation algorithm is attributed to PSYLINK from this control.
These are reconstruction link addresses, not the curated retail identities.

| Measurement | Before | After |
| --- | --- | --- |
| Global name | `game_exit_code` | `game_next_overlay_mode` |
| Native address in GAME.SYM | `0x800a6480` | `0x8009e3a8` |
| Curated retail address | `0x800958f8` | `0x800958f8` |
| Native assembly reservation | `.comm game_exit_code,8` | `.comm game_next_overlay_mode,8` |
| GAME.EXE bytes | 286,720 | 286,720 |
| GAME.EXE SHA-256 | `3a7ec1d610059104c19a18447be1ffeb38f4c11f01fe0f4b2a2c59ae6be3c094` | `bc4df30d474ee6222372c39a794b398e3c97116d58447fefe6ed0544f0694093` |

GAME.EXE differs at 221 bytes. The first differing instruction is at file offset
`0x2218`, native address `0x80013a18`: `0x8c426480` becomes `0x8c42e3a8`, the
load of the renamed datum through its relocated address. GAME.MAP section
boundaries and lengths are unchanged. The compiler's eight-byte COMMON
reservation is also unchanged; the C datum and curated claim remain four bytes.

All 61 native GAME unit objects were decoded with `scripts.kf.lnk.read`.
Section bytes and alignment, ordered relocation expressions resolved through
symbol names, and unordered symbol/local metadata agree after the three GAME
identity renames: this global, `display_show_error_screen` to
`display_show_system_screen`, and `error_screen_path` to `system_screen_path`.
Only symbol identity spelling is normalized; addresses, addends, reservation
sizes, relocation order, and instruction bytes are compared literally.

A scratch control then restored only `game_next_overlay_mode` to
`game_exit_code` in the current generated assembly of `game.main`, `game.game`,
and `game.player_update`. Reassembling those three copied files with their
recorded flags and relinking all current inputs reproduced the entire baseline
GAME.EXE byte for byte, with the baseline SHA-256 above. The other reviewed
function/data/parameter renames remained present. This isolates every native
GAME byte difference to the single global-name change.

## Reproduce the control

After `nix develop -c kf build`, run the following from the repository root inside
`nix develop`. It copies generated inputs to a fresh temporary directory,
changes only the identifier in copied assembly, and uses each unit's recorded
assembler command. The `-G0` versus `-G8` distinction must be retained.

```python
import hashlib
import json
from pathlib import Path
import re
import shutil
import tempfile
from scripts.kf.sdk import dos_run, tool_succeeded

current = Path('build/link/game')
scratch = Path(tempfile.mkdtemp(prefix='kf-common-name-control-', dir='/tmp'))
for path in current.iterdir():
    if (path.is_file() and path.suffix.upper() in {'.OBJ', '.LIB', '.EXE', '.LNK'}
            and path.name != 'GAME.EXE'):
        shutil.copyfile(path, scratch / path.name)
for unit in json.loads((current / 'build.json').read_text())['units']:
    source = (current / unit['assembly']).read_bytes()
    restored, count = re.subn(rb'\bgame_next_overlay_mode\b', b'game_exit_code', source)
    if count:
        (scratch / unit['assembly']).write_bytes(restored)
        dos_run(scratch, [unit['assembler_command']], 'asm' + Path(unit['assembly']).stem)
        tool_succeeded(scratch, unit['log'], unit['object'], b'LNK')
dos_run(scratch, ['psylink /c @LINK.LNK,GAME.CPE,GAME.SYM,GAME.MAP > LINK.TXT'], 'link')
tool_succeeded(scratch, 'LINK.TXT', 'GAME.CPE', b'CPE')
dos_run(scratch, ['cpe2x GAME.CPE > CONVERT.TXT'], 'convert')
print(scratch)
print(hashlib.sha256((scratch / 'GAME.EXE').read_bytes()).hexdigest())
```

The recorded native commands are:

```text
aspsx -g -G0 -o U0000.OBJ U0000.S > U0000.TXT
aspsx -g -G8 -o U0004.OBJ U0004.S > U0004.TXT
aspsx -g -G8 -o U0013.OBJ U0013.S > U0013.TXT
psylink /c @LINK.LNK,GAME.CPE,GAME.SYM,GAME.MAP > LINK.TXT
cpe2x GAME.CPE > CONVERT.TXT
```

The campaign's ignored `build/semantic-audit/native-common-name-control.json`
records input hashes, commands, all differing words, and the successful exact
comparison. The compiled-object comparison and strict objdiff results remain
unchanged. The native executable is explicitly not byte-identical. This leaves
native COMMON/BSS placement as an existing ownership/link reconstruction limit;
it does not justify individually placing globals, padding source, or preserving
a less precise name to protect a checksum. The control establishes the cause
of the byte difference, not complete native executable runtime correctness.
