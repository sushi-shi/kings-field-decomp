"""Positive and negative controls for the whole-source Boolean audit."""

from __future__ import annotations

from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.booleans import collect
from scripts.kf.manifest import Manifest, Profile, Unit


def audit_sources(sources: dict[str, str], *, variants: bool = False, jobs: int = 1,
                  names: tuple[str, ...] = ()) -> dict:
    with TemporaryDirectory() as directory:
        repo = Path(directory)
        (repo / 'src').mkdir()
        (repo / 'include').mkdir()
        sdk = repo / 'sdk'
        sdk.mkdir()
        for name, source in sources.items():
            path = repo / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(source)
        profile = Profile('c', 'c', 'gcc257-native', 'O2', 0, '1.07', ())
        units = tuple(Unit(f'game.{Path(name).stem}', 'GAME.EXE', name, 'c', ())
                      for name in sources if name.endswith('.c'))
        if variants:
            units += tuple(Unit(f'open.{Path(name).stem}', 'OPEN.EXE', name, 'c', (),
                                defines=('OPEN_VARIANT',))
                           for name in sources if name.endswith('.c'))
        return collect(repo=repo, sdk=sdk, manifest=Manifest({'c': profile}, units),
                       jobs=jobs, names=names)


def slot(audit: dict, name: str, *, kind: str = 'return', owner: str | None = None,
         image: str = 'GAME.EXE') -> dict:
    rows = [row for row in audit['slots'] if row['name'] == name and row['kind'] == kind
            and row['image'] == image and (owner is None or row['owner'] == owner)]
    if len(rows) != 1:
        raise AssertionError((name, kind, owner, rows))
    return rows[0]


