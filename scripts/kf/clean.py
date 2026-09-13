"""Generate and publish the standalone C and C++ source projects."""

from __future__ import annotations

import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import tempfile
import tokenize
import tomllib

from scripts.kf.clean_lexer import resolve_conditionals, rewrite_calls, strip_comments, tidy, tokens
from scripts.kf.paths import REPO


MARKER = '.kf-clean-generated'
PROVENANCE = 'Generated-By: kf clean'
TEMPLATE = 'scripts/kf/clean_project/'
CLAIMS = {'ADDRESS': 2, 'ADDRESS_AT': 3, 'DATA': 2, 'RODATA': 2}
TYPE_MACROS = {
    'KF_ENUM_BEGIN', 'KF_ENUM_END', 'KF_ENUM_PROMOTED', 'KF_ENUM_STORAGE',
    'KF_ENUM_PARAM', 'KF_ENUM_COUNTER', 'KF_ENUM_FLAGS', 'KF_ENUM_DECODE', 'KF_ENUM_ENCODE',
}
IMPLEMENTATION = {'KF_MENU_MODE_IMPLEMENTATION', 'KF_MENU_LIST_IMPLEMENTATION',
                  'KF_EFFECT_POOL_IMPLEMENTATION'}
EFFECT_ARGUMENTS = {'BRANCH': 1, 'ROTATION': 1, 'ROTATION_SOUND': 2, 'DURATION_SOUND': 2,
                    'SCATTER': 3, 'SOUND': 1, 'HOMING': 3, 'PARENT': 1}
ORIGINS = {'PSX.EXE': 0x80010000, 'GAME.EXE': 0x80012000, 'OPEN.EXE': 0x80012000}


def git(repo: Path, *arguments: str) -> str:
    return subprocess.check_output(['git', '-C', str(repo), *arguments], text=True).strip()


def snapshot(repo: Path, revision: str, *, working: bool = False) -> tuple[str, dict[str, bytes]]:
    commit = git(repo, 'rev-parse', '--verify', f'{revision}^{{commit}}')
    if working:
        names = git(repo, 'ls-files', '-z').split('\0')
        files = {}
        for name in names:
            if name:
                path = repo / name
                if path.is_symlink():
                    continue
                files[name] = path.read_bytes()
        return commit, files
    archive = subprocess.check_output(['git', '-C', str(repo), 'archive', commit])
    with tarfile.open(fileobj=io.BytesIO(archive)) as stream:
        files = {}
        for member in stream:
            if member.isfile():
                files[member.name] = stream.extractfile(member).read()
    return commit, files


def clean_c(text: str, promoted: dict[str, str]) -> str:
    text = resolve_conditionals(strip_comments(text))
    text = re.sub(r'^\s*#\s*include\s*[<"]kf/(?:address|enum)\.h[>"]\s*$', '', text, flags=re.M)
    for name in IMPLEMENTATION:
        text = re.sub(rf'^\s*#\s*define\s+{name}\s*$', '', text, flags=re.M)
    lines = iter(text.splitlines(keepends=True))
    retained = []
    for line in lines:
        if re.match(r'^\s*#\s*define\s+KF_EFFECT_ARGS_', line):
            while line.rstrip().endswith('\\'):
                line = next(lines)
        else:
            retained.append(line)
    text = ''.join(retained)
    rules = {name: (arity, lambda args: '') for name, arity in CLAIMS.items()}
    rules.update({f'KF_EFFECT_ARGS_{name}': (arity, lambda a: ', '.join(f'({value})' for value in a))
                  for name, arity in EFFECT_ARGUMENTS.items()})
    rules.update({
        'KF_ENUM_BEGIN': (2, lambda a: f'typedef {a[1]} {a[0]}; enum {{'),
        'KF_ENUM_END': (1, lambda a: '};'),
        'KF_ENUM_PROMOTED': (1, lambda a: promoted[a[0]]),
        'KF_ENUM_STORAGE': (2, lambda a: a[1]),
        'KF_ENUM_PARAM': (2, lambda a: a[1]),
        'KF_ENUM_COUNTER': (2, lambda a: ''),
        'KF_ENUM_FLAGS': (2, lambda a: ''),
        'KF_ENUM_DECODE': (2, lambda a: f'(({a[0]})({a[1]}))'),
        'KF_ENUM_ENCODE': (2, lambda a: f'(({a[0]})({a[1]}))'),
    })
    text = rewrite_calls(text, rules)
    residue = [word for kind, word in tokens(text) if kind == 'word' and (
        word in CLAIMS or word.startswith(('KF_ENUM_', 'KF_EFFECT_ARGS_')) or word == 'KF_MODERN_TYPES'
        or word in IMPLEMENTATION)]
    if residue:
        raise ValueError(f'unremoved scaffolding: {sorted(set(residue))}')
    return tidy(text)


