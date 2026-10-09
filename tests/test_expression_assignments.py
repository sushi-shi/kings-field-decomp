"""Exercise assignment context and macro provenance with real libclang ASTs."""
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.expression_assignments import scan_file


class ExpressionAssignmentsTest(unittest.TestCase):
    def scan(self, text, *, vendor='', mode='c'):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'src').mkdir()
            (root / 'vendor').mkdir()
            (root / 'vendor/sdk.h').write_text(vendor)
            source = root / 'src/control.c'
            source.write_text(text)
            args = ['-x', mode, '-std=c++20' if mode == 'c++' else '-std=c89']
            return scan_file(source, args, root=root)

    def test_conditions_arguments_returns_initializers_and_chains(self):
        sites = self.scan('''int call(int);
int f(int a, int b) {
 int c = (a = 1);
 call(a += 2);
 if ((a = call(b))) b = 1;
 while ((a = call(b))) b = 2;
 do b = 3; while ((a = call(b)));
 for (a = 0; (b = call(a)); a += 1) b = 4;
 a = b = 5;
 return a = c;
}''')
        self.assertEqual(len(sites), 8)

    def test_standalone_assignments_and_loop_clauses_are_excluded(self):
        self.assertFalse(self.scan('''void f(int a, int b) {
 a = 1; (a += 2);
 if (a == b) a = b; else b = a;
 while (a < b) a += 1;
 do a = b; while (a != b);
 for (a = 0; a < b; a += 1) b = a;
 for (; a < b; ) a = b;
}'''))

    def test_project_macros_count_and_sdk_macros_do_not_hide_arguments(self):
        sites = self.scan('''#include "../vendor/sdk.h"
#define NEXT(x) ((x) += 1)
void f(int a, int b) {
 SDK(a, b);
 a = NEXT(b);
 SDK(a, (b = 2));
}''', vendor='#define SDK(x,y) ((x) = (y), (x) += 1)\n')
        self.assertEqual(len(sites), 2)
        self.assertTrue(all(site.origin_file == 'src/control.c' for site in sites))

    def test_parse_errors_fail_closed(self):
        with self.assertRaises(ValueError):
            self.scan('void f(void) { unknown = 1; }')

    def test_cpp_overloaded_assignment_is_counted_when_nested(self):
        sites = self.scan('''struct S { S& operator=(int); };
S& f(S& a) { return a = 2; }
void g(S& a) { a = 3; }
''', mode='c++')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0].operator, 'Assign')


if __name__ == '__main__':
    unittest.main()
