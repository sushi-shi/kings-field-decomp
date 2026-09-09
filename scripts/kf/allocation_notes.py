"""Preserve native exported reservations in the flattened analysis container."""

from __future__ import annotations

import json
import re
import struct

from elftools.elf.elffile import ELFFile


SECTION = '.note.kf.allocations'
OWNER = b'KF\0'
NOTE_TYPE = 1


def compiler_requests(assembly: str) -> list[dict]:
    requests = []
    for line in assembly.splitlines():
        if not re.match(r'\s*\.comm\b', line):
            continue
        match = re.fullmatch(r'\s*\.comm\s+([A-Za-z_$][\w$]*),\s*(\d+)\s*', line)
        if match is None:
            raise ValueError(f'unsupported compiler COMMON directive: {line}')
        name, size = match.groups()
        requests.append({'name': name, 'reservation_size': int(size)})
    return requests


def assembly_note(requests: list[dict]) -> str:
    if not requests:
        return ''
    descriptor = json.dumps({'version': 1, 'requests': requests},
                            separators=(',', ':'), sort_keys=True).encode('ascii')
    payload = struct.pack('<III', len(OWNER), len(descriptor), NOTE_TYPE)
    payload += OWNER + bytes((-len(OWNER)) % 4)
    payload += descriptor + bytes((-len(descriptor)) % 4)
    lines = [f'\n\t.pushsection {SECTION},"",@note', '\t.balign 4']
    lines += ['\t.byte ' + ','.join(str(value) for value in payload[start:start + 24])
              for start in range(0, len(payload), 24)]
    return '\n'.join([*lines, '\t.popsection', ''])


def read_requests(elf: ELFFile) -> list[dict]:
    sections = [section for section in elf.iter_sections() if section.name == SECTION]
    if not sections:
        return []
    if len(sections) != 1 or sections[0]['sh_type'] != 'SHT_NOTE' or sections[0]['sh_flags']:
        raise ValueError('invalid allocation note section')
    data = sections[0].data()
    if len(data) < 12:
        raise ValueError('truncated allocation note')
    names, size, kind = struct.unpack_from('<III', data)
    start = 12 + ((names + 3) & ~3)
    end = start + size
    if (names != len(OWNER) or kind != NOTE_TYPE or data[12:12 + names] != OWNER
            or len(data) != start + ((size + 3) & ~3)
            or any(data[12 + names:start]) or any(data[end:])):
        raise ValueError('invalid allocation note framing')
    try:
        document = json.loads(data[start:end])
    except (ValueError, UnicodeError) as error:
        raise ValueError('invalid allocation note descriptor') from error
    if (not isinstance(document, dict) or set(document) != {'version', 'requests'}
            or type(document['version']) is not int or document['version'] != 1
            or not isinstance(document['requests'], list)
            or not document['requests']):
        raise ValueError('unsupported allocation note descriptor')
    seen = set()
    for request in document['requests']:
        if (not isinstance(request, dict) or set(request) != {'name', 'reservation_size'}
                or not isinstance(request['name'], str)
                or not re.fullmatch(r'[A-Za-z_$][\w$]*', request['name'])
                or request['name'] in seen
                or type(request['reservation_size']) is not int
                or not 0 <= request['reservation_size'] <= 0xffffffff):
            raise ValueError('invalid exported allocation request')
        seen.add(request['name'])
    return document['requests']
