"""kf verify - cleanliness ratchet board (ported from gruntz).

    kf verify board [--gate|--update|--externs]   source cleanliness ratchet

``board`` counts the address-derived spellings and ``extern``/cast/view
crutches in ``src/`` + ``include/`` and the curated identity TSVs against
committed floors (``down = good``). See ``scripts/kf/cleanliness.py`` for the
full contract.
"""

from __future__ import annotations

import sys


def main(argv: list[str] | None = None) -> int:
    args = list(sys.argv[1:] if argv is None else argv)
    if not args or args[0] in ("-h", "--help"):
        print(__doc__)
        return 0
    sub, rest = args[0], args[1:]
    if sub == "board":
        from scripts.kf.cleanliness import main as board_main

        return board_main(rest)
    print(f"kf verify: unknown subcommand {sub!r} (choose board)",
          file=sys.stderr)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
