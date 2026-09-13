"""Resource access keeps grid domains, complete extents, and byte offsets."""

import os
from pathlib import Path
import re
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.paths import REPO


class ResourceAccessViewTests(unittest.TestCase):
    def compile(self, source: str, mode: str = "modern", *, ir: bool = False):
        compiler = shutil.which("clang")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if not compiler or not sdk:
            self.skipTest("target Clang and SDK required; run in nix develop")
        with TemporaryDirectory(prefix="kf-resource-views-") as directory:
            path = Path(directory) / "control.c"
            path.write_text(
                "#include <kf/map_data.h>\n#include <kf/tmd.h>\n"
                "#include <kf/game_map.h>\n#include <kf/resources.h>\n" + source
            )
            output = ["-O1", "-S", "-emit-llvm", "-o", "-"] if ir else ["-fsyntax-only"]
            return subprocess.run(
                [compiler, *FLAGS, *MODES[mode], *output,
                 "-Werror=incompatible-pointer-types", "-Werror=incompatible-pointer-types-discards-qualifiers",
                 "-I", str(REPO / "include"), "-I", str(REPO / "vendor/include"),
                 "-I", sdk, str(path)], capture_output=True, text=True,
            )

    def test_grid_views_share_extent_offsets_and_enum_domains(self):
        grids = {
            "KfMapGrid": "u8", "KfMapAttributeGrid": "KfMapAttribute",
            "KfMapCollisionGrid": "KfMapCellKind", "KfMapOrientationGrid": "KfMapOrientation",
        }
        checks = []
        for grid, element in grids.items():
            checks.append(f"typedef char size_{grid}[sizeof({grid}) == 10000 ? 1 : -1];")
            checks.append(f"typedef char align_{grid}[__alignof__({grid}) == __alignof__(u32) ? 1 : -1];")
            checks.append(f"typedef char words_size_{grid}[sizeof((({grid} *)0)->words) == 10000 ? 1 : -1];")
            checks.append(f"u32 *words_{grid}({grid} *grid) {{ return grid->words; }}")
            checks.append(f"typedef char linear_size_{grid}[sizeof((({grid} *)0)->linear) == 10000 ? 1 : -1];")
            for index in (0, 99, 100, 101, 9999):
                row, col = divmod(index, 100)
                checks.append(
                    f"typedef char offset_{grid}_{index}["
                    f"__builtin_offsetof({grid}, cells[{row}][{col}]) == {index} && "
                    f"__builtin_offsetof({grid}, linear[{index}]) == {index} ? 1 : -1];"
                )
            checks.append(f"{element} *view_{grid}({grid} *grid) {{ return &grid->linear[100]; }}")
        for mode in ("retail", "modern"):
            with self.subTest(mode=mode):
                result = self.compile("\n".join(checks), mode)
                self.assertEqual(result.returncode, 0, result.stderr)
        for grid in list(grids)[1:]:
            result = self.compile(f"void bad({grid} *grid) {{ grid->linear[100] = 7; }}")
            self.assertNotEqual(result.returncode, 0, grid)

    def test_copy_api_accepts_word_storage_but_not_const_destinations(self):
        for function in ("map_resource_copy_words", "resource_stream_copy_words"):
            body = f"const u32 *copy(KfMapOrientationGrid *grid, const u32 *source) {{ return {function}(grid->words, source, 2500); }}"
            for mode in ("retail", "modern"):
                with self.subTest(function=function, mode=mode):
                    result = self.compile(body, mode)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    const_body = body.replace("KfMapOrientationGrid *grid", "const KfMapOrientationGrid *grid")
                    result = self.compile(const_body, mode)
                    self.assertNotEqual(result.returncode, 0)
                    result = self.compile(body.replace("grid->words", "grid->linear"), mode)
                    self.assertNotEqual(result.returncode, 0)

    def test_tmd_vertices_use_payload_relative_bytes_and_evaluate_once(self):
        source = """
            SVECTOR *offset(KfTmdHeader *asset) {
                KfTmdObject object = {28, 2, 0, 0, 0, 0, 0};
                return TMD_OBJECT_VERTICES(asset, &object);
            }
            KfTmdHeader *next_asset(void);
            KfTmdObject *next_object(void);
            SVECTOR *once(void) {
                return TMD_OBJECT_VERTICES(next_asset(), next_object());
            }
        """
        result = self.compile(source, "retail", ir=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        offset = result.stdout.split("@offset(", 1)[1].split("}", 1)[0]
        self.assertRegex(offset, r"getelementptr[^\n]*i8[^\n]*i32 40")
        for name in ("next_asset", "next_object"):
            self.assertEqual(len(re.findall(r"call[^\n]*@" + name + r"\(", result.stdout)), 1)


if __name__ == "__main__":
    unittest.main()