def without_python_comments(text: str) -> str:
    stream = tokenize.generate_tokens(io.StringIO(text).readline)
    return tokenize.untokenize(token for token in stream if token.type != tokenize.COMMENT)


def generate(files: dict[str, bytes], *, modern: bool = False) -> dict[str, bytes]:
    manifest = tomllib.loads(files['config/units.toml'].decode())
    units = [unit for unit in manifest['unit'] if unit.get('scope') != 'vendored']
    source_names = {unit['source'] for unit in units}
    source_names.update(name for name in files if name.endswith('.h') and (
        name.startswith('include/') or name.startswith('vendor/include/')))
    source_names -= {'include/kf/address.h', 'include/kf/enum.h'}
    promoted = {}
    promotions = {'s8': 'int', 'u8': 'int', 's16': 'int', 'u16': 'int',
                  's32': 's32', 'u32': 'u32', 'int': 'int'}
    for name in sorted(source_names):
        for domain, storage in re.findall(r'KF_ENUM_BEGIN\(\s*(\w+)\s*,\s*(\w+)\s*\)',
                                          files[name].decode()):
            if storage not in promotions:
                raise ValueError(f'{domain}: unknown C integer promotion for {storage}')
            promoted[domain] = promotions[storage]
    output = {}
    for name in sorted(source_names):
        try:
            output[name] = clean_c(files[name].decode(), promoted).encode()
        except (ValueError, KeyError) as error:
            raise ValueError(f'{name}: {error}') from error
    for name, data in sorted(files.items()):
        if name.startswith(TEMPLATE):
            output[name.removeprefix(TEMPLATE)] = data
        elif name.startswith('scripts/psxbuild/') and name.endswith('.py'):
            output[name] = data
        elif name.startswith('tools/kf-codec/src/') and name.endswith('.rs') and '/bin/' not in name:
            text = strip_comments(data.decode(), rust=True)
            if re.search(r'#\s*\[\s*(?:test|cfg\s*\(\s*test\s*\))\s*\]', text):
                raise ValueError(f'{name}: unexpected inline tests in library source')
            output[name.replace('tools/kf-codec/', 'codecs/')] = tidy(text, rust=True).encode()
    output['scripts/__init__.py'] = b''
    output['LICENSE'] = files['LICENSE']
    for name in ('nix/psx-toolchain.nix', 'nix/pcsx-redux.nix'):
        output[name] = re.sub(rb'(?m)^\s*#[^\n]*\n', b'', files[name])
    output['scripts/create-toolchain.py'] = without_python_comments(
        files['scripts/create-toolchain.py'].decode()).encode()
    output['link/overlay_bounds.asm'] = re.sub(rb'(?m)^;[^\n]*\n', b'',
                                             files['config/link/overlay_bounds.asm'])
    output['codecs/Cargo.toml'] = b'''[package]
name = "kf-codec"
version = "0.1.0"
edition = "2021"
publish = false
autobins = false
autotests = false
autobenches = false
autoexamples = false

[lib]
path = "src/lib.rs"
test = false
doctest = false

[profile.release]
overflow-checks = true
'''
    output['codecs/Cargo.lock'] = files['tools/Cargo.lock']
    images = []
    for image, origin in ORIGINS.items():
        rows = []
        for unit in units:
            if unit['image'] != image:
                continue
            profile = manifest['profiles'][unit['profile']]
            if profile['language'] != 'c' or profile['aspsx_version'] != '1.07':
                raise ValueError(f'{unit["unit"]}: unsupported standalone compiler profile')
            options = {key: profile[key] for key in ('compiler', 'optimization', 'small_data', 'cc1_flags')}
            options['defines'] = unit.get('defines', [])
            rows.append({'source': unit['source'], 'options': options})
        if not rows:
            raise ValueError(f'{image}: no source units')
        images.append({'name': image, 'load_address': origin, 'units': rows})
    output['build.json'] = (json.dumps({'images': images}, indent=2) + '\n').encode()
    lock = json.loads(files['flake.lock'])
    nixpkgs = lock['nodes'][lock['nodes']['root']['inputs']['nixpkgs']]
    output['flake.lock'] = (json.dumps({'nodes': {'nixpkgs': nixpkgs,
        'root': {'inputs': {'nixpkgs': 'nixpkgs'}}}, 'root': 'root', 'version': lock['version']},
        indent=2) + '\n').encode()
    if modern:
        from scripts.kf.clean_cpp import modernize
        output = modernize(files, output)
    else:
        output = {name: data for name, data in output.items()
                  if not name.startswith('codecs/') and name not in (
                      'scripts/psxbuild/clang.py', 'scripts/psxbuild/elf_to_lnk.py')}
        output['flake.nix'] = output['flake.nix'].replace(
            b'        codecs = pkgs.mkShell { packages = [ pkgs.cargo pkgs.rustc ]; };\n', b'')
    for name in output:
        if any(part in ('tests', '__pycache__', '.git') for part in Path(name).parts):
            raise ValueError(f'forbidden generated path: {name}')
    if re.search(rb'\bchecks\b|\bpytest\b|\bcargo test\b', output['flake.nix']):
        raise ValueError('generated flake includes tests')
    return output


