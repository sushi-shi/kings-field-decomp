"""Moved executable islands must retain one-to-one raw-byte accounting."""

import random
import struct
import unittest

from scripts.kf.executable_diff import compare_executables, compare_payloads
from scripts.kf.sema.image import RetailImage


class ExecutableIslandTests(unittest.TestCase):
    def setUp(self):
        rng = random.Random(17)
        self.a, self.b, self.c = (rng.randbytes(size) for size in (400, 280, 320))

    def assert_partition(self, report):
        for side in ('retail', 'candidate'):
            spans = [(item[f'{side}_offset'], item[f'{side}_size'])
                     for item in report['islands']]
            spans += [(item['offset'], item['size']) for item in report[f'{side}_unanchored']]
            cursor = 0
            for start, size in sorted(spans):
                self.assertEqual(start, cursor)
                self.assertGreater(size, 0)
                cursor += size
            self.assertEqual(cursor, report[f'{side}_bytes'])
            self.assertLessEqual(report['equal_paired_bytes'], cursor)

    def test_reordered_regions_match_without_reusing_positions(self):
        retail = self.a + self.b + self.c
        candidate = self.c + self.a + self.b
        report = compare_payloads(retail, candidate)
        self.assertEqual(report['byte_similarity_percent'], 100)
        self.assertGreater(report['island_count'], 1)
        self.assert_partition(report)
        for island in report['islands']:
            a, b, size = island['retail_offset'], island['candidate_offset'], island['retail_size']
            self.assertEqual(retail[a:a + size], candidate[b:b + size])

    def test_small_changes_inside_moved_island_cost_credit(self):
        changed = bytearray(self.a)
        changed[100] ^= 0xff
        changed[200] ^= 0xff
        report = compare_payloads(self.a + self.b, self.b + changed)
        self.assertEqual(report['equal_paired_bytes'], len(self.a + self.b) - 2)
        self.assertTrue(any(not item['exact_bytes'] for item in report['islands']))
        self.assertLess(report['byte_similarity_percent'], 100)
        self.assert_partition(report)

    def test_insertions_and_deletions_remain_in_full_denominator(self):
        candidate = self.a[:120] + b'INSERTED' + self.a[120:]
        for left, right in ((self.a, candidate), (candidate, self.a)):
            report = compare_payloads(left, right)
            self.assertEqual(report['equal_paired_bytes'], len(self.a))
            self.assertAlmostEqual(report['byte_similarity_percent'],
                                   200 * len(self.a) / (len(self.a) + len(candidate)))
            self.assert_partition(report)

    def test_one_copy_cannot_cover_repeated_retail_copies(self):
        report = compare_payloads(self.a * 3, self.a)
        self.assertEqual(report['equal_paired_bytes'], len(self.a))
        self.assertEqual(report['byte_similarity_percent'], 50)
        self.assert_partition(report)

    def test_unrelated_content_is_not_hidden_by_zero_padding(self):
        report = compare_payloads(self.a + bytes(8192), self.b + bytes(8192))
        self.assertGreater(report['byte_similarity_percent'], 90)
        self.assertLess(report['nonzero_byte_similarity_percent'], 1)
        self.assert_partition(report)
        unrelated = compare_payloads(self.a + self.b, self.c * 2)
        self.assertLess(unrelated['byte_similarity_percent'], 1)
        self.assert_partition(unrelated)

    def test_changed_tail_followed_by_padding_retains_equal_suffix(self):
        report = compare_payloads(self.a + b'CPE\1\10\0\3' + bytes(1000),
                                  self.a + bytes(1007))
        self.assertEqual(report['equal_paired_bytes'], len(self.a) + 1001)
        self.assert_partition(report)

    def test_addresses_and_immediates_are_not_masked(self):
        left = self.a + struct.pack('<4I', 0x80012340, 0x0c004321, 0x3c028005, 0x24427e90) + self.b
        right = self.a + struct.pack('<4I', 0x80014340, 0x0c004421, 0x3c028006, 0x24422e90) + self.b
        report = compare_payloads(left, right)
        self.assertEqual(report['equal_paired_bytes'], len(left) - 4)
        self.assertLess(report['byte_similarity_percent'], 100)
        self.assert_partition(report)

    def test_exe_scope_hashes_addresses_and_malformed_lengths(self):
        left = RetailImage.synthetic('GAME.EXE', 0x80012000, self.a + self.b)
        right = RetailImage.synthetic('GAME.EXE', 0x80013000, self.b + self.a)
        report = compare_executables(left.data, right.data)
        self.assertEqual(report['byte_similarity_percent'], 100)
        self.assertEqual(report['retail_bytes'], len(self.a + self.b))
        self.assertNotEqual(report['retail_sha256'], report['candidate_sha256'])
        for island in report['islands']:
            self.assertEqual(island['retail_va'], 0x80012000 + island['retail_offset'])
            self.assertEqual(island['candidate_file_offset'], 0x800 + island['candidate_offset'])
        for bad in (b'', right.data[:-1], right.data + b'\0'):
            with self.subTest(size=len(bad)), self.assertRaises(ValueError):
                compare_executables(left.data, bad)

    def test_empty_identical_and_low_entropy_payloads(self):
        for payload in (b'', b'A', bytes(4096), self.a):
            report = compare_payloads(payload, payload)
            self.assertEqual(report['byte_similarity_percent'], 100)
            self.assertEqual(report['equal_paired_bytes'], len(payload))
            self.assert_partition(report)
        self.assertEqual(compare_payloads(b'', self.a)['byte_similarity_percent'], 0)
        with self.assertRaises(ValueError):
            compare_payloads(self.a, self.a, seed_size=0)


if __name__ == '__main__':
    unittest.main()
