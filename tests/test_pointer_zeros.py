"""Exercise pointer-zero searches against real pylibclang translation units."""

from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch
from contextlib import redirect_stdout, redirect_stderr
from io import StringIO
import json

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.manifest import Manifest, Profile, Unit
from scripts.kf.pointer_zeros import main, scan_file


class PointerZerosTest(unittest.TestCase):
    def scan(self, source: str):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "probe.c"
            path.write_text(source, encoding="utf-8")
            return scan_file(path, [*MODES["retail"], *FLAGS], root=root)

    def test_assignments_initializers_and_other_pointer_contexts(self):
        source = """typedef int *Pointer;
struct S { int number; Pointer pointer; void (*callback)(void); };
Pointer global = 0;
Pointer array[] = {0, (int *)0U};
struct S record = {0, 0, 0};
struct S designated = {.pointer = 0};
void take(Pointer pointer);
Pointer probe(Pointer p, Pointer *out, struct S *s, int condition) {
    Pointer local = (0);
    p = 0x0;
    *out = 00;
    s->pointer = 0L;
    array[0] = 0;
    s->callback = (void (*)(void))0;
    p = local = 0;
    p = condition ? 0 : local;
    p = (Pointer)(long)0;
    take(0);
    if (p == 0) return 0;
    return local;
}
"""
        sites = self.scan(source)
        self.assertEqual(len(sites), 18)
        self.assertEqual([site.line for site in sites],
                         [3, 4, 4, 5, 5, 6, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 19])
        for site in sites:
            self.assertEqual(source.encode()[site.offset:site.offset + len(site.spelling)],
                             site.spelling.encode())

    def test_ordinary_zeros_null_and_implicit_zero_fill_are_not_reported(self):
        sites = self.scan("""#define NULL ((void *)0)
#define NIL NULL
struct S { int n; int *p; };
struct S s = {0};
int n = 0;
int *uninitialized;
int *probe(int *p, int index) {
    int value = 0;
    p = NULL;
    p = NIL;
    p = (int *)NULL;
    p += 0;
    p = p + 0;
    p = &p[0];
    p = (int *)1;
    value = p == p;
    return &p[index == 0];
}
""")
        self.assertEqual(sites, ())

    def test_macro_definitions_arguments_headers_and_image_variants(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            header = root / "probe.h"
            header.write_text("#define NULL 0\n#define ZERO 0\n#define CLEAR(p) ((p)=0)\n"
                              "#define ID(p) (p)\nstatic int *header_pointer = 0;\n")
            path = root / "probe.c"
            path.write_text('#include "probe.h"\nvoid probe(int *p) {\n'
                            'p=ZERO; p=ZERO; CLEAR(p); p=ID(0); p=ID(NULL);\n'
                            '#ifdef OPEN\np=0;\n#endif\n}\n')
            profile = Profile("c", "c", "gcc257-native", "O2", 0, "1.07", ())
            manifest = Manifest({"c": profile}, (
                Unit("game.probe", "GAME.EXE", "probe.c", "c", ()),
                Unit("open.probe", "OPEN.EXE", "probe.c", "c", (), defines=("OPEN",)),
            ))
            stdout, stderr = StringIO(), StringIO()
            with (patch("scripts.kf.pointer_zeros.REPO", root),
                  patch("scripts.kf.pointer_zeros.load_manifest", return_value=manifest),
                  patch("scripts.kf.pointer_zeros.environment", return_value=("clang", root)),
                  redirect_stdout(stdout), redirect_stderr(stderr)):
                self.assertEqual(main(["--json", "--check"]), 1)
            report = json.loads(stdout.getvalue())
            self.assertEqual(report["variants"], 2)
            self.assertEqual(report["count"], 5, report)
            sites = report["sites"]
            self.assertEqual([(site["file"], site["line"]) for site in sites],
                             [("probe.c", 3), ("probe.c", 5),
                              ("probe.h", 2), ("probe.h", 3), ("probe.h", 5)])
            self.assertEqual(len(sites[2]["contexts"]), 2)
            self.assertEqual(sites[1]["contexts"][0]["image"], "OPEN.EXE")

    def test_parse_errors_abort_instead_of_reporting_a_clean_scan(self):
        with self.assertRaisesRegex(ValueError, "parsing failed"):
            self.scan("#include <missing.h>\nint *p = 0;\n")

    def test_null_inside_nested_header_macro_is_not_a_written_zero(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "null.h").write_text("#define NULL 0\n")
            source = root / "probe.c"
            source.write_text(
                '#include "null.h"\n'
                'struct S { int n; int p; };\n'
                '#define NIL NULL\n'
                '#define OWNER(p) ((struct S *)((char *)(p) - '
                '(long)&((struct S *)NIL)->p))\n'
                '#define PICK(p) ((p) ? (struct S *)0 : 0)\n'
                'struct S *probe(int *p) { return OWNER(p); }\n'
                'struct S *pick(int *p) { return PICK(p); }\n')
            sites = scan_file(source, [*MODES["retail"], *FLAGS], root=root)
            self.assertEqual([(site.line, site.spelling) for site in sites],
                             [(5, "0"), (5, "0")])


if __name__ == "__main__":
    unittest.main()
