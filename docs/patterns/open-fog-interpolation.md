# OPEN fog interpolation expression

## Function Match Plan

At `0d62ce8`, OPEN `80019658 fog_interpolate_near(s32 start, s32 end,
s32 ratio)` occupies 68 retail bytes and 64 compiled bytes, strict 92.941180%.
The six image-qualified queries, full body/CFG, adjacent light-matrix wrapper
and fog setter, SDK callee/header, source history and existing GAME source
homolog were inspected. There are no incoming references, strings, branches
or candidate outgoing references. Its one proven call is SetFogNear; the one
validated HI16/LO16 pair stores the distance to the graphics owner's +24660
field. The 24-byte frame, return delay slot and projection constant 200 agree.
The LIBGTE GEO archive independently identifies SetFogNear; the surrounding
Q12 interpolation and state write are game-owned policy.

Retail preserves start in v0, computes `(end - start) * ratio` into a0,
arithmetic-shifts a0 by 12, then adds preserved start as the second operand.
The current C starts a distance accumulator at start and adds the scaled
difference; it instead leaves start in a0 and computes the change in v0,
omitting the initial move. Test the direct expression
`(((end - start) * ratio) >> 12) + start`, preserving the actual signed
arithmetic and SDK argument. This is an expression hypothesis, not a claimed
historical register-allocation mechanism; no dummy carrier is added.

Rebuild the entire matrix unit and compare all seven functions, preserving
the six exact neighbors. Check every raw word, ordered relocation, call and
delay slot; a fuzzy-score increase alone is not closure.

## Result

The direct expression recovers all 17 retail instructions, including the
initial `move v0,a0`, `mflo a0`, ordered add operands, state store, call and
return delay slot. The compiled size is exactly 68 bytes and strict objdiff
is **100%**. All six other unit functions retain strict 100% and their linked
retail words. No compiler option, type, identity or relocation was changed.

Only OPEN `80019658` is newly banked. OPEN advances **93 -> 94 / 108 exact**,
leaving 14 partial functions and none unstarted. GAME source is unchanged.
The display-initializer pointer experiments in the sibling note produced no
instruction changes and were reverted; floor-item inspection found no new
supported width correction, so that function was not edited.

Full `kf build` retains the known TMD switch-addend failure in OPEN, thirteen
GAME data-addend failures and four GAME historical-best deficits; no exact
function regresses. The existing matrix inventory test's expected score set
is updated to all-exact, without adding new test infrastructure.
Ruff, `git diff --check`, and all 401 existing repository tests pass.
