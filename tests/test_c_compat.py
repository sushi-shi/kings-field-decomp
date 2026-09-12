from __future__ import annotations

import importlib.util
import shutil
import subprocess
import tempfile
from pathlib import Path
import unittest

from scripts.kf.c_compat import VoidConversion, converted_source, void_conversions, write_overlay
from scripts.kf.clangd import unit_arguments
from scripts.kf.manifest import Unit


class ConversionSpanTests(unittest.TestCase):
    def test_nested_expressions_preserve_each_conversion(self) -> None:
        path = Path("source.c")
        self.assertEqual(converted_source(b"f(p)", (
            VoidConversion(path, 0, 4, "Thing *"),
            VoidConversion(path, 2, 3, "char *"),
        )), b"(Thing *)(f((char *)(p)))")

    def test_ambiguous_macro_and_crossing_spans_fail_closed(self) -> None:
        path = Path("source.c")
        for spans in (
            (VoidConversion(path, 0, 3, "A *"), VoidConversion(path, 0, 3, "B *")),
            (VoidConversion(path, 0, 3, "A *"), VoidConversion(path, 2, 4, "B *")),
        ):
            with self.subTest(spans=spans), self.assertRaises(ValueError):
                converted_source(b"text", spans)


@unittest.skipUnless(shutil.which("clang") and importlib.util.find_spec("clang"),
                     "the pinned Clang and Python binding are required")
class TargetCCompatibilityTests(unittest.TestCase):
    def check_source(self, source: str) -> tuple[tuple[VoidConversion, ...], str, int]:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory).resolve()
            path = repo / "source.c"
            path.write_text(source)
            unit = Unit("game.control", "GAME.EXE", "source.c", "c", ())
            sdk = repo / "sdk"
            sdk.mkdir()
            conversions = void_conversions(unit, repo, sdk)
            overlay = write_overlay(conversions, repo, repo / "generated")
            args = unit_arguments(unit, repo, shutil.which("clang"), sdk)
            args[args.index("-c")] = "-fsyntax-only"
            result = subprocess.run([*args, "-ivfsoverlay", str(overlay)], text=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            self.assertEqual(path.read_text(), source)
            return conversions, result.stdout, result.returncode

    def test_assignment_argument_return_and_array_pointer(self) -> None:
        conversions, diagnostics, status = self.check_source("""\
struct Thing { int value; };
typedef int (*Row)[3];
extern void *allocate(void);
extern void consume(struct Thing *);
struct Thing *make(void) {
    struct Thing *item = allocate();
    Row row = allocate();
    consume(allocate());
    item = allocate();
    return allocate();
}
""")
        self.assertEqual(len(conversions), 5)
        self.assertEqual(status, 0, diagnostics)

    def test_other_argument_enum_error_remains_fatal(self) -> None:
        conversions, diagnostics, status = self.check_source("""\
#ifdef __cplusplus
enum class Floor : unsigned char { one };
enum class Item : unsigned char { one };
#else
typedef unsigned char Floor;
typedef unsigned char Item;
#endif
struct Thing { int value; };
extern void *allocate(void);
extern void consume(struct Thing *, Floor);
void bad(Item item) { consume(allocate(), item); }
""")
        self.assertEqual(len(conversions), 1)
        self.assertNotEqual(status, 0)
        self.assertIn("no matching function", diagnostics)

    def test_const_discard_and_function_pointer_are_not_adapted(self) -> None:
        for source in (
            "void bad(const void *p) { int *value = p; }",
            "void bad(void *p) { void (*callback)(void) = p; }",
        ):
            with self.subTest(source=source):
                conversions, _, status = self.check_source(source)
                self.assertEqual(conversions, ())
                self.assertNotEqual(status, 0)

    def test_macro_returning_void_pointer_is_checked(self) -> None:
        conversions, diagnostics, status = self.check_source("""\
#define ALLOC() allocate()
extern void *allocate(void);
void good(void) { int *value = ALLOC(); }
""")
        self.assertEqual(len(conversions), 1)
        self.assertEqual(status, 0, diagnostics)


if __name__ == "__main__":
    unittest.main()
