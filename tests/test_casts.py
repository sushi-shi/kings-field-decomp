"""Tests for the target-C AST cast census."""

from __future__ import annotations

import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.casts import collect, document, filter_sites, summary
from scripts.kf.manifest import Manifest, Profile, Unit


def manifest() -> Manifest:
    profile = Profile("c", "c", "gcc257-native", "O2", 0, "1.07", ())
    return Manifest({"c": profile}, (
        Unit("game.cast_probe", "GAME.EXE", "src/probe.c", "c", ()),
        Unit(
            "open.cast_probe", "OPEN.EXE", "src/probe.c", "c", (),
            defines=("OPEN_VARIANT",),
        ),
    ))


class CastAuditTest(unittest.TestCase):
    def test_target_ast_deduplicates_source_header_and_macro_sites(self) -> None:
        with TemporaryDirectory() as directory:
            repo = Path(directory)
            (repo / "src").mkdir()
            (repo / "include").mkdir()
            sdk = repo / "sdk"
            sdk.mkdir()
            (repo / "include/casts.h").write_text(
                "#define TO_WORD(value) ((unsigned long)(value))\n"
                "#define TO_POINTER(value) ((void *)(value))\n",
                encoding="utf-8",
            )
            (repo / "src/probe.c").write_text(
                "#include <casts.h>\n"
                "void *probe(long value, void *pointer)\n"
                "{\n"
                "    unsigned long word = (unsigned short)value;\n"
                "    word += TO_WORD(value);\n"
                "    return word ? TO_POINTER(pointer) : (void *)0;\n"
                "}\n",
                encoding="utf-8",
            )

            audit = collect(
                repo=repo, manifest=manifest(), sdk=sdk, jobs=2,
            )
            self.assertEqual(audit.variants, 2)
            self.assertEqual(len(audit.sites), 4)
            self.assertEqual(summary(audit.sites)["by_kind"], {
                "pointer": 2,
                "scalar": 2,
            })
            self.assertEqual(
                {site.location.file for site in audit.sites},
                {"src/probe.c", "include/casts.h"},
            )
            macro = next(site for site in audit.sites
                         if site.location.file == "include/casts.h"
                         and site.category == "pointer")
            self.assertEqual(len(macro.contexts), 2)
            self.assertEqual({context.image for context in macro.contexts},
                             {"GAME.EXE", "OPEN.EXE"})

    def test_filters_and_json_document_keep_written_site_counts(self) -> None:
        with TemporaryDirectory() as directory:
            repo = Path(directory)
            (repo / "src").mkdir()
            (repo / "include").mkdir()
            sdk = repo / "sdk"
            sdk.mkdir()
            (repo / "include/casts.h").write_text(
                "#define TO_POINTER(value) ((void *)(value))\n",
                encoding="utf-8",
            )
            (repo / "src/probe.c").write_text(
                "#include <casts.h>\n"
                "void *probe(long value) { return TO_POINTER(value); }\n",
                encoding="utf-8",
            )
            audit = collect(repo=repo, manifest=manifest(), sdk=sdk, jobs=1)

            sites = filter_sites(
                audit.sites, kind="pointer", scope="header", paths=("include/",),
            )
            self.assertEqual(len(sites), 1)
            payload = document(audit, sites)
            self.assertEqual(payload["summary"]["total"], 1)
            self.assertEqual(json.loads(json.dumps(payload))["schema"], 1)


if __name__ == "__main__":
    unittest.main()
