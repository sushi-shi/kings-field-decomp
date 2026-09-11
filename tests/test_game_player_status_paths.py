"""Compare the complete slowed/poison/curse instruction region with retail."""
from __future__ import annotations

import os
import shutil
import struct
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.compile import compile_source
from scripts.kf.manifest import load
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO
from scripts.kf.relocations import decode_hi_lo_target


def status_region(path, region='status'):
    obj = _load_object(path)
    function = obj.named_symbol('player_update')
    words = list(struct.unpack_from(
        f'<{function.size // 4}I', obj.sections['.text'], function.value))
    normalized = list(words)
    pending = []
    boundaries = {}
    internal_jumps = []
    for reloc in obj.relocations:
        if reloc.section != '.text' or not function.value <= reloc.offset < function.value + function.size:
            continue
        index = (reloc.offset - function.value) // 4
        symbol = obj.symbol(reloc.symbol_index)
        if reloc.kind == 5:
            pending.append((index, symbol.index))
        elif reloc.kind == 6:
            assert len(pending) == 1
            high, symbol_index = pending.pop()
            assert symbol_index == symbol.index
            addend = decode_hi_lo_target(words[high], words[index])
            name = symbol.name
            if symbol.kind == 'STT_SECTION':
                offset = symbol.value + addend
                owners = [item for item in obj.symbols
                          if item.section == symbol.section and item.kind == 'STT_OBJECT'
                          and item.value <= offset < item.value + item.size]
                assert len(owners) == 1
                name, addend = owners[0].name, offset - owners[0].value
            for site in (high, index):
                normalized[site] = ('address', words[site] & 0xFFFF0000, name, addend)
            if symbol.name == 'player_state':
                start_field, start_opcode = (0x4E, 9) if region == 'status' else (0x4A, 33)
                end_field, end_opcode = (0x50, 33) if region == 'status' else (0xA2, 9)
                if addend == start_field and words[index] >> 26 == start_opcode:
                    assert 'start' not in boundaries
                    boundaries['start'] = high
                if addend == end_field and words[index] >> 26 == end_opcode:
                    assert 'end' not in boundaries
                    boundaries['end'] = high
        elif reloc.kind == 4:
            addend = (words[index] & 0x3FFFFFF) * 4
            if words[index] >> 26 == 3:
                normalized[index] = ('call', symbol.name, addend)
            else:
                assert symbol.section == '.text'
                target = symbol.value + addend - function.value
                internal_jumps.append((index, target))
        else:
            raise AssertionError(reloc)
    assert not pending
    start, end = boundaries['start'], boundaries['end']
    for index, target in internal_jumps:
        normalized[index] = ('jump', target - start * 4)
    return normalized[start:end]


def external_referents(path):
    """Return ordered text address identities and calls, independent of placement."""
    obj = _load_object(path)
    function = obj.named_symbol('player_update')
    pending = []
    calls = []
    addresses = []
    for reloc in obj.relocations:
        if reloc.section != '.text' or not function.value <= reloc.offset < function.value + function.size:
            continue
        symbol = obj.symbol(reloc.symbol_index)
        index = (reloc.offset - function.value) // 4
        words = struct.unpack_from('<I', obj.sections['.text'], reloc.offset)[0]
        if reloc.kind == 5:
            pending.append((index, symbol.index))
        elif reloc.kind == 6:
            assert len(pending) == 1
            high, symbol_index = pending.pop()
            assert symbol_index == symbol.index
            addend = decode_hi_lo_target(
                struct.unpack_from('<I', obj.sections['.text'], function.value + high * 4)[0],
                words,
            )
            name = symbol.name
            if symbol.kind == 'STT_SECTION':
                offset = symbol.value + addend
                owners = [item for item in obj.symbols
                          if item.section == symbol.section and item.kind == 'STT_OBJECT'
                          and item.value <= offset < item.value + item.size]
                assert len(owners) == 1
                name, addend = owners[0].name, offset - owners[0].value
            addresses.append((name, addend))
        elif reloc.kind == 4 and words >> 26 == 3:
            calls.append((symbol.name, (words & 0x3FFFFFF) * 4))
    assert not pending
    return calls, addresses


class PlayerStatusPathTests(unittest.TestCase):
    def test_compiled_status_paths_preserve_every_retail_instruction(self):
        if shutil.which('cc1psx-260') is None or not os.environ.get('PSYQ_INCLUDE'):
            self.skipTest('pinned compiler and SDK headers required')
        manifest = load()
        unit = manifest.by_name()['game.player_update']
        target = BUILD / 'delink/game/modules' / unit.object_name
        if not target.is_file():
            self.skipTest('freshly delinked GAME player target required')
        with TemporaryDirectory(prefix='kf-player-status-') as directory:
            output = Path(directory) / unit.object_name
            profile = manifest.profiles[unit.profile]
            compile_source(
                unit.source_path, unit.image, output, BUILD / 'delink', profile.optimization,
                profile.small_data, profile.aspsx_version,
                (REPO / 'include', REPO / 'vendor/include', Path(os.environ['PSYQ_INCLUDE'])),
                profile.cc1_flags, profile.compiler, defines=unit.defines,
            )
            actual, expected = status_region(output), status_region(target)
            # All three complete status operations, including poison callbacks,
            # branch displacements and delay slots. Only relocation identities
            # and jumps relative to this region replace linked address bits.
            self.assertEqual(len(expected), (0x1944 - 0x1794) // 4)
            self.assertEqual(actual, expected)

            self.assertEqual(len(status_region(target, 'darkness')), (0x1198 - 0x1090) // 4)
            self.assertEqual(status_region(output, 'darkness'), status_region(target, 'darkness'))
            actual_calls, actual_addresses = external_referents(output)
            expected_calls, expected_addresses = external_referents(target)
            self.assertEqual((len(expected_calls), len(expected_addresses)), (66, 210))
            self.assertEqual(actual_calls, expected_calls)
            self.assertEqual(actual_addresses, expected_addresses)
