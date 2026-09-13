"""Matching adapters for the standalone compiler and assembler."""

from scripts.kf.compile import _run
from scripts.psxbuild.sdk import (
    assemble as assemble,
    assemble_many as assemble_many,
    compile_c as _compile_c,
    dos_run as dos_run,
    dos_text as dos_text,
    tool_succeeded as tool_succeeded,
)


def compile_c(*args, trace_environment=None, **kwargs):
    def run(arguments, *, environment=None):
        return _run(arguments, trace_environment=environment)

    return _compile_c(*args, environment=trace_environment, run=run, **kwargs)
