"""Explicit void boundaries are checked without rewriting the source."""

from contextlib import redirect_stdout
import importlib.util
from io import StringIO
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest
from unittest import mock

from scripts.kf.check_types import check
from scripts.kf.clangd import unit_arguments
from scripts.kf.manifest import Manifest, Profile, Unit
from scripts.kf.pointer_policy import implicit_void_erasures


@unittest.skipUnless(shutil.which("clang") and importlib.util.find_spec("clang"),
                     "the pinned Clang and Python binding are required")
class PointerPolicyTests(unittest.TestCase):
    def setup_source(self, directory, source):
        repo = Path(directory).resolve()
        (repo / "source.c").write_text(source)
        sdk = repo / "sdk"
        sdk.mkdir()
        unit = Unit("game.control", "GAME.EXE", "source.c", "c", ())
        return repo, sdk, unit

    def sites(self, source):
        with TemporaryDirectory() as directory:
            repo, sdk, unit = self.setup_source(directory, source)
            sites = implicit_void_erasures(unit, repo, sdk)
            self.assertEqual((repo / "source.c").read_text(), source)
            return sites

    def test_assignment_argument_return_and_conditional_are_reported(self):
        sites = self.sites("""
            void consume(void *p);
            void *f(int *p, void *q, int flag) {
                void *out = p;
                consume(p);
                out = flag ? p : q;
                return p;
            }
        """)
        self.assertEqual(len(sites), 4)
        self.assertTrue(all(s.source == "int *" and s.target == "void *" for s in sites))

    def test_const_array_decay_pointer_to_pointer_and_macro_are_reported(self):
        sites = self.sites("""
            #define CONSUME(p) consume(p)
            void consume(const void *p);
            void f(const int *p, int **pp) {
                int array[3];
                consume(p);
                consume(pp);
                consume(array);
                CONSUME(p);
            }
        """)
        self.assertEqual(len(sites), 4)
        self.assertTrue(all(s.target == "const void *" for s in sites))

    def test_explicit_casts_null_void_qualification_and_va_arg_are_not_erasure(self):
        self.assertEqual(self.sites("""
            void consume(const void *p);
            void f(int *p, void *v, __builtin_va_list args) {
                char *text = __builtin_va_arg(args, char *);
                void *raw = __builtin_va_arg(args, void *);
                consume((const void *)p);
                consume(v);
                consume(0);
            }
        """), ())

    def test_modern_only_libc_prototypes_do_not_hide_erasures(self):
        sites = self.sites("""
            #ifdef __cplusplus
            void *copy(void *out, const void *in, unsigned long size);
            void release(void *allocation);
            #else
            void *copy();
            void release();
            #endif
            void f(int *out, const int *in) {
                copy(out, in, sizeof(*out));
                release(out);
            }
        """)
        self.assertEqual(len(sites), 3)
        self.assertEqual([site.target for site in sites],
                         ["void *", "const void *", "void *"])

    def test_retail_only_conversion_is_not_lost_or_double_counted(self):
        sites = self.sites("""
            #ifdef __cplusplus
            void consume(int *p);
            #else
            void consume(void *p);
            #endif
            void shared(void *p);
            void f(int *p) { consume(p); shared(p); }
        """)
        self.assertEqual(len(sites), 2)

    def test_explicit_modern_only_libc_boundaries_pass(self):
        self.assertEqual(self.sites("""
            #ifdef __cplusplus
            void *copy(void *out, const void *in, unsigned long size);
            #else
            void *copy();
            #endif
            void f(int *out, const int *in) {
                copy((void *)out, (const void *)in, sizeof(*out));
            }
        """), ())

    def test_outer_explicit_cast_does_not_hide_nested_call_argument(self):
        sites = self.sites("""
            void *identity(void *p);
            void *f(int *p) { return (void *)identity(p); }
        """)
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0].source, "int *")

    def test_reverse_conversion_is_a_native_clang_error_in_both_modes(self):
        with TemporaryDirectory() as directory:
            repo, sdk, unit = self.setup_source(directory, "int *f(void *p) { return p; }")
            for mode in ("retail", "modern"):
                with self.subTest(mode=mode):
                    args = unit_arguments(unit, repo, "clang", sdk, mode=mode)
                    args[args.index("-c")] = "-fsyntax-only"
                    result = subprocess.run(args, text=True, capture_output=True)
                    self.assertNotEqual(result.returncode, 0, result.stderr)

    def test_cli_rejects_erasure_and_enum_errors_and_accepts_explicit_boundaries(self):
        sources = (
            ("void *f(int *p) { return p; }", 1, "kf-implicit-void-erasure"),
            ("void *f(int *p) { return (void *)p; }", 0, ""),
            ("int *f(void *p) { return (int *)p; }", 0, ""),
            ("""
                #ifdef __cplusplus
                enum class Floor : unsigned char { one };
                enum class Item : unsigned char { one };
                #else
                typedef unsigned char Floor;
                typedef unsigned char Item;
                #endif
                void consume(void *, Floor);
                void bad(int *p, Item item) { consume((void *)p, item); }
            """, 1, "no matching function"),
        )
        for source, status, diagnostic in sources:
            with self.subTest(source=source), TemporaryDirectory() as directory:
                repo, sdk, unit = self.setup_source(directory, source)
                manifest = Manifest({"c": Profile("c", "c", "gcc257-native", "O2", 0,
                                                   "1.07", ())}, (unit,))
                with (mock.patch("scripts.kf.check_types.environment", return_value=("clang", sdk)),
                      redirect_stdout(StringIO()) as output):
                    self.assertEqual(check(repo=repo, manifest=manifest, jobs=1), status)
                self.assertIn(diagnostic, output.getvalue())
                self.assertEqual((repo / "source.c").read_text(), source)
                self.assertEqual(list((repo / "build").rglob("overlay.json")), [])
