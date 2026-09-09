"""Conservative, layout-tolerant byte islands for complete executable payloads.

This is a content heuristic, not symbol recovery, instruction equivalence or
objdiff's algorithm. No opcode, immediate, address or padding byte is masked.
"""

from __future__ import annotations

from dataclasses import dataclass
from difflib import SequenceMatcher
import hashlib
from html import escape
import struct


@dataclass
class _Run:
    retail: int
    candidate: int
    size: int


def _unique_windows(data: bytes, size: int) -> dict[bytes, int]:
    positions: dict[bytes, int] = {}
    for offset in range(len(data) - size + 1):
        window = data[offset:offset + size]
        positions[window] = -1 if window in positions else offset
    return positions


def _anchors(retail: bytes, candidate: bytes, seed_size: int) -> list[_Run]:
    left = _unique_windows(retail, seed_size)
    right = _unique_windows(candidate, seed_size)
    # Dict insertion order is source position order. Coalesce overlapping
    # windows only when their displacement agrees; no fuzzy credit yet.
    active: dict[int, _Run] = {}
    runs: list[_Run] = []
    for window, a in left.items():
        b = right.get(window, -1)
        if a < 0 or b < 0:
            continue
        delta = b - a
        previous = active.get(delta)
        if previous is not None and a <= previous.retail + previous.size:
            previous.size = a + seed_size - previous.retail
        else:
            run = _Run(a, b, seed_size)
            runs.append(run)
            active[delta] = run

    used_left, used_right = bytearray(len(retail)), bytearray(len(candidate))
    selected = []
    # Competing/overlapping anchors are resolved deterministically, longest
    # first. Rejecting a competitor is conservative; bytes are never reused.
    for run in sorted(runs, key=lambda item: (-item.size, item.retail, item.candidate)):
        a, b, size = run.retail, run.candidate, run.size
        if any(used_left[a:a + size]) or any(used_right[b:b + size]):
            continue
        used_left[a:a + size] = b'\1' * size
        used_right[b:b + size] = b'\1' * size
        selected.append(run)

    # An accepted unique anchor can extend through repeated bytes, but only
    # while both sides agree and neither side reaches another accepted run.
    for run in selected:
        a, b, size = run.retail, run.candidate, run.size
        while a and b and not used_left[a - 1] and not used_right[b - 1]:
            if retail[a - 1] != candidate[b - 1]:
                break
            a -= 1
            b -= 1
            size += 1
            used_left[a] = used_right[b] = 1
        while a + size < len(retail) and b + size < len(candidate):
            if used_left[a + size] or used_right[b + size]:
                break
            if retail[a + size] != candidate[b + size]:
                break
            used_left[a + size] = used_right[b + size] = 1
            size += 1
        run.retail, run.candidate, run.size = a, b, size
    # File boundaries also supply a positional correspondence. Preserve an
    # equal prefix/suffix even when repeated padding supplies no unique seed.
    # These runs still compete for the same one-use byte budget as all others.
    size = 0
    while size < min(len(retail), len(candidate)):
        if used_left[size] or used_right[size] or retail[size] != candidate[size]:
            break
        used_left[size] = used_right[size] = 1
        size += 1
    if size:
        selected.append(_Run(0, 0, size))
    size = 0
    while size < min(len(retail), len(candidate)):
        a, b = len(retail) - size - 1, len(candidate) - size - 1
        if used_left[a] or used_right[b] or retail[a] != candidate[b]:
            break
        used_left[a] = used_right[b] = 1
        size += 1
    if size:
        selected.append(_Run(len(retail) - size, len(candidate) - size, size))
    return sorted(selected, key=lambda item: item.retail)


def _ratio(matched: int, left: int, right: int) -> float:
    return 200.0 * matched / (left + right) if left + right else 100.0


def _unanchored(size: int, spans: list[tuple[int, int]]) -> list[dict]:
    result = []
    cursor = 0
    for start, length in sorted(spans):
        if start > cursor:
            result.append({'offset': cursor, 'size': start - cursor})
        cursor = start + length
    if cursor < size:
        result.append({'offset': cursor, 'size': size - cursor})
    return result


