"""Controls for deterministic source export and publication safety."""

import json
from pathlib import Path
import tempfile
import unittest

from scripts.kf.clean import (
    MARKER, PROVENANCE, clean_c, compare_program, generate, git, publish, snapshot,
    validate_output, write_output,
)
from scripts.kf.clean_lexer import resolve_conditionals, rewrite_calls, strip_comments, tidy


class LexicalControls(unittest.TestCase):
    def test_nested_calls_literals_and_token_boundaries(self):
        source = 'DATA(1, 4)\nconst char *x = "DATA(1, 2) /*literal*/";\nint/**/x;\n'
        result = clean_c(source, {})
        self.assertIn('"DATA(1, 2) /*literal*/"', result)
        self.assertIn('int x;', result)
        self.assertNotIn('DATA(1, 4)', result)
        self.assertEqual(clean_c('KF_ENUM_DECODE(KF_ENUM_PROMOTED(Id), call(1, 2))', {'Id': 'int'}),
                         '((int)(call(1, 2)))\n')

    def test_c_type_representation(self):
        source = '''#include <kf/lib/enum.h>
KF_ENUM_BEGIN(Id, u8)
    A = 1, B = 2
KF_ENUM_END(Id)
KF_ENUM_FLAGS(Id, u8)
KF_ENUM_STORAGE(Id, u16) x;
KF_ENUM_PARAM(Id, s32) f(KF_ENUM_PROMOTED(Id) y);
'''
        output = clean_c(source, {'Id': 'int'})
        self.assertIn('typedef u8 Id; enum {', output)
        self.assertIn('u16 x;', output)
        self.assertIn('s32 f(int y);', output)
        self.assertNotIn('KF_ENUM', output)

    def test_effect_argument_wrappers_become_original_argument_lists(self):
        source = '#define KF_EFFECT_ARGS_ROTATION_SOUND(r, s) (r), (s)\n' \
                 'spawn(kind, KF_EFFECT_ARGS_ROTATION_SOUND(rotation(1, 2), sound));\n'
        self.assertEqual(clean_c(source, {}), 'spawn(kind, (rotation(1, 2)), (sound));\n')

    def test_nested_conditionals_preserve_image_selection(self):
        source = '''#if KF_MODERN_TYPES
wrong
#ifdef KF_OPEN
also_wrong
#endif
#else
#ifdef KF_OPEN
open
#else
game
#endif
#endif
#if KF_MODERN_TYPES && !defined(KF_EFFECT_POOL_IMPLEMENTATION)
wrong
#endif
'''
        self.assertEqual(resolve_conditionals(source), '#ifdef KF_OPEN\nopen\n#else\ngame\n#endif\n')
        with self.assertRaisesRegex(ValueError, 'unsupported type conditional'):
            resolve_conditionals('#if KF_MODERN_TYPES || SOMETHING\ntext\n#endif\n')

    def test_rust_literals_lifetimes_and_nested_comments(self):
        source = '''fn f<'a>(x: &'a str) { let s = r##"/* untouched */"##;
let c = '/'; /* outer /* inner */ end */ let b = b'\\n'; } // removed
'''
        result = strip_comments(source, rust=True)
        self.assertIn("fn f<'a>(x: &'a str)", result)
        self.assertIn('r##"/* untouched */"##', result)
        self.assertIn("b'\\n'", result)
        self.assertNotIn('outer', result)
        self.assertNotIn('removed', result)

    def test_whitespace_cleanup_preserves_multiline_literals(self):
        literal = 'r#"first  \n\n\nlast  "#'
        result = tidy(f'\n\nconst TEXT: &str = {literal};  \n\n\n', rust=True)
        self.assertEqual(result, f'const TEXT: &str = {literal};\n')

    def test_bad_macro_and_unterminated_inputs_fail(self):
        rules = {'DROP': (2, lambda args: '')}
        for text in ('DROP(1)', 'DROP(1, (2)', 'DROP(1, [2))', 'DROP'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                rewrite_calls(text, rules)
        with self.assertRaises(ValueError):
            strip_comments('/* unterminated')
        with self.assertRaises(ValueError):
            clean_c('KF_ENUM_NEW_KIND(x)', {})

    def test_line_comment_splicing_and_license(self):
        result = strip_comments('// hidden \\\nalso hidden\nint x; /* Copyright Example */')
        self.assertNotIn('also hidden', result)
        self.assertIn('Copyright Example', result)


class ExportControls(unittest.TestCase):
    def test_verification_separates_reserved_header_bytes_from_link_identity(self):
        original = b'PS-X EXE' + bytes(2048)
        link = b'CPE\x01native link'
        self.assertEqual(compare_program(original, original, link, link), [])
        changed = bytearray(original)
        changed[8] = 17
        self.assertEqual(compare_program(original, changed, link, link), [8])
        with self.assertRaisesRegex(ValueError, 'native linker'):
            compare_program(original, changed, link, link + b'changed')
        for offset in (0, 16, 20, 48, 128, 2048):
            changed = bytearray(original)
            changed[offset] ^= 1
            with self.subTest(offset=offset), self.assertRaisesRegex(ValueError, 'executable differs'):
                compare_program(original, changed, link, link)

    def test_output_replacement_guards(self):
        with tempfile.TemporaryDirectory() as temporary:
            repo = Path(temporary) / 'repo'
            repo.mkdir()
            for output in (repo, repo.parent, repo / 'build', repo / 'src'):
                with self.subTest(output=output), self.assertRaises(ValueError):
                    validate_output(repo, output)
            output = repo / 'build/clean'
            write_output(repo, output, {'hello': b'first'}, 'a' * 40, False)
            write_output(repo, output, {'world': b'second'}, 'a' * 40, False)
            self.assertFalse((output / 'hello').exists())
            self.assertEqual((output / 'world').read_bytes(), b'second')
            (output / '.git').write_text('gitdir: somewhere')
            with self.assertRaises(ValueError):
                validate_output(repo, output)
            (output / '.git').unlink()
            (output / 'nested').mkdir()
            (output / 'nested/.git').write_text('gitdir: somewhere')
            with self.assertRaises(ValueError):
                validate_output(repo, output)
            link = repo / 'build/link'
            link.symlink_to(output, target_is_directory=True)
            with self.assertRaises(ValueError):
                validate_output(repo, link)

    def test_real_tree_generation_is_deterministic_and_has_no_tests(self):
        repo = Path(__file__).resolve().parents[1]
        # Nix's source-only test fixture has no Git metadata.
        if not (repo / '.git').exists():
            self.skipTest('requires a Git source snapshot')
        _, files = snapshot(repo, 'HEAD', working=True)
        if 'scripts/kf/clean_project/flake.nix' not in files:
            self.skipTest('new generator files are not staged yet')
        first, second = generate(files), generate(files)
        self.assertEqual(first, second)
        self.assertNotIn('checks', first['flake.nix'].decode())
        self.assertNotIn('include/kf/lib/address.h', first)
        self.assertFalse(any('/tests/' in path or '/bin/' in path or path.startswith('tests/')
                             for path in first))
        self.assertNotIn('codecs/src/lib.rs', first)
        self.assertNotIn('vendor/include/stdarg.h', first)
        for declaration in (b'extern void AddPrim(', b'extern void DrawOTag(',
                            b'extern void SetSemiTrans('):
            self.assertNotIn(declaration, first['vendor/include/psyq/sdk.h'])
        for declaration in (b'extern void *memcpy(', b'extern void *memset(',
                            b'extern void *malloc(', b'extern void free('):
            self.assertNotIn(declaration, first['vendor/include/psyq/libc.h'])
        modern = generate(files, modern=True)
        self.assertIn('codecs/src/lib.rs', modern)
        self.assertFalse(any(
            b'#include <kf/lib/address.h>' in data for data in modern.values()
        ))
        self.assertIn(b'enum class', modern['include/kf/game/menu.h'])
        self.assertIn('src/game/main.cpp', modern)
        self.assertNotIn('scripts/kf/cli.py', first)
        self.assertEqual(len(json.loads(first['build.json'])['images']), 3)
        fragments = {name for name in files if name.startswith('src/lib/')
                     and name.endswith('.inc')}
        self.assertTrue(fragments)
        for output in (first, modern):
            for name in fragments:
                self.assertIn(name, output)
                self.assertNotIn(b'ADDRESS', output[name])
                self.assertNotIn(b'KF_ENUM_', output[name])

    def test_publish_keeps_one_snapshot_and_refuses_local_changes(self):
        with tempfile.TemporaryDirectory() as temporary:
            repo = Path(temporary) / 'repo'
            repo.mkdir()
            git(repo, 'init', '-b', 'master')
            git(repo, 'config', 'user.name', 'Export Control')
            git(repo, 'config', 'user.email', 'export@example.invalid')
            (repo / 'tracked').write_text('input')
            git(repo, 'add', 'tracked')
            git(repo, 'commit', '-m', 'input')
            commit = git(repo, 'rev-parse', 'HEAD')
            worktree = Path(temporary) / 'source'
            files = {'README.md': b'generated\n', '.gitignore': b'/build/\n'}
            publish(repo, files, commit, 'source', worktree)
            tip = git(repo, 'rev-parse', 'source')
            self.assertEqual(git(repo, 'rev-list', '--count', 'source'), '1')
            self.assertIn(PROVENANCE, git(worktree, 'log', '-1', '--format=%B'))
            self.assertEqual(git(worktree, 'status', '--porcelain'), '')
            publish(repo, files, commit, 'source', worktree)
            self.assertEqual(git(repo, 'rev-parse', 'source'), tip)
            (worktree / 'README.md').write_text('local edit')
            with self.assertRaisesRegex(ValueError, 'local changes'):
                publish(repo, files, commit, 'source', worktree)
            (worktree / 'README.md').write_bytes(files['README.md'])
            (repo / 'tracked').write_text('new input')
            git(repo, 'commit', '-am', 'second input')
            second = git(repo, 'rev-parse', 'HEAD')
            publish(repo, {'README.md': b'new generation\n'}, second, 'source', worktree)
            parents = git(repo, 'rev-list', '--parents', '-1', 'source').split()[1:]
            self.assertEqual(parents, [])
            self.assertEqual(git(repo, 'rev-list', '--count', 'source'), '1')
            tree = git(repo, 'rev-parse', 'source^{tree}')
            # Even identical contents must repair an older export with ancestry.
            message = git(worktree, 'log', '-1', '--format=%B')
            legacy = git(repo, 'commit-tree', tree, '-p', second, '-m', message)
            git(repo, 'update-ref', 'refs/heads/source', legacy)
            publish(repo, {'README.md': b'new generation\n'}, second, 'source', worktree)
            self.assertEqual(git(repo, 'rev-list', '--count', 'source'), '1')
            self.assertEqual(git(repo, 'rev-parse', 'source^{tree}'), tree)
            self.assertEqual(git(worktree, 'status', '--porcelain'), '')
            git(repo, 'branch', 'unrelated')
            with self.assertRaisesRegex(ValueError, 'not generated'):
                publish(repo, files, second, 'unrelated', Path(temporary) / 'unrelated')
            self.assertFalse((worktree / MARKER).exists())


if __name__ == '__main__':
    unittest.main()
