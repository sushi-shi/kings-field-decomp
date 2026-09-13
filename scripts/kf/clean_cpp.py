"""Select the modern language view and supply real typed call boundaries."""

import re
import json

from scripts.kf.clean_lexer import resolve_conditionals, rewrite_calls, strip_comments, tidy, tokens


def replace(text, old, new, count=1):
    actual = text.count(old)
    if actual != count:
        raise ValueError(f'C++ export expected {count} occurrences of {old!r}, found {actual}')
    return text.replace(old, new)


def enum_support(source):
    text = strip_comments(source)
    text = text.split('#define KF_MODERN_TYPES 1', 1)[1].split('\n#else', 1)[0]
    macros = {}
    lines = iter(text.splitlines())
    kept = []
    for line in lines:
        if line.startswith('#define '):
            while line.endswith('\\'):
                line = line[:-1] + '\n' + next(lines)
            match = re.fullmatch(r'#define (\w+)\(([^)]*)\)\s*(.*)', line, re.S)
            if not match:
                raise ValueError(f'unsupported enum macro: {line}')
            name, parameters, body = match.groups()
            parameters = [p.strip() for p in parameters.split(',')]

            def expand(arguments, parameters=parameters, body=body):
                mapping = dict(zip(parameters, arguments))
                return re.sub(r'\b\w+\b', lambda m: mapping.get(m[0], m[0]), body)

            macros[name] = (len(parameters), expand)
        else:
            kept.append(line)
    return macros, '#pragma once\n' + tidy('\n'.join(kept))


def clean_cpp(text, rules):
    from scripts.kf.clean import CLAIMS, EFFECT_ARGUMENTS, IMPLEMENTATION

    text = resolve_conditionals(strip_comments(text), modern=True)
    text = re.sub(r'^\s*#\s*include\s*[<"]kf/address\.h[>"]\s*$', '', text, flags=re.M)
    for name in IMPLEMENTATION:
        text = re.sub(rf'^\s*#\s*define\s+{name}\s*$', '', text, flags=re.M)
    lines = iter(text.splitlines(keepends=True))
    kept = []
    for line in lines:
        if re.match(r'^\s*#\s*define\s+KF_EFFECT_ARGS_', line):
            while line.rstrip().endswith('\\'):
                line = next(lines)
        else:
            kept.append(line)
    rules = dict(rules)
    rules.update({name: (arity, lambda a: '') for name, arity in CLAIMS.items()})
    for name, arity in EFFECT_ARGUMENTS.items():
        typename = 'KfEffect' + ''.join(part.title() for part in name.split('_')) + 'Arguments'
        rules['KF_EFFECT_ARGS_' + name] = (arity, lambda a, t=typename: t + '{' + ', '.join(a) + '}')
    result = rewrite_calls(''.join(kept), rules)
    residue = [word for kind, word in tokens(result) if kind == 'word' and (
        word in CLAIMS or word in IMPLEMENTATION or word == 'KF_MODERN_TYPES'
        or word.startswith(('KF_ENUM_', 'KF_EFFECT_ARGS_')))]
    if residue:
        raise ValueError(f'unremoved C++ scaffolding: {sorted(set(residue))}')
    return tidy(result)


def effect_constructor(text):
    # Typed overloads replace the original compiler's implicit argument home slots.
    declaration = ('KfEffectRecord *effect_pool_construct(\n'
                   '    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,\n'
                   '    const SVECTOR *direction, ...)')
    parameters = ('u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,\n'
                  '    const SVECTOR *direction')
    common = '''struct EffectArguments {
    const SVECTOR *rotation = nullptr;
    KfEffectSoundRequest sound = KF_EFFECT_SOUND_SILENT;
    KfEffectGroundBranchRole role{};
    KfEffectHomingMode target{};
    s32 duration = 0;
    s32 generations = 0;
    s32 scale = 0;
    s32 parent_index = 0;
};

static KfEffectRecord *effect_pool_construct_impl(''' + parameters + ', const EffectArguments& arguments)'
    text = replace(text, declaration, common)
    text = replace(text, '    s32 *va = (s32 *)&direction;', '')
    text = replace(text, 'KF_ENUM_DECODE(KfEffectGroundBranchRole, *(u16 *)(va + 1))',
                   'arguments.role')
    text = replace(text, 'record->propagation.generations_remaining = *(u16 *)(va + 1)',
                   'record->propagation.generations_remaining = arguments.generations')
    text = replace(text, '*(u16 *)(va + 1)', 'arguments.duration', 2)
    text = replace(text, '*(u16 *)(va + 2)', 'arguments.duration')
    text = replace(text, '*(u16 *)(va + 3)', 'arguments.scale')
    text = replace(text, '*(u8 *)(va + 1)', 'arguments.parent_index')
    text = replace(text, 'KF_ENUM_DECODE(KfEffectHomingMode, *(u8 *)(va + 2))', 'arguments.target', 2)
    text = replace(text, '*(const SVECTOR *)va[1]', '*arguments.rotation', 9)
    text = re.sub(r'KF_ENUM_DECODE\(KfEffectSoundRequest, va\[[123]\]\)', 'arguments.sound', text)
    if re.search(r'\bva\b', text):
        raise ValueError('unconverted effect argument home-slot access')
    call = 'effect_pool_construct_impl(id, type, kind, position, direction, arguments)'
    text += '\nKfEffectRecord *effect_pool_construct(' + parameters + ')\n{\n' \
        '    EffectArguments arguments;\n    return ' + call + ';\n}\n'
    fields = {'Branch': ('role',), 'Rotation': ('rotation',),
              'RotationSound': ('rotation', 'sound'), 'DurationSound': ('duration', 'sound'),
              'Scatter': ('generations', 'duration', 'scale'), 'Sound': ('sound',),
              'Homing': ('rotation', 'target', 'sound'), 'Parent': ('parent_index',)}
    for name, members in fields.items():
        text += '\nKfEffectRecord *effect_pool_construct(' + parameters + \
            f', KfEffect{name}Arguments value)\n{{\n    EffectArguments arguments;\n'
        text += ''.join(f'    arguments.{member} = value.{member};\n' for member in members)
        text += '    return ' + call + ';\n}\n'
    return text


