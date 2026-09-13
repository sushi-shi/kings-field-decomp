"""Compare real target-C enums without mistaking equal values for identities."""

from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.enums import collect, main, scan_file


class EnumsTest(unittest.TestCase):
    def test_evaluation_scope_and_inactive_branches(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "probe.c"
            source.write_text("""
#define TEN (5 * 2)
enum { DECIMAL = 10, HEX = 0xa, ALIAS = DECIMAL, EXPRESSION = TEN,
       IMPLICIT, NEGATIVE = -10, HIGH = 0xffffffffU };
enum Tagged { TAGGED = 10 };
int probe(void) { enum { LOCAL = 10 }; return LOCAL; }
#if 0
enum { INACTIVE = 10 };
#endif
""")
            rows = scan_file(source, [*MODES["retail"], *FLAGS], root=root)
            values = {row.name: row.value for row in rows}
            self.assertEqual(values, {
                "DECIMAL": 10, "HEX": 10, "ALIAS": 10, "EXPRESSION": 10,
                "IMPLICIT": 11, "NEGATIVE": -10, "HIGH": 0xffffffff,
                "TAGGED": 10, "LOCAL": 10,
            })
            self.assertEqual(next(row.enum for row in rows if row.name == "TAGGED"), "Tagged")
            for row in rows:
                self.assertEqual(source.read_bytes()[row.offset:row.offset + len(row.name)],
                                 row.name.encode())

    def test_headers_and_variant_values_preserve_context(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "include").mkdir()
            (root / "include/shared.h").write_text(
                "enum { SHARED = 10, VARIANT = CHOICE };\n")
            (root / "a.c").write_text('#include <shared.h>\nenum { A = 10 };\n')
            (root / "b.c").write_text('#include <shared.h>\nenum { B = 10 };\n')
            units = [SimpleNamespace(source=source, unit=name, image=image, defines=defines)
                     for source, name, image, defines in (
                         ("a.c", "a-game", "GAME.EXE", ("CHOICE=1",)),
                         ("a.c", "a-open", "OPEN.EXE", ("CHOICE=2",)),
                         ("b.c", "b-game", "GAME.EXE", ("CHOICE=1",)),
                     )]
            rows = collect(units, "clang", root, root=root)
            shared = [row for row in rows if row["name"] == "SHARED"]
            self.assertEqual(len(shared), 1)
            self.assertEqual(len(shared[0]["contexts"]), 3)
            variants = [row for row in rows if row["name"] == "VARIANT"]
            self.assertEqual([row["value"] for row in variants], [1, 2])
            self.assertEqual([len(row["contexts"]) for row in variants], [2, 1])
            self.assertEqual(len(rows), 5)

    def test_parse_errors_abort(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "bad.c"
            source.write_text("enum { BAD = missing };\n")
            with self.assertRaisesRegex(ValueError, "parsing failed"):
                scan_file(source, [*MODES["retail"], *FLAGS], root=root)

    def test_filters_and_failures(self):
        rows = [{"name": name, "value": value} for name, value in
                (("A", 10), ("B", 10), ("ONLY", 11), ("ONLY", 11))]
        with patch("scripts.kf.enums.load_manifest"), \
                patch("scripts.kf.enums.select_units", return_value=[None]), \
                patch("scripts.kf.enums.environment", return_value=("clang", Path("/tmp"))), \
                patch("scripts.kf.enums.collect", return_value=rows) as scan, \
                redirect_stdout(StringIO()) as output, redirect_stderr(StringIO()):
            self.assertEqual(main(["--duplicates", "--value", "0xa", "--json"]), 0)
            self.assertEqual([row["name"] for row in json.loads(output.getvalue())["constants"]],
                             ["A", "B"])
            output.truncate(0)
            output.seek(0)
            scan.side_effect = ValueError("bad parse")
            self.assertEqual(main(["--json"]), 2)
            self.assertEqual(output.getvalue(), "")


if __name__ == "__main__":
    unittest.main()