class BooleanAuditTests(unittest.TestCase):
    def test_shared_body_is_audited_and_unincluded_fragment_fails_coverage(self):
        sources = {
            'src/probe.c': '#include "shared.inc"\n',
            'src/shared.inc': 'int predicate(int value) { return value != 0; }\n',
        }
        audit = audit_sources(sources, variants=True)
        for image in ('GAME.EXE', 'OPEN.EXE'):
            self.assertEqual(slot(audit, 'predicate', image=image)['classification'], 'candidate')
        sources['src/unseen.inc'] = 'int other(void) { return 2; }\n'
        with self.assertRaisesRegex(RuntimeError, 'src/unseen.inc'):
            audit_sources(sources)

    def test_value_flow_joins_fields_parameters_locals_and_returns_across_units(self) -> None:
        audit = audit_sources({
            'include/probe.h': '''
                struct State { unsigned char ready; int count; };
                extern struct State state;
                int consume(int declaration_name);
                int predicate(int value);
            ''',
            'src/provider.c': '''
                #include <probe.h>
                struct State state;
                int predicate(int value) { return value != 0; }
                int consume(int flag) { state.ready = flag; return flag; }
            ''',
            'src/caller.c': '''
                #include <probe.h>
                int run(int unknown) {
                    int ready = predicate(unknown);
                    consume(ready);
                    if (state.ready) return 1;
                    return 0;
                }
            ''',
        })
        for name, kind, owner in (('predicate', 'return', None),
                                  ('consume', 'return', None),
                                  ('flag', 'parameter', 'consume'),
                                  ('ready', 'local', 'run'),
                                  ('ready', 'field', 'State')):
            with self.subTest(name=name, kind=kind):
                row = slot(audit, name, kind=kind, owner=owner)
                self.assertEqual(row['values'], ['0', '1'])
                self.assertEqual(row['classification'], 'candidate')
        self.assertEqual(audit['coverage']['variants'], 2)
        self.assertEqual(audit['coverage']['parse_errors'], 0)

    def test_non_boolean_writes_and_numeric_uses_do_not_become_predicates(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            int count;
            int flags[2] = {0, 1};
            int mixed(int value) { if (value) return 1; return -1; }
            int zero(void) { return 0; }
            void run(int value) {
                count = 0; count++;
                flags[0] = value ? 1 : 0;
                if (flags[1]) count = 2;
            }
        '''})
        self.assertIn('other', slot(audit, 'mixed')['values'])
        self.assertNotEqual(slot(audit, 'mixed')['classification'], 'candidate')
        self.assertEqual(slot(audit, 'zero')['classification'], 'review-single-value')
        self.assertEqual(slot(audit, 'count', kind='global')['classification'], 'non-boolean')
        self.assertEqual(slot(audit, 'flags', kind='global')['classification'], 'candidate')

    def test_pointer_alias_and_output_argument_writes_reach_the_original_slot(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            int flag;
            void set(int *out) { *out = 2; }
            void run(void) { int *alias = &flag; flag = 0; set(alias); }
        '''})
        flag = slot(audit, 'flag', kind='global')
        self.assertIn('other', flag['values'])
        self.assertEqual(flag['classification'], 'non-boolean')
        self.assertIn('pointer-write', flag['hazards'])
        self.assertIn('other', slot(audit, 'out', kind='parameter_pointee')['values'])

    def test_external_pointer_writes_and_union_views_remain_unknown(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            void receive(void *destination);
            union State { int flag; unsigned char bytes[4]; } state;
            int escaped;
            void run(void) {
                escaped = 0; escaped = 1; receive(&escaped);
                state.flag = 0; state.flag = 1; state.bytes[0] = 7;
            }
        '''})
        escaped = slot(audit, 'escaped', kind='global')
        self.assertIn('unknown', escaped['values'])
        self.assertIn('external-pointer-write', escaped['hazards'])
        flag = slot(audit, 'flag', kind='field')
        self.assertIn('unknown', flag['values'])
        self.assertIn('overlapping-union-write', flag['hazards'])

    def test_initializers_include_designated_fields_and_implicit_zeroes(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            struct State { int flag; int number; int unset; };
            struct State state = { .number = 7, .flag = 1 };
            void run(void) { state.flag = 0; }
        '''})
        self.assertEqual(slot(audit, 'flag', kind='field')['classification'], 'candidate')
        self.assertEqual(slot(audit, 'number', kind='field')['classification'], 'non-boolean')
        self.assertEqual(slot(audit, 'unset', kind='field')['values'], ['0'])

    def test_images_and_same_named_locals_keep_distinct_declaration_identities(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            int first(void) { int value = 0; value = 1; return value; }
            int second(void) { int value = 5; return value; }
            int overlay(void) {
            #ifdef OPEN_VARIANT
                return 7;
            #else
                return 1;
            #endif
            }
        '''}, variants=True)
        self.assertEqual(slot(audit, 'value', kind='local', owner='first')['values'], ['0', '1'])
        self.assertEqual(slot(audit, 'value', kind='local', owner='second')['values'], ['other'])
        self.assertEqual(slot(audit, 'overlay', image='GAME.EXE')['values'], ['1'])
        self.assertEqual(slot(audit, 'overlay', image='OPEN.EXE')['values'], ['other'])

    def test_unknown_input_does_not_taint_normalized_comparison_result(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            int input(void);
            int copied(void) { return input(); }
            int normalized(void) { return input() != 0; }
        '''})
        self.assertEqual(slot(audit, 'copied')['values'], ['unknown'])
        self.assertEqual(slot(audit, 'normalized')['values'], ['0', '1'])
        self.assertEqual(slot(audit, 'normalized')['classification'], 'candidate')

    def test_incremented_pointer_writes_and_pointer_reads_are_tracked(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            int text[2];
            int flag;
            void write(int *out) { *out++ = 65; *out = 0; }
            int read_flag(const int *value) { return *value; }
            int run(int condition) {
                int *cursor = text;
                flag = condition != 0;
                write(cursor);
                return read_flag(&flag);
            }
        '''})
        self.assertEqual(slot(audit, 'text', kind='global')['classification'], 'non-boolean')
        self.assertEqual(slot(audit, 'run')['values'], ['0', '1'])

    def test_macro_domains_and_macro_negation_keep_ast_meaning(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            #define KF_ENUM_BEGIN(name, storage) typedef storage name; enum {
            #define KF_ENUM_END(name) };
            #define KF_ENUM_PARAM(name, storage) storage
            #define NEGATE(value) (!(value))
            KF_ENUM_BEGIN(Variant, unsigned char) FIRST = 0, SECOND = 1 KF_ENUM_END(Variant)
            int choose(KF_ENUM_PARAM(Variant, unsigned int) variant) { return variant; }
            int run(int input) { choose(FIRST); choose(SECOND); return NEGATE(input); }
        '''})
        self.assertEqual(slot(audit, 'variant', kind='parameter')['classification'], 'already-enum')
        self.assertEqual(slot(audit, 'run')['values'], ['0', '1'])
        self.assertNotIn('other', audit['by_kind'])

    def test_nested_array_indices_and_compound_operands_are_numeric_uses(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            struct Point { int step; int x; } points[2];
            void run(int condition) {
                int index = condition != 0;
                points[0].step = 0; points[1].step = 1;
                points[index].x += points[0].step;
            }
        '''})
        self.assertEqual(slot(audit, 'step', kind='field')['classification'], 'review-numeric-use')
        self.assertEqual(slot(audit, 'index', kind='local')['classification'], 'review-numeric-use')

    def test_complete_memset_preserves_zero_but_word_fills_are_not_boolean_one(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            void *memset(void *destination, int value, unsigned long size);
            struct State { unsigned char ready; int word; } state;
            void run(void) {
                memset(&state, 0, sizeof state);
                state.ready = 1;
                memset(&state.word, 1, sizeof state.word);
            }
        '''})
        self.assertEqual(slot(audit, 'ready', kind='field')['values'], ['0', '1'])
        self.assertIn('other', slot(audit, 'word', kind='field')['values'])

    def test_falling_off_nonvoid_function_is_not_a_boolean_return_proof(self) -> None:
        audit = audit_sources({'src/probe.c': '''
            int partial(int value) { if (value > 0) return 1; if (value < 0) return 0; }
        '''})
        self.assertIn('unknown', slot(audit, 'partial')['values'])
        self.assertNotEqual(slot(audit, 'partial')['classification'], 'candidate')


    def test_array_zero_fill_does_not_invent_a_second_value(self) -> None:
        audit = audit_sources({'src/probe.c': """
            int complete[2][2] = {{1, 1}, {1, 1}};
            int omitted[2][2] = {{1}, {1}};
        """})
        self.assertEqual(slot(audit, 'complete', kind='global')['values'], ['1'])
        self.assertEqual(slot(audit, 'omitted', kind='global')['values'], ['0', '1'])

    def test_numeric_use_propagates_through_parameters_and_copies(self) -> None:
        audit = audit_sources({'src/probe.c': """
            int values[2];
            int select(int index) { return values[index]; }
            void run(int condition) {
                int sequence = condition != 0;
                int copy = sequence;
                select(copy);
            }
        """})
        for name in ('sequence', 'copy'):
            self.assertEqual(slot(audit, name, kind='local')['classification'], 'review-numeric-use')

    def test_bitfield_storage_needs_review(self) -> None:
        audit = audit_sources({'src/probe.c': """
            struct State { signed int flag : 1; } state;
            void run(int input) { state.flag = input != 0; }
        """})
        self.assertEqual(slot(audit, 'flag', kind='field')['classification'],
                         'review-storage-conversion')

    def test_parallel_results_are_deterministic(self) -> None:
        sources = {
            'include/probe.h': 'extern int flag; void set(int *out);',
            'src/a.c': '#include <probe.h>\nint flag; void set(int *out) { *out = 1; }',
            'src/b.c': '#include <probe.h>\nvoid run(void) { set(&flag); }',
        }
        self.assertEqual(audit_sources(sources, jobs=1), audit_sources(sources, jobs=2))

    def test_parse_and_coverage_errors_are_not_silently_dropped(self) -> None:
        with self.assertRaisesRegex(RuntimeError, 'parsing failed'):
            audit_sources({'src/probe.c': 'int broken( { return 1; }'})
        sources = {'src/probe.c': 'int flag;', 'include/orphan.h': 'extern int other;'}
        with self.assertRaisesRegex(RuntimeError, 'incomplete.*coverage'):
            audit_sources(sources)
        partial = audit_sources(sources, names=('game.probe',))
        self.assertTrue(partial['coverage']['partial_selection'])
        self.assertEqual(partial['coverage']['missing_headers'], ['include/orphan.h'])


if __name__ == '__main__':
    unittest.main()