def validate_output(repo: Path, requested: Path) -> Path:
    path = requested.absolute()
    if path.is_symlink() or any(parent.is_symlink() for parent in path.parents):
        raise ValueError('output must not traverse symlinks')
    path = path.resolve()
    repo = repo.resolve()
    if path == repo or path in repo.parents:
        raise ValueError('output must not contain the repository')
    if path.is_relative_to(repo) and not path.is_relative_to(repo / 'build'):
        raise ValueError('output inside the repository must be under build/')
    if path == repo / 'build':
        raise ValueError('output must be a child of build/')
    if path.exists() and (not (path / MARKER).is_file() or any(path.rglob('.git'))):
        raise ValueError('existing output must be a marked generated directory, not a worktree')
    return path


def write_output(repo: Path, requested: Path, files: dict[str, bytes], commit: str, working: bool) -> Path:
    output = validate_output(repo, requested)
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.kf-clean-', dir=output.parent) as directory:
        staging = Path(directory) / 'project'
        staging.mkdir()
        for name, data in sorted(files.items()):
            path = staging / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        (staging / MARKER).write_text(json.dumps({'commit': commit, 'working': working}) + '\n')
        if output.exists():
            shutil.rmtree(output)
        staging.rename(output)
    return output


def compare_program(original: bytes, cleaned: bytes, original_cpe: bytes, cleaned_cpe: bytes) -> list[int]:
    """Require identical native links and EXEs apart from two reserved header words."""
    if not original_cpe.startswith(b'CPE\x01') or original_cpe != cleaned_cpe:
        raise ValueError('native linker output differs')
    if len(original) < 2048 or original[:8] != b'PS-X EXE':
        raise ValueError('invalid reference executable')
    if original[:8] + original[16:] != cleaned[:8] + cleaned[16:]:
        raise ValueError('executable differs outside reserved header words')
    return [offset for offset in range(8, 16) if original[offset] != cleaned[offset]]


def verify(output: Path, repo: Path, *, compare: bool = True) -> None:
    subprocess.run(['nix', 'build', f'path:{output}', '--out-link', str(output / 'result')], check=True)
    if compare:
        subprocess.run(['kf', 'build'], cwd=repo, check=True)
        for name in ORIGINS:
            original = repo / 'build/link' / name[:-4].lower() / name
            try:
                reserved = compare_program(
                    original.read_bytes(), (output / 'result' / name).read_bytes(),
                    original.with_suffix('.CPE').read_bytes(),
                    (output / 'result/link' / name.replace('.EXE', '.CPE')).read_bytes())
            except ValueError as error:
                raise ValueError(f'{name}: {error}') from error
            if reserved:
                print(f'{name}: native link and executable contents agree; whole-file equality: false; '
                      f'CPE2X reserved header differences: {[hex(offset) for offset in reserved]}',
                      flush=True)
            else:
                print(f'{name}: native link and complete executable are byte-identical', flush=True)
    if (output / 'codecs/Cargo.toml').exists():
        subprocess.run(['nix', 'develop', f'path:{output}#codecs', '-c', 'cargo', 'build', '--offline',
                        '--manifest-path', str(output / 'codecs/Cargo.toml')], check=True)