def compare_payloads(
    retail: bytes, candidate: bytes, *, seed_size: int = 16,
    max_gap: int = 128, min_bridge_similarity: float = 0.6,
) -> dict:
    """Find moved exact islands and bounded, anchor-supported fuzzy bridges.

    Scores count equal byte pairs only, over both complete inputs (Dice).
    Different lengths, insertions, deletions and unanchored content all cost
    credit. Similarity does not establish that an address has the same referent.
    """
    if seed_size < 4 or max_gap < 0 or not 0 <= min_bridge_similarity <= 1:
        raise ValueError('invalid executable island comparison parameters')
    runs = ([_Run(0, 0, len(retail))] if retail and retail == candidate
            else _anchors(retail, candidate, seed_size))
    candidate_order = sorted(runs, key=lambda item: item.candidate)
    candidate_next = {run.candidate: successor.candidate
                      for run, successor in zip(candidate_order, candidate_order[1:])}
    islands: list[dict] = []
    previous: _Run | None = None
    for run in runs:
        bridge = None
        if previous is not None:
            a_end = previous.retail + previous.size
            b_end = previous.candidate + previous.size
            a_gap, b_gap = run.retail - a_end, run.candidate - b_end
            if (candidate_next.get(previous.candidate) == run.candidate
                    and 0 <= a_gap <= max_gap and 0 <= b_gap <= max_gap):
                left, right = retail[a_end:run.retail], candidate[b_end:run.candidate]
                blocks = SequenceMatcher(None, left, right, autojunk=False).get_matching_blocks()
                matched = sum(block.size for block in blocks)
                similarity = _ratio(matched, a_gap, b_gap) / 100
                # Tiny edits can have zero gap similarity (even one changed
                # byte). Require strong surrounding exact context; edited
                # bytes still receive no credit merely for joining an island.
                context = _ratio(previous.size + run.size + matched,
                                 previous.size + run.size + a_gap,
                                 previous.size + run.size + b_gap) / 100
                short_edit = max(a_gap, b_gap) <= seed_size and context >= 0.85
                if similarity >= min_bridge_similarity or short_edit:
                    nonzero = sum(sum(value != 0 for value in left[block.a:block.a + block.size])
                                  for block in blocks)
                    bridge = matched, nonzero
        nonzero = sum(value != 0 for value in retail[run.retail:run.retail + run.size])
        if bridge is not None:
            island = islands[-1]
            island['retail_size'] = run.retail + run.size - island['retail_offset']
            island['candidate_size'] = run.candidate + run.size - island['candidate_offset']
            island['equal_bytes'] += run.size + bridge[0]
            island['equal_nonzero_bytes'] += nonzero + bridge[1]
            island['anchor_runs'] += 1
        else:
            islands.append({'retail_offset': run.retail, 'candidate_offset': run.candidate,
                            'retail_size': run.size, 'candidate_size': run.size,
                            'equal_bytes': run.size, 'equal_nonzero_bytes': nonzero,
                            'anchor_runs': 1})
        previous = run
    for island in islands:
        island['byte_similarity_percent'] = _ratio(
            island['equal_bytes'], island['retail_size'], island['candidate_size'])
        island['exact_bytes'] = (island['equal_bytes'] == island['retail_size']
                                 == island['candidate_size'])
    matched = sum(item['equal_bytes'] for item in islands)
    nonzero = sum(item['equal_nonzero_bytes'] for item in islands)
    left_nonzero = sum(value != 0 for value in retail)
    right_nonzero = sum(value != 0 for value in candidate)
    return {
        'method': 'unique-byte-islands-v1',
        'heuristic_only': True,
        'parameters': {'seed_bytes': seed_size, 'max_bridge_bytes_per_side': max_gap,
                       'min_bridge_similarity': min_bridge_similarity,
                       'min_short_edit_context_similarity': 0.85},
        'retail_bytes': len(retail), 'candidate_bytes': len(candidate),
        'retail_nonzero_bytes': left_nonzero, 'candidate_nonzero_bytes': right_nonzero,
        'equal_paired_bytes': matched, 'equal_paired_nonzero_bytes': nonzero,
        'byte_similarity_percent': _ratio(matched, len(retail), len(candidate)),
        'nonzero_byte_similarity_percent': _ratio(nonzero, left_nonzero, right_nonzero),
        'retail_bytes_without_equal_partner': len(retail) - matched,
        'candidate_bytes_without_equal_partner': len(candidate) - matched,
        'retail_island_coverage_percent': 100 * sum(i['retail_size'] for i in islands) / len(retail)
        if retail else 100.0,
        'candidate_island_coverage_percent': 100 * sum(i['candidate_size'] for i in islands) / len(candidate)
        if candidate else 100.0,
        'island_count': len(islands),
        'exact_island_count': sum(item['exact_bytes'] for item in islands),
        'islands': islands,
        'retail_unanchored': _unanchored(len(retail), [(i['retail_offset'], i['retail_size']) for i in islands]),
        'candidate_unanchored': _unanchored(len(candidate), [(i['candidate_offset'], i['candidate_size']) for i in islands]),
    }