def prepare(name, text):
    """Explicit, counted compatibility changes, confined to the exported C++ tree."""
    text = strip_comments(text)
    if name == 'src/game/effect_pool.c':
        text = effect_constructor(text)
    elif name == 'src/game/menu_enter_mode.c':
        text = replace(text, 'u32 menu_enter_mode(KfMenuMode menu_mode, ...)',
                       'static u32 menu_enter_mode_impl(KfMenuMode menu_mode, int argument)')
        text = replace(text, '        va_list arguments;\n', '', 2)
        text = replace(text, '        va_start(arguments, menu_mode);\n', '', 2)
        text = replace(text, 'va_arg(arguments, int)', 'argument', 2)
        text = replace(text, '        va_end(arguments);\n', '', 2)
        text += '''
u32 menu_enter_mode(KfMenuMode mode) { return menu_enter_mode_impl(mode, 0); }
u32 menu_enter_mode(KfMenuMode mode, KfObjectId id)
{ return menu_enter_mode_impl(mode, static_cast<int>(id)); }
u32 menu_enter_mode(KfMenuMode mode, KfItemStockBank bank)
{ return menu_enter_mode_impl(mode, static_cast<int>(bank)); }
'''
    elif name == 'src/game/menu_runtime.c':
        text = replace(text, 'KfMenuResult menu_list_interact(\n',
                       'static KfMenuResult menu_list_interact_impl(\n')
        for domain in ('KfObjectId', 'KfEffectKind'):
            text += f'''
KfMenuResult menu_list_interact(
    const KfMenuList *list, KfMenuConfirmKind confirmation, KfMenuPreviewMode preview,
    {domain} id, KfItemStockBank bank, KfTradeMode trade)
{{
    return menu_list_interact_impl(list, confirmation, preview, static_cast<s32>(id), bank, trade);
}}
'''
    if name in ('src/psx/main.c', 'src/game/main.c', 'src/open/main.c'):
        text = re.sub(r'\b(void|int) main\(', r'extern "C" \1 main(', text)
    return text


def modernize(files, output):
    rules, support = enum_support(files['include/kf/enum.h'].decode())
    for name in list(output):
        if name.endswith(('.c', '.h', '.inc')):
            text = clean_cpp(prepare(name, files[name].decode()), rules)
            if name == 'vendor/include/psyq/sdk.h':
                text = text.replace('#include <LIBGPU.H>',
                    '\n'.join(f'#define {n} {n}_unprototyped' for n in
                              ('AddPrim', 'DrawOTag', 'SetSemiTrans')) +
                    '\n#include <LIBGPU.H>\n' +
                    '\n'.join(f'#undef {n}' for n in ('AddPrim', 'DrawOTag', 'SetSemiTrans')))
            if name == 'vendor/include/psyq/libc.h':
                names = ('memcpy', 'memset', 'malloc', 'free')
                text = text.replace('#include <MEMORY.H>',
                    '\n'.join(f'#define {n} {n}_unprototyped' for n in names) +
                    '\n#include <MEMORY.H>')
                text = text.replace('#include <MALLOC.H>', '#include <MALLOC.H>\n' +
                    '\n'.join(f'#undef {n}' for n in names))
            if name == 'vendor/include/psyq/pad.h':
                text = replace(text, 'extern u32 PadRead();\n', '')
                text = replace(text, 'PadRead(s32 ignored_identifier)',
                               'PadRead(s32 ignored_identifier = 0)')
            if name.startswith('vendor/include/psyq/'):
                text = 'extern "C" {\n' + text + '\n}\n'
            output[name] = text.encode()
            if name.endswith('.c'):
                output[name[:-2] + '.cpp'] = output.pop(name)
    output['include/kf/enum.h'] = support.encode()
    manifest = json.loads(output['build.json'])
    for image in manifest['images']:
        for unit in image['units']:
            unit['source'] = unit['source'][:-2] + '.cpp'
            unit['options'] = {'defines': unit['options']['defines']}
    output['build.json'] = (json.dumps(manifest, indent=2) + '\n').encode()
    prefix = 'scripts/kf/clean_cpp_project/'
    for name, data in files.items():
        if name.startswith(prefix):
            output[name.removeprefix(prefix)] = data
    return output
