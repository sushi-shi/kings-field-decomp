"""Boolean storage must retain target layout and reject non-Boolean writes."""

from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.paths import REPO


class BooleanTypeTests(unittest.TestCase):
    def compile(self, source: str, mode: str = 'modern') -> subprocess.CompletedProcess:
        compiler = shutil.which('clang')
        if not compiler:
            self.skipTest('target Clang required')
        with TemporaryDirectory(prefix='kf-bool-types-') as directory:
            path = Path(directory) / 'control.c'
            path.write_text('#include <kf/bool.h>\n' + source)
            return subprocess.run(
                [compiler, *FLAGS, *MODES[mode], '-fsyntax-only',
                 '-I', str(REPO / 'include'), '-I', str(REPO / 'vendor/include'), str(path)], capture_output=True, text=True)

    def test_target_width_alignment_and_retail_canonical_types(self) -> None:
        aliases = {'KfBool': 'int', 'KfBool32': 's32', 'KfBoolU32': 'u32',
                   'KfBool8': 'u8', 'KfBool16': 'u16'}
        for mode in ('retail', 'modern'):
            checks = []
            for alias, storage in aliases.items():
                checks.extend([
                    f'typedef char size_{alias}[sizeof({alias}) == sizeof({storage}) ? 1 : -1];',
                    f'typedef char align_{alias}[__alignof__({alias}) == '
                    f'__alignof__({storage}) ? 1 : -1];',
                ])
                if mode == 'modern':
                    checks.extend([
                        f'static_assert(__is_trivially_copyable({alias}));',
                        f'static_assert(__is_standard_layout({alias}));',
                    ])
                else:
                    checks.append(f'typedef char type_{alias}['
                                  f'__builtin_types_compatible_p({alias}, {storage}) ? 1 : -1];')
            checks.append('''
                struct State { KfBool8 ready; KfBool16 ended; KfBool32 active; };
                typedef char size_State[sizeof(struct State) == 8 ? 1 : -1];
                typedef char offset_active[__builtin_offsetof(struct State, active) == 4 ? 1 : -1];
            ''')
            with self.subTest(mode=mode):
                result = self.compile('\n'.join(checks), mode)
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_boolean_fields_arguments_returns_arrays_and_outputs(self) -> None:
        result = self.compile('''
            struct State { KfBool8 ready; KfBool16 ended; };
            KfBool32 predicate(int input) { return input != 0; }
            void set(KfBool8 *out, KfBool value) { *out = value; }
            void run(int input) {
                State state = {KF_FALSE, KF_TRUE};
                KfBool8 flags[2] = {KF_FALSE, predicate(input)};
                KfBoolU32 result = predicate(input);
                set(&state.ready, result);
                state.ended = state.ready;
                if (state.ready && !flags[0]) flags[1] = KF_TRUE;
            }
        ''')
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_integer_and_pointer_writes_fail_modern_checking(self) -> None:
        cases = [
            'KfBool32 value = 1;',
            'KfBool8 value; value = 2;',
            'KfBool16 values[2] = {0, 1};',
            'KfBool value = input;',
            'KfBool value = &input;',
            'struct State { KfBool8 ready; } state; state.ready = input;',
            'KfBool32 value; ++value;',
        ]
        for body in cases:
            with self.subTest(body=body):
                result = self.compile('void run(int input) { ' + body + ' }')
                self.assertNotEqual(result.returncode, 0, body)
        result = self.compile('KfBool32 predicate(void) { return 1; }')
        self.assertNotEqual(result.returncode, 0)