def _payload(executable: bytes) -> tuple[int, bytes]:
    if len(executable) < 0x800 or executable[:8] != b'PS-X EXE':
        raise ValueError('expected a PS-X EXE with its complete header')
    base, size = struct.unpack_from('<II', executable, 0x18)
    if len(executable) != 0x800 + size:
        raise ValueError('EXE length differs from its declared load size')
    return base, executable[0x800:]


def compare_executables(retail: bytes, candidate: bytes) -> dict:
    """Compare load areas; keep headers separate and include sector padding."""
    retail_base, left = _payload(retail)
    candidate_base, right = _payload(candidate)
    report = compare_payloads(left, right)
    report.update(scope='complete load areas including sector padding; 2048-byte headers excluded',
                  retail_sha256=hashlib.sha256(retail).hexdigest(),
                  candidate_sha256=hashlib.sha256(candidate).hexdigest(),
                  retail_load_address=retail_base, candidate_load_address=candidate_base)
    for island in report['islands']:
        island.update(retail_va=retail_base + island['retail_offset'],
                      candidate_va=candidate_base + island['candidate_offset'],
                      retail_file_offset=0x800 + island['retail_offset'],
                      candidate_file_offset=0x800 + island['candidate_offset'])
    return report


def render_html(image: str, strict: dict, report: dict) -> str:
    """Standalone movement map and searchable island table; no external assets."""
    islands = report['islands']
    sizes = {'retail': max(1, report['retail_bytes']),
             'candidate': max(1, report['candidate_bytes'])}
    svg = []
    for island in sorted(islands, key=lambda item: -item['equal_bytes'])[:60]:
        a = 80 + 900 * island['retail_offset'] / sizes['retail']
        b = 80 + 900 * island['candidate_offset'] / sizes['candidate']
        svg.append(f'<line x1="{a:.2f}" y1="32" x2="{b:.2f}" y2="105" '
                   'stroke="#97b3cc" stroke-opacity=".3"/>')
    for side, y in (('retail', 20), ('candidate', 105)):
        svg.append(f'<text x="0" y="{y + 10}">{side.title()}</text>'
                   f'<rect x="80" y="{y}" width="900" height="12" fill="#d7dce1"/>')
        for island in islands:
            x = 80 + 900 * island[f'{side}_offset'] / sizes[side]
            width = 900 * island[f'{side}_size'] / sizes[side]
            color = '#2874a6' if island['exact_bytes'] else '#b57616'
            title = (f"Retail {island['retail_va']:08x} → candidate {island['candidate_va']:08x}; "
                     f"{island['retail_size']} / {island['candidate_size']} bytes; "
                     f"{island['byte_similarity_percent']:.2f}% equal")
            svg.append(f'<rect x="{x:.2f}" y="{y}" width="{width:.3f}" height="12" '
                       f'fill="{color}"><title>{title}</title></rect>')
    rows = []
    for island in sorted(islands, key=lambda item: -item['retail_size']):
        kind = 'exact bytes' if island['exact_bytes'] else 'approximate'
        rows.append(
            f'<tr data-size="{island["retail_size"]}" data-exact="{int(island["exact_bytes"])}">'
            f'<td><code>{island["retail_va"]:08x}</code></td>'
            f'<td><code>{island["candidate_va"]:08x}</code></td>'
            f'<td>{island["retail_size"]:,}</td><td>{island["candidate_size"]:,}</td>'
            f'<td>{island["equal_bytes"]:,}</td>'
            f'<td>{island["byte_similarity_percent"]:.2f}%</td><td>{kind}</td></tr>')
    gaps = []
    for side in ('retail', 'candidate'):
        for gap in sorted(report[f'{side}_unanchored'], key=lambda item: -item['size'])[:20]:
            va = report[f'{side}_load_address'] + gap['offset']
            gaps.append(f'<tr><td>{side.title()}</td><td><code>{va:08x}</code></td>'
                        f'<td>{gap["size"]:,}</td></tr>')
    return f'''<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>{escape(image)} executable islands</title>
<style>
body {{ max-width:1100px; margin:36px auto; padding:0 20px; font:16px/1.5 system-ui; color:#243447; }}
h1 {{ font-size:28px; }} .metrics {{ display:flex; flex-wrap:wrap; gap:28px; margin:24px 0; }}
.metrics strong {{ display:block; font-size:30px; }}
svg {{ width:100%; height:auto; font:12px system-ui; }}
table {{ border-collapse:collapse; width:100%; margin:16px 0; font-variant-numeric:tabular-nums; }}
th,td {{ padding:7px 12px; border-bottom:1px solid #e1e5ea; text-align:right; }}
th:first-child,td:first-child {{ text-align:left; }} th {{ background:#edf2f6; }}
label {{ display:inline-block; margin-right:18px; }} input,select {{ font:inherit; }}
.scroll {{ max-height:620px; overflow:auto; }} .scroll th {{ position:sticky; top:0; }}
code {{ font-size:14px; }} small {{ color:#506070; }}
</style>
<h1>{escape(image)}: executable byte islands</h1>
<p>Raw content aligned across moved regions. This heuristic does not establish function identity,
correct address referents, runtime equivalence or an exact executable match.</p>
<div class="metrics">
<div><strong>{report['byte_similarity_percent']:.2f}%</strong>island-aligned byte similarity</div>
<div><strong>{report['nonzero_byte_similarity_percent']:.2f}%</strong>nonzero-byte similarity</div>
<div><strong>{len(islands):,}</strong>paired islands</div></div>
<p>Strict whole-file comparison: <b>{strict['differing_bytes']:,} differing positions</b>;
retail {strict['retail_size']:,} bytes, candidate {strict['linked_size']:,} bytes.</p>
<p>The island scores cover complete load areas, including sector padding, and exclude both
2048-byte headers. Score = 200 × equal paired bytes ÷ total bytes in both load areas.
Each byte is used at most once; unmatched bytes remain in the denominator. The nonzero
score applies the same formula to nonzero bytes. No addresses or instructions are masked.</p>
<h2>Movement map</h2>
<svg viewBox="0 0 1000 140" role="img" aria-label="Retail and candidate island positions">
{''.join(svg)}</svg>
<small>Blue: exact byte runs. Amber: islands containing differences. Gray: unanchored content.
Lines connect the 60 largest islands; hover over either strip for addresses and sizes.</small>
<h2>Paired islands</h2>
<label>Minimum retail bytes <input id="minimum" type="number" value="64" min="0" step="16"></label>
<label>Show <select id="kind"><option value="all">All islands</option>
<option value="0">Approximate</option><option value="1">Exact bytes</option></select></label>
<label>Address <input id="address" placeholder="8001..."></label>
<span id="count"></span>
<div class="scroll"><table id="islands"><thead><tr><th>Retail VA</th><th>Candidate VA</th>
<th>Retail bytes</th><th>Candidate bytes</th><th>Equal pairs</th><th>Similarity</th><th>Kind</th>
</tr></thead><tbody>{''.join(rows)}</tbody></table></div>
<h2>Largest unanchored regions</h2>
<p>These did not receive an island correspondence. They can contain repeated content,
changed references, different code or absent data; this is not a diagnosis of missing code.</p>
<table><thead><tr><th>Image</th><th>Start VA</th><th>Bytes</th></tr></thead>
<tbody>{''.join(gaps)}</tbody></table>
<p><small>Method: {report['method']}; unique 16-byte seeds, at most 128 bytes per bridge side,
60% bridge byte similarity, or edits of at most 16 bytes per side with at least
85% similarity across the edit and its two neighboring exact runs.
Repeated bytes can extend accepted runs; equal file-boundary prefixes/suffixes are retained.
All addresses, spans and scores are also recorded in the accompanying JSON.</small></p>
<script>
const rows = [...document.querySelectorAll('#islands tbody tr')];
const minimum = document.getElementById('minimum'), kind = document.getElementById('kind');
const address = document.getElementById('address');
function filter() {{
  let count = 0;
  for (const row of rows) {{
    const visible = Number(row.dataset.size) >= Number(minimum.value)
      && (kind.value === 'all' || row.dataset.exact === kind.value)
      && (row.cells[0].textContent + ' ' + row.cells[1].textContent)
         .includes(address.value.toLowerCase().replace(/^0x/, ''));
    row.hidden = !visible; count += visible;
  }}
  document.getElementById('count').textContent = count + ' / ' + rows.length + ' islands';
}}
for (const input of [minimum, kind, address]) input.addEventListener('input', filter);
filter();
</script></html>
'''
