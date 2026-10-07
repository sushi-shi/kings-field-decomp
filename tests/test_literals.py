"""Controls for the literal sink census and its value-flow domains."""

from __future__ import annotations

from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.literals import build_domains, collect, load_kf2, typed_enum_literals
from scripts.kf.manifest import Manifest, Profile, Unit


def census(sources: dict[str, str], *, kf2: dict[str, str] | None = None,
           hub_degree: int = 12) -> dict:
    with TemporaryDirectory() as directory:
        repo = Path(directory) / "repo"
        for name, source in sources.items():
            path = repo / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(source)
        (repo / "include").mkdir(exist_ok=True)
        sdk = repo / "sdk"
        sdk.mkdir()
        kf2_root = None
        if kf2 is not None:
            kf2_root = Path(directory) / "kf2"
            for name, source in kf2.items():
                path = kf2_root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(source)
            (kf2_root / "src").mkdir(exist_ok=True)
        profile = Profile("c", "c", "gcc257-native", "O2", 0, "1.07", ())
        units = tuple(Unit(f"game.{Path(name).stem}", "GAME.EXE", name, "c", ())
                      for name in sources if name.endswith(".c"))
        return collect(repo=repo, sdk=sdk, manifest=Manifest({"c": profile}, units), jobs=1,
                       kf2_root=kf2_root, hub_degree=hub_degree)


def sinks(report: dict, line: int, file: str = "src/probe.c") -> list[tuple[str, str, int]]:
    return sorted((row["sink"]["kind"], row["sink"]["target"], row["value"])
                  for row in report["sites"] if row["file"] == file
                  and row["line"] == line and row["origin"] in {"literal", "character"})


def domain_of(report: dict, slot: str) -> dict:
    matches = [domain for domain in report["domains"]
               if slot in {member["slot"] for member in domain["members"]}]
    if len(matches) != 1:
        raise AssertionError((slot, matches))
    return matches[0]


HEADER = """
#define ACTION_LIMIT 0x40
#define TWICE(value) ((value) * 2)
enum { KIND_DOOR = 5 };
typedef struct Template { unsigned char kind; unsigned char flags; } Template;
typedef struct Object { unsigned char action; short timer; int *data; } Object;
typedef char template_size[sizeof(Template) == 2 ? 1 : -1];
void start(Object *object, unsigned char action);
int lookup(int index);
extern int table[16];
"""


