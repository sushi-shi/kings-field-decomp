"""An SDK use in one image must not promote the same address in another."""

import unittest

from scripts.kf.sdk_usage import usage_rows


class SdkUsageTest(unittest.TestCase):
    def test_image_identity_and_weak_evidence_survive_export(self):
        inventory = [{"image": image, "va": "0x1000"} for image in ("GAME.EXE", "OPEN.EXE")]
        reports = [{"ranges": [{"image": "GAME.EXE", "va": 0x1000,
                                "kind": "function", "reachability": "candidate",
                                "via_owner": "function:00002000", "via_reference": 0}]}]
        rows = usage_rows(inventory, reports)
        self.assertEqual(rows[0]["reference_status"], "candidate")
        self.assertEqual(rows[0]["via_reference"], 0)
        self.assertEqual(rows[1]["reference_status"], "unreached")
        self.assertEqual(rows[1]["via_owner"], "")
