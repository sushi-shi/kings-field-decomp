"""Small lexical transforms for source export, without parsing either language."""

from __future__ import annotations

import re


def tokens(text: str, *, rust: bool = False):
    """Yield (kind, spelling), preserving literals, Rust lifetimes and token boundaries."""
    i = 0
    while i < len(text):
        start = i
        if text.startswith('//', i):
            i = text.find('\n', i)
            if i < 0:
                i = len(text)
            while not rust and i < len(text) and text[i - 1] == '\\':
                following = text.find('\n', i + 1)
                i = len(text) if following < 0 else following
            yield 'comment', text[start:i]
        elif text.startswith('/*', i):
            i += 2
            depth = 1
            while i < len(text) and depth:
                if rust and text.startswith('/*', i):
                    depth += 1
                    i += 2
                elif text.startswith('*/', i):
                    depth -= 1
                    i += 2
                else:
                    i += 1
            if depth:
                raise ValueError('unterminated block comment')
            yield 'comment', text[start:i]
        elif rust and (raw := re.match(r'(?:br|r)(#*)"', text[i:])):
            ending = '"' + raw[1]
            end = text.find(ending, i + raw.end())
            if end < 0:
                raise ValueError('unterminated Rust raw string')
            i = end + len(ending)
            yield 'literal', text[start:i]
        elif text[i] == '"' or (text[i] == "'" and (
                not rust or re.match(r"'(?:\\(?:.|u\{[0-9a-fA-F]+\})|[^'\\])'", text[i:]))):
            quote = text[i]
            i += 1
            while i < len(text) and text[i] != quote:
                i += 2 if text[i] == '\\' else 1
            if i >= len(text):
                raise ValueError('unterminated quoted literal')
            i += 1
            yield 'literal', text[start:i]
        elif text[i].isalpha() or text[i] == '_':
            i += 1
            while i < len(text) and (text[i].isalnum() or text[i] == '_'):
                i += 1
            yield 'word', text[start:i]
        else:
            i += 1
            yield 'punctuation', text[start:i]


def strip_comments(text: str, *, rust: bool = False) -> str:
    return ''.join(
        (' ' + '\n' * spelling.count('\n'))
        if kind == 'comment' and not re.search(r'copyright|SPDX-License', spelling, re.I)
        else spelling
        for kind, spelling in tokens(text, rust=rust)
    )


def rewrite_calls(text: str, rules: dict) -> str:
    stream = list(tokens(text))

    def rewrite(parts):
        output = []
        i = 0
        while i < len(parts):
            kind, word = parts[i]
            if kind != 'word' or word not in rules:
                output.append(word)
                i += 1
                continue
            opening = i + 1
            while opening < len(parts) and parts[opening][1].isspace():
                opening += 1
            if opening == len(parts) or parts[opening][1] != '(':
                raise ValueError(f'{word}: expected macro invocation')
            stack = ['(']
            args = []
            start = opening + 1
            cursor = start
            while cursor < len(parts):
                token_kind, spelling = parts[cursor]
                if token_kind == 'punctuation':
                    if spelling in '([{':
                        stack.append(spelling)
                    elif spelling in ')]}':
                        if not stack or stack.pop() != {')': '(', ']': '[', '}': '{'}[spelling]:
                            raise ValueError(f'{word}: unbalanced macro arguments')
                        if not stack:
                            args.append(rewrite(parts[start:cursor]).strip())
                            break
                    elif spelling == ',' and len(stack) == 1:
                        args.append(rewrite(parts[start:cursor]).strip())
                        start = cursor + 1
                cursor += 1
            else:
                raise ValueError(f'{word}: unterminated macro invocation')
            arity, transform = rules[word]
            if len(args) != arity:
                raise ValueError(f'{word}: expected {arity} arguments, got {len(args)}')
            output.append(transform(args))
            i = cursor + 1
        return ''.join(output)

    return rewrite(stream)


def c_condition(expression: str) -> bool | None:
    expression = expression.strip()
    if expression == 'KF_MODERN_TYPES' or re.fullmatch(
            r'KF_MODERN_TYPES\s*&&\s*!defined\(KF_(?:MENU_MODE|MENU_LIST|EFFECT_POOL)_IMPLEMENTATION\)',
            expression):
        return False
    if expression == '!KF_MODERN_TYPES':
        return True
    if expression == 'defined(__cplusplus)':
        return False
    if 'KF_MODERN_TYPES' in expression:
        raise ValueError(f'unsupported type conditional: {expression}')
    return None


def resolve_conditionals(text: str) -> str:
    """Select C-only type branches; leave other preprocessor decisions in source."""
    stack = []
    output = []
    for line in text.splitlines(keepends=True):
        directive = re.match(r'^\s*#\s*(if|ifdef|ifndef|else|elif|endif)\b(.*)', line)
        if directive:
            command, expression = directive.groups()
            if command in ('if', 'ifdef', 'ifndef'):
                condition = c_condition(expression) if command == 'if' else None
                stack.append(None if condition is None else [condition, condition])
                if condition is not None:
                    continue
            elif command in ('else', 'elif'):
                if not stack:
                    raise ValueError('unmatched conditional arm')
                if stack[-1] is not None:
                    frame = stack[-1]
                    condition = True if command == 'else' else c_condition(expression)
                    if condition is None:
                        raise ValueError(f'unsupported conditional arm: {line.strip()}')
                    frame[0] = not frame[1] and condition
                    frame[1] |= frame[0]
                    continue
            elif command == 'endif':
                if not stack:
                    raise ValueError('unmatched endif')
                if stack.pop() is not None:
                    continue
        if all(frame is None or frame[0] for frame in stack):
            output.append(line)
    if stack:
        raise ValueError('unterminated preprocessor conditional')
    return ''.join(output)


def tidy(text: str) -> str:
    text = '\n'.join(line.rstrip() for line in text.splitlines())
    return re.sub(r'\n{3,}', '\n\n', text).strip() + '\n'
