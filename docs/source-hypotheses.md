# Source-hypothesis batches

`kf hypotheses` compiles and strict-scores a bounded Cartesian product of
reviewed source spellings for one function. It invokes the configured compiler
directly for only the selected translation unit; it does not run Ninja or a
whole-program build. Every compile receives a disposable sibling source and an
isolated object directory, so parallel states cannot overwrite one another and
the configured source is never rewritten.

The manifest uses exact, unique text spans rather than regular expressions:

```json
{
  "schema": 1,
  "unit": "open.opening_scenes",
  "function": "opening_ending_scroll_run",
  "axes": [
    {
      "name": "phase_test",
      "find": "if (lighting_phase == ENDING_LIGHT_TO_GREEN)",
      "options": [
        {"name": "baseline"},
        {
          "name": "numeric_zero",
          "replace": "if (lighting_phase == 0)"
        }
      ]
    }
  ]
}
```

Each option can include `extra_edits`, a list of additional exact
`{"find": ..., "replace": ...}` substitutions that travel atomically with the
option. Axes must not overlap. Five two-option axes generate 32 states; a
five-by-eight matrix generates 40.

Run it from `nix develop` after the unit's retail module has been generated:

```sh
kf hypotheses /tmp/open-scroll.json -j 8 --keep-top 10
```

The command writes the input manifest, ranked `results.json`, and the requested
number of top candidate sources under `build/hypotheses/`. A score is taken
from objdiff's single-symbol strict diff, including relocation identity. The
tool never applies or banks a winner: inspect the exact candidate, apply the
source-level facts deliberately, then use the normal focused match, full build,
and bank gates.