def publish(repo: Path, files: dict[str, bytes], commit: str, branch: str, requested: Path,
            *, reset_history: bool = False) -> Path:
    git(repo, 'check-ref-format', '--branch', branch)
    ref = f'refs/heads/{branch}'
    current = git(repo, 'branch', '--show-current')
    if branch == current:
        raise ValueError('cannot publish over the current branch')
    old = subprocess.run(['git', '-C', str(repo), 'rev-parse', '--verify', ref],
                         capture_output=True, text=True)
    tip = old.stdout.strip() if old.returncode == 0 else None
    if tip and PROVENANCE not in git(repo, 'log', '-1', '--format=%B', tip).splitlines():
        raise ValueError(f'{branch}: branch tip was not generated by kf clean')
    worktree = requested.absolute()
    if worktree.is_symlink() or any(parent.is_symlink() for parent in worktree.parents):
        raise ValueError('worktree must not traverse symlinks')
    worktree = worktree.resolve()
    if worktree == repo.resolve() or worktree in repo.resolve().parents:
        raise ValueError('worktree must not contain the reconstruction checkout')
    for entry in git(repo, 'worktree', 'list', '--porcelain').split('\n\n'):
        lines = entry.splitlines()
        if f'branch {ref}' in lines:
            existing = Path(lines[0].removeprefix('worktree ')).resolve()
            if existing != worktree:
                raise ValueError(f'{branch}: already checked out at {existing}')
    if worktree.exists():
        if not (worktree / '.git').is_file() or git(worktree, 'symbolic-ref', 'HEAD') != ref:
            raise ValueError(f'{worktree}: not the generated branch worktree')
        if git(worktree, 'status', '--porcelain', '--untracked-files=all'):
            raise ValueError(f'{worktree}: generated worktree has local changes')
    else:
        worktree.parent.mkdir(parents=True, exist_ok=True)
        args = ('worktree', 'add', str(worktree), branch) if tip else (
            'worktree', 'add', '-b', branch, str(worktree), commit)
        git(repo, *args)
    tracked = set(git(worktree, 'ls-files', '-z').split('\0')) - {''}
    for name in files:
        target = worktree / name
        if target.exists() and name not in tracked:
            raise ValueError(f'{target}: would overwrite an untracked or ignored file')
    # Verify the full tracked contents before treating a previous export as a no-op.
    same = tracked == set(files) and all((worktree / name).read_bytes() == data
                                        for name, data in files.items())
    if not reset_history and same and f'Source-Commit: {commit}' in git(
            worktree, 'log', '-1', '--format=%B').splitlines():
        return worktree
    for name in sorted(tracked - set(files)):
        git(worktree, 'rm', '--', name)
    for name, data in sorted(files.items()):
        target = worktree / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    git(worktree, 'add', '--all', '--', *sorted(files))
    message = f'{branch}: regenerate from {commit[:12]}\n\n{PROVENANCE}\nSource-Commit: {commit}\n'
    tree = git(worktree, 'write-tree')
    parents = ('-p', tip) if tip and not reset_history else ()
    new_tip = git(repo, 'commit-tree', tree, *parents, '-m', message)
    git(repo, 'update-ref', ref, new_tip, tip or commit)
    return worktree


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=Path('build/clean-source'))
    parser.add_argument('--ref', default='HEAD')
    parser.add_argument('--working-tree', action='store_true', help='preview tracked working files')
    parser.add_argument('--classic', action='store_true', help='export plain C without codecs')
    parser.add_argument('--verify', action='store_true')
    parser.add_argument('--publish', metavar='BRANCH')
    parser.add_argument('--reset-history', action='store_true',
                        help='replace a generated branch with a single root commit')
    parser.add_argument('--worktree', type=Path, default=Path('build/source'))
    args = parser.parse_args(argv)
    try:
        if args.reset_history and not args.publish:
            raise ValueError('--reset-history requires --publish')
        if args.publish and args.working_tree:
            raise ValueError('publication requires a committed revision, not --working-tree')
        if args.publish:
            output_path, worktree_path = args.out.resolve(), args.worktree.resolve()
            if output_path.is_relative_to(worktree_path) or worktree_path.is_relative_to(output_path):
                raise ValueError('output and worktree directories must not overlap')
        if args.verify and args.ref != 'HEAD':
            raise ValueError('--verify compares with current HEAD; check out the requested revision first')
        commit, inputs = snapshot(REPO, args.ref, working=args.working_tree)
        files = generate(inputs, modern=not args.classic)
        output = write_output(REPO, args.out, files, commit, args.working_tree)
        fingerprint = hashlib.sha256(b''.join(name.encode() + b'\0' + data
                                             for name, data in sorted(files.items()))).hexdigest()
        print(f'Generated {len(files)} files at {output}; SHA-256 {fingerprint}', flush=True)
        if args.verify:
            verify(output, REPO, compare=args.classic)
        if args.publish:
            worktree = publish(REPO, files, commit, args.publish, args.worktree,
                               reset_history=args.reset_history)
            print(f'{args.publish}: {worktree}', flush=True)
        return 0
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    raise SystemExit(main())