class LiteralCensusTests(unittest.TestCase):
    def test_each_literal_names_its_sink(self) -> None:
        report = census({"include/probe.h": HEADER, "src/probe.c": """#include <probe.h>
int table[16];
int probe(Object *object, const Template *definition) {
    object->action = 5;
    if (object->action == 7) start(object, 9);
    switch (definition->kind) { case 0x58: return 3; default: break; }
    object->timer += 30;
    object->timer = table[4] << 2;
    object->data = 0;
    if (definition->flags & 0x20) return -1;
    return TWICE(6) + ACTION_LIMIT;
}
"""})
        self.assertEqual(sinks(report, 4), [("assign", "Object.action", 5)])
        self.assertEqual(sinks(report, 5), [("argument", "start:arg1", 9),
                                            ("compare", "Object.action", 7)])
        self.assertEqual(sinks(report, 6), [("case", "Template.kind", 0x58),
                                            ("return", "probe:return", 3)])
        self.assertEqual(sinks(report, 7), [("arithmetic", "Object.timer", 30)])
        self.assertEqual(sinks(report, 8), [("index", "[table]", 4), ("shift", "table", 2)])
        self.assertEqual(sinks(report, 9), [("pointer-zero", "Object.data", 0)])
        self.assertEqual(sinks(report, 10), [("mask", "Template.flags", 0x20),
                                             ("return", "probe:return", -1)])
        # A macro argument is a written literal; a macro body is a named constant.
        self.assertEqual(sinks(report, 11), [("arithmetic", "", 6)])
        macro = [row for row in report["sites"] if row["origin"] == "macro-constant"]
        self.assertEqual(sorted((row["constant"], row["value"], row["line"]) for row in macro),
                         [("ACTION_LIMIT", 0x40, 11), ("TWICE", 2, 11)])
        classes = {row["sink"]["kind"] for row in report["sites"]
                   if row["file"] == "include/probe.h"}
        self.assertEqual(classes, {"declaration", "enum-definition"})

    def test_flow_joins_one_domain_and_quantities_stay_apart(self) -> None:
        report = census({"include/probe.h": HEADER, "src/probe.c": """#include <probe.h>
void start(Object *object, unsigned char action) {
    if (object->action == 0xff) object->action = action;
}
void load(Object *object, const Template *definition) {
    switch (definition->kind) {
    case 2:
        object->action = 2;
        break;
    case 3:
        object->action = 3;
        break;
    case 4:
        object->timer = 4;
        break;
    case KIND_DOOR:
        start(object, definition->kind);
        break;
    }
}
void tick(Object *object, int frames) {
    object->timer = frames * 3;
    if (object->timer == 4) object->timer = 0;
}
int count(int frames) { return frames; }
"""})
        action = domain_of(report, "Object.action")
        self.assertEqual({member["slot"] for member in action["members"]},
                         {"Object.action", "Template.kind", "start:arg1"})
        self.assertEqual([row["value"] for row in action["values"]], [2, 3, 4, 0xff])
        self.assertEqual([row["name"] for row in action["constants_used"]], ["KIND_DOOR"])
        self.assertEqual(action["verdict"], "enum-candidate")
        self.assertIn("case-echo=3", " ".join(action["edges"]))
        # One coincidental echo (case 4 into the timer) does not join a domain.
        self.assertNotIn("Object.timer", {member["slot"] for member in action["members"]})
        timer = domain_of(report, "Object.timer")
        self.assertEqual(timer["verdict"], "quantity")
        self.assertNotIn("tick:arg1", {member["slot"] for member in timer["members"]})

    def test_literals_reaching_enum_typed_slots_are_reported(self) -> None:
        sources = {"include/probe.h": """
#define KF_ENUM_BEGIN(name, storage) typedef storage name; enum {
#define KF_ENUM_END(name) };
#define KF_ENUM_PROMOTED(name) int
KF_ENUM_BEGIN(Op, unsigned char)
    OP_IDLE = 0xff,
    OP_RUN = 5
KF_ENUM_END(Op)
typedef struct Object { Op op; unsigned char other; } Object;
""", "src/probe.c": """#include <probe.h>
void tick(Object *object) {
    KF_ENUM_PROMOTED(Op) local = object->op;
    object->op = OP_RUN;
    object->other = 5;
    if (local == OP_IDLE) object->op = (Op)5;
    switch (local) { case 7: break; }
}
"""}
        with TemporaryDirectory() as directory:
            repo = Path(directory)
            for name, source in sources.items():
                (repo / name).parent.mkdir(parents=True, exist_ok=True)
                (repo / name).write_text(source)
            (repo / "sdk").mkdir()
            profile = Profile("c", "c", "gcc257-native", "O2", 0, "1.07", ())
            unit = Unit("game.probe", "GAME.EXE", "src/probe.c", "c", ())
            sites = typed_enum_literals(repo=repo, sdk=repo / "sdk", jobs=1,
                                        manifest=Manifest({"c": profile}, (unit,)))
        self.assertEqual(sorted((row["line"], row["spelling"], row["sink"]["target"])
                                for row in sites),
                         [(6, "5", "Object.op"), (7, "7", "tick::local")])

    def test_encoded_and_masked_enum_reads_are_integer_views(self) -> None:
        sources = {"include/probe.h": """
#define KF_ENUM_BEGIN(name, storage) typedef storage name; enum {
#define KF_ENUM_END(name) };
#define KF_ENUM_ENCODE(storage, value) ((storage)(value))
KF_ENUM_BEGIN(Phase, unsigned char)
    PHASE_IDLE = 0,
    PHASE_RUN = 4
KF_ENUM_END(Phase)
typedef struct Object { Phase phase; } Object;
""", "src/probe.c": """#include <probe.h>
int tick(Object *object, Phase direction) {
    if (KF_ENUM_ENCODE(int, direction) < 0) return 1;
    if ((object->phase & 3) == 0) return 2;
    if (object->phase == 5) return 3;
    return 0;
}
"""}
        with TemporaryDirectory() as directory:
            repo = Path(directory)
            for name, source in sources.items():
                (repo / name).parent.mkdir(parents=True, exist_ok=True)
                (repo / name).write_text(source)
            (repo / "sdk").mkdir()
            profile = Profile("c", "c", "gcc257-native", "O2", 0, "1.07", ())
            unit = Unit("game.probe", "GAME.EXE", "src/probe.c", "c", ())
            report = collect(repo=repo, sdk=repo / "sdk", jobs=1,
                             manifest=Manifest({"c": profile}, (unit,)))
        typed = sorted((row["line"], row["sink"]["target"]) for row in report["sites"]
                       if row["origin"] == "literal" and row["class"] == "typed-enum")
        # Only the direct comparison with the stored enum is a typed literal.
        self.assertEqual(typed, [(5, "Object.phase")])
        self.assertEqual(sinks(report, 3)[0], ("compare", "tick:arg1#raw", 0))
        self.assertEqual(report["slots"]["tick:arg1#raw"]["enum_domain"], "")
        self.assertEqual(report["slots"]["Object.phase&0x3"]["enum_domain"], "")

    def test_unprototyped_arguments_keep_their_position_and_type(self) -> None:
        report = census({"include/probe.h": "void *memset();\n", "src/probe.c": """#include <probe.h>
char buffer[8];
void clear(void) { memset((void *)buffer, 0xff, 8); memset(buffer, 0, 0); }
"""})
        self.assertEqual(sinks(report, 3), [("argument", "memset:arg1", 0),
                                            ("argument", "memset:arg1", 0xff),
                                            ("argument", "memset:arg2", 0),
                                            ("argument", "memset:arg2", 8)])

    def test_typed_storage_types_only_the_name_it_precedes(self) -> None:
        report = census({"include/probe.h": """
#define KF_ENUM_BEGIN(name, storage) typedef storage name; enum {
#define KF_ENUM_END(name) };
#define KF_ENUM_PARAM(name, storage) storage
#define KF_ENUM_STORAGE(name, storage) storage
KF_ENUM_BEGIN(Op, unsigned short)
    OP_IDLE = 0xff
KF_ENUM_END(Op)
typedef struct Object { KF_ENUM_STORAGE(Op, unsigned char) op; unsigned char other; } Object;
int count(KF_ENUM_PARAM(Op, int) index, int amount);
""", "src/probe.c": """#include <probe.h>
int count(KF_ENUM_PARAM(Op, int) index, int amount) { return index + amount; }
int probe(Object *object) {
    object->op = OP_IDLE;
    object->other = 1;
    return count(object->op, 2) == 3;
}
"""})
        domains = {key: slot["enum_domain"] for key, slot in report["slots"].items()
                   if key.startswith(("count:", "Object."))}
        self.assertEqual(domains["Object.op"], "Op")
        self.assertEqual(domains["Object.other"], "")
        self.assertEqual(domains["count:arg0"], "Op")
        self.assertEqual(domains["count:arg1"], "")
        self.assertEqual(domains["count:return"], "")

    def test_hubs_do_not_weld_domains(self) -> None:
        sites = [{"sink": {"kind": "assign", "target": name}, "value": value,
                  "origin": "literal", "constant": "", "constant_file": "", "class": "assign",
                  "function": "f", "file": "src/a.c", "line": 1}
                 for name, value in (("A.a", 1), ("B.b", 2), ("C.c", 3))]
        edges = [{"left": "A.a", "right": "hub", "kind": "copy", "file": "src/a.c", "line": 1},
                 {"left": "B.b", "right": "hub", "kind": "copy", "file": "src/a.c", "line": 2},
                 {"left": "C.c", "right": "hub", "kind": "copy", "file": "src/a.c", "line": 3}]
        domains, hubs = build_domains(sites, edges, {}, set(), {}, hub_degree=2)
        self.assertEqual(hubs, ["hub"])
        self.assertEqual(len(domains), 3)
        domains, hubs = build_domains(sites, edges, {}, set(), {}, hub_degree=3)
        self.assertEqual((len(domains), hubs), (1, []))

    def test_kf2_counterparts_prefer_qualified_slots(self) -> None:
        kf2 = {"include/kf/map.h": """
KF_ENUM_BEGIN(KfMapObjectOperation, u8)
    KF_MAP_OBJECT_OP_LIFT_DOOR = 2,
    KF_MAP_OBJECT_OP_NONE = 255
KF_ENUM_END(KfMapObjectOperation)
KF_ENUM_BEGIN(KfActorAction, u8)
    KF_ACTOR_ACTION_IDLE = 0,
    KF_ACTOR_ACTION_WALK = 2
KF_ENUM_END(KfActorAction)
typedef struct Actor { KF_ENUM_STORAGE(KfActorAction, u8) action; } Actor;
typedef struct Object { KfMapObjectOperation action; /* runtime */ } Object;
void start(Object *object, KfMapObjectOperation action);
"""}
        with TemporaryDirectory() as directory:
            root = Path(directory)
            for name, text in kf2.items():
                (root / name).parent.mkdir(parents=True, exist_ok=True)
                (root / name).write_text(text)
            (root / "src").mkdir()
            loaded = load_kf2(root)
        self.assertEqual(loaded["qualified"]["Object.action"], ["KfMapObjectOperation"])
        self.assertEqual(loaded["qualified"]["Actor.action"], ["KfActorAction"])
        self.assertEqual(loaded["qualified"]["start:arg1"], ["KfMapObjectOperation"])
        self.assertEqual(loaded["enums"]["KfMapObjectOperation"]["members"],
                         {"KF_MAP_OBJECT_OP_LIFT_DOOR": 2, "KF_MAP_OBJECT_OP_NONE": 255})
        report = census({"include/probe.h": HEADER, "src/probe.c": """#include <probe.h>
void reset(Object *object) { object->action = 0xff; object->action = 2; }
"""}, kf2=kf2)
        best = domain_of(report, "Object.action")["kf2"][0]
        self.assertEqual(best["enum"], "KfMapObjectOperation")
        self.assertEqual(best["value_overlap"], 2)


if __name__ == "__main__":
    unittest.main()
