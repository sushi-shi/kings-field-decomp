from pathlib import Path
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from scripts.kf.codec_oracle import ORACLES, main


class CompleteCodecOracleTests(unittest.TestCase):
    def run_oracles(self, args, fail_at=None):
        observed = []

        def load(module):
            def run(arguments):
                observed.append((module, arguments))
                return int(module == fail_at)
            return SimpleNamespace(main=run)

        with (
            patch("scripts.kf.codec_oracle.build_driver", return_value=Path("/driver")),
            patch("scripts.kf.codec_oracle.import_module", side_effect=load),
            patch("builtins.print") as output,
        ):
            result = main(args)
        if fail_at is not None:
            self.assertFalse(any("PASS:" in str(call) for call in output.call_args_list))
        return result, observed

    def test_default_runs_every_family_without_case_limits_or_stale_objects(self):
        result, observed = self.run_oracles([])
        self.assertEqual(result, 0)
        self.assertEqual(
            [module for module, _ in observed],
            [f"scripts.kf.{name}_oracle" for name in ORACLES],
        )
        self.assertEqual(len(observed), 10)
        self.assertTrue(all(args == ["--rust-driver", "/driver"] for _, args in observed))

    def test_first_failure_prevents_an_aggregate_pass(self):
        result, observed = self.run_oracles([], "scripts.kf.resource_oracle")
        self.assertEqual(result, 1)
        self.assertEqual(len(observed), 2)

    def test_reusing_c_objects_requires_the_explicit_option(self):
        result, observed = self.run_oracles(["--no-rebuild"])
        self.assertEqual(result, 0)
        self.assertTrue(all(args[-1] == "--no-rebuild" for _, args in observed))
