"""AST parameter inventory preserves semantic domains and review boundaries."""

from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.parameters import collect, main, scan_file, write_tsv


def unit(source="probe.c", image="GAME.EXE", name="game.probe", defines=()):
    return SimpleNamespace(source=source, image=image, unit=name, defines=defines)


class ParametersTest(unittest.TestCase):
    def test_aliases_const_arrays_unions_callbacks_and_unnamed(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "probe.c"
            source.write_text("""
typedef enum Mode { MODE_A } Mode;
typedef struct State { int x; } State;
typedef State *StatePointer;
union Value { int x; char y; };
void probe(Mode mode, const State *source, State destination[4],
           StatePointer *slot, union Value value, Mode,
           void (*callback)(State *), int count);
""")
            rows = scan_file(source, [*MODES["retail"], *FLAGS], root=root)["parameters"]
            self.assertEqual(len(rows), 9)
            self.assertEqual([r["index"] for r in rows[:8]], list(range(8)))
            self.assertEqual([r["domains"][0]["kind"] for r in rows[:6]],
                             ["enum", "struct", "struct", "struct", "union", "enum"])
            self.assertIn("const", rows[1]["type"])
            self.assertIn("array", rows[2]["domains"][0]["route"])
            self.assertEqual(rows[5]["name"], "")
            self.assertIn("argument:0", rows[6]["domains"][0]["route"])
            self.assertEqual(rows[7]["domains"], [])
            self.assertEqual(rows[8]["callable_category"], "callback_signature")
            self.assertEqual(rows[8]["function"], "probe::callback")
            self.assertEqual(rows[8]["name"], "")

    def test_callback_typedef_and_field_parameters_have_owners(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "probe.c"
            source.write_text("""
struct S { int value; };
typedef void (*Handler)(struct S *state);
struct T { void (*callback)(struct S *record); };
""")
            report = scan_file(source, [*MODES["retail"], *FLAGS], root=root)
            self.assertEqual([(r["function"], r["name"]) for r in report["parameters"]],
                             [("Handler", "state"), ("T::callback", "record")])
            self.assertEqual(report["unowned_parameters"], [])

    def test_macro_overloads_and_internal_functions_remain_distinct(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "probe.c").write_text("""
typedef struct State { int x; } State;
#ifdef __cplusplus
#define HELPERS(name) void name(State *value); void name(const State *value);
HELPERS(helper)
#endif
static void local(State *record) {}
""")
            (root / "other.c").write_text(
                "typedef struct State { int x; } State;\nstatic void local(State *other) {}\n")
            report = collect([unit(), unit(source="other.c", name="game.other")],
                             "clang", root, root=root)
            helpers = [row for row in report["parameters"] if row["function"] == "helper"]
            self.assertEqual(len(helpers), 2)
            self.assertEqual({row["expansion_occurrence"] for row in helpers}, {0, 1})
            groups = [g for g in report["groups"] if g["function"] == "helper"]
            self.assertEqual(len(groups), 2)
            self.assertTrue(all(g["overloaded_family"] for g in groups))
            local = [g for g in report["groups"] if g["function"] == "local"]
            self.assertEqual(len(local), 2)
            self.assertTrue(all(not g["spelling_difference"] for g in local))

    def test_same_usr_callback_fields_in_unrelated_tus_are_separate(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "probe.c").write_text(
                "struct A { int x; }; struct T { void (*callback)(struct A *first); };\n")
            (root / "other.c").write_text(
                "struct B { char x; }; struct T { void (*callback)(struct B *second); };\n")
            report = collect([unit(), unit(source="other.c", name="game.other")],
                             "clang", root, root=root)
            groups = report["groups"]
            self.assertEqual(len(groups), 2)
            self.assertEqual({g["function"] for g in groups}, {"T::callback"})
            self.assertTrue(all(not g["spelling_difference"] for g in groups))

    def test_dual_view_recovers_erased_enum_aliases_and_storage(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "probe.c").write_text("""
#ifdef __cplusplus
enum class Mode : unsigned char { A };
template<class Enum, class Storage> class KfEnumStorage { Storage value; };
#define PARAM(domain, storage) domain
#define STORAGE(domain, storage) KfEnumStorage<domain, storage>
#else
typedef unsigned char Mode;
#define PARAM(domain, storage) storage
#define STORAGE(domain, storage) storage
#endif
typedef Mode ModeAlias;
typedef STORAGE(Mode, unsigned int) ResultWord;
void probe(PARAM(Mode, int) mode, ModeAlias alias, ResultWord result);
""")
            report = collect([unit()], "clang", root, root=root)
            rows = report["parameters"]
            self.assertEqual(len(rows), 3)
            self.assertTrue(all(row["selected"] for row in rows))
            for row in rows:
                self.assertEqual(row["modes"], ["modern", "retail"])
                self.assertEqual(row["observations"][0]["domains"][0]["name"], "Mode")
                self.assertEqual(row["observations"][1]["domains"], [])
            self.assertIn("enum_storage", rows[2]["observations"][0]["domains"][0]["route"])
            self.assertEqual(len(report["by_type"]), 1)
            domain = report["by_type"][0]
            self.assertEqual(domain["type"], "Mode")
            self.assertEqual(domain["names"], ["alias", "mode", "result"])
            self.assertEqual(len(domain["sites"]), 3)
            result = next(use for use in domain["uses"] if use["name"] == "result")
            self.assertEqual(result["routes"], ["value/enum_storage"])

    def test_declarations_definitions_image_variants_and_unseen_headers(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "include").mkdir()
            (root / "include/shared.h").write_text("""
typedef struct State { int x; } State;
void transform(State *vector);
#ifdef EXTRA
void extra(State *state);
#endif
""")
            (root / "include/unseen.h").write_text("void unseen(void);\n")
            (root / "probe.c").write_text(
                '#include "shared.h"\nvoid transform(State *destination) {}\n')
            report = collect([unit(), unit(image="OPEN.EXE", name="open.probe",
                                          defines=("EXTRA",))], "clang", root, root=root)
            rows = report["parameters"]
            self.assertEqual(len(rows), 3)
            self.assertEqual(report["coverage"]["unseen_headers"], ["include/unseen.h"])
            groups = [g for g in report["groups"] if g["function"] == "transform"]
            self.assertEqual(len(groups), 2)
            self.assertEqual({g["image"] for g in groups}, {"GAME.EXE", "OPEN.EXE"})
            self.assertTrue(all(g["spelling_difference"] for g in groups))
            self.assertEqual(groups[0]["names"], ["destination", "vector"])
            shared = next(r for r in rows if r["name"] == "vector")
            self.assertEqual(len(shared["observations"]), 4)
            extra = next(r for r in rows if r["function"] == "extra")
            self.assertEqual({o["image"] for o in extra["observations"]}, {"OPEN.EXE"})
            domains = {g["image"]: g for g in report["by_type"]}
            self.assertEqual(domains["GAME.EXE"]["names"], ["destination", "vector"])
            self.assertEqual(domains["OPEN.EXE"]["names"], ["destination", "state", "vector"])
            self.assertEqual({use["function"] for use in domains["OPEN.EXE"]["uses"]},
                             {"transform", "extra"})
            with redirect_stdout(StringIO()) as output:
                write_tsv(report, by_type=True)
            self.assertIn("OPEN.EXE\tState\tstruct", output.getvalue())
            self.assertIn("\tstate\t", output.getvalue())

    def test_type_index_does_not_merge_unrelated_same_named_records(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "probe.c").write_text("struct State { int x; }; void a(struct State *first);\n")
            (root / "other.c").write_text("struct State { char x; }; void b(struct State *second);\n")
            report = collect([unit(), unit(source="other.c", name="game.other")],
                             "clang", root, root=root)
            groups = report["by_type"]
            self.assertEqual(len(groups), 2)
            self.assertEqual({g["type"] for g in groups}, {"State"})
            self.assertEqual({g["declaration"]["file"] for g in groups}, {"probe.c", "other.c"})
            self.assertTrue(all(len(g["names"]) == 1 for g in groups))

    def test_sdk_types_at_project_functions_and_modern_helpers(self):
        with TemporaryDirectory() as directory, TemporaryDirectory() as sdk_directory:
            root, sdk = Path(directory), Path(sdk_directory)
            (sdk / "sdk.h").write_text(
                "typedef struct { short x, y, z; } SVECTOR;\nvoid sdk_call(SVECTOR *sdk_arg);\n")
            (root / "probe.c").write_text("""
#include <sdk.h>
void project_call(const SVECTOR *position);
#ifdef __cplusplus
template<class T> void helper(T value) {}
struct Record {};
void modern_helper(Record &record) {}
#endif
""")
            report = collect([unit()], "clang", sdk, root=root)
            rows = report["parameters"]
            self.assertEqual({r["function"] for r in rows},
                             {"project_call", "helper", "modern_helper"})
            project = next(r for r in rows if r["function"] == "project_call")
            self.assertTrue(project["selected"])
            self.assertEqual(project["observations"][0]["domains"][0]["file"], str(sdk / "sdk.h"))
            helper = next(r for r in rows if r["function"] == "helper")
            self.assertEqual(helper["modes"], ["modern"])
            self.assertFalse(helper["selected"])
            modern = next(r for r in rows if r["function"] == "modern_helper")
            self.assertTrue(modern["selected"])
            self.assertIn("LValueReference", modern["observations"][0]["domains"][0]["route"])
            with redirect_stdout(StringIO()) as output:
                write_tsv(report)
            self.assertIn("project_call", output.getvalue())
            self.assertNotIn("\thelper\t", output.getvalue())

    def test_parse_errors_and_cli_json(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "probe.c"
            source.write_text("void broken(Missing parameter);\n")
            with self.assertRaisesRegex(ValueError, "parsing failed"):
                scan_file(source, [*MODES["retail"], *FLAGS], root=root)
        report = {"parameters": [], "variants": 1,
                  "coverage": {"unseen_headers": [], "unowned_parameters": []}}
        with patch("scripts.kf.parameters.load_manifest"), \
                patch("scripts.kf.parameters.select_units", return_value=[unit()]), \
                patch("scripts.kf.parameters.environment", return_value=("clang", Path("/tmp"))), \
                patch("scripts.kf.parameters.collect", return_value=report) as scan, \
                redirect_stdout(StringIO()) as output, redirect_stderr(StringIO()):
            self.assertEqual(main(["--json"]), 0)
            self.assertEqual(json.loads(output.getvalue()), report)
            output.truncate(0)
            output.seek(0)
            scan.side_effect = ValueError("bad parse")
            self.assertEqual(main(["--json"]), 2)
            self.assertEqual(output.getvalue(), "")


if __name__ == "__main__":
    unittest.main()
