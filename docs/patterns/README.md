# Compiler and MIPS pattern notes

Reusable, evidence-backed observations about the retail code generation and
about which source shapes reproduce it under the current probes. Each note
records the retail evidence first and the probe behaviour second; none of them
promotes a probe to a proven historical toolchain.

- [gcc257-epilogue-and-scheduling.md](gcc257-epilogue-and-scheduling.md):
  the framed-epilogue delay-slot form, the checked `div` expansion, and the
  prologue/load scheduling that separate the GCC 2.5.7 probe from the 2.6.0
  probe, with the corpus numbers behind the profile change.
- [source-shapes-gcc257.md](source-shapes-gcc257.md): source shapes that
  decided exact matches under the 2.5.7 probe (loop counters, return-value
  joins, struct copies, two-statement accumulations).
- [compiler-attribution.md](compiler-attribution.md): a dedicated
  cross-version pass over the residue corpus. How GCC 2.4.1 CC1PSX was made
  runnable (a go32-stub frankenbinary under DOSBox) and the finding that it
  reproduces retail no better than 2.5.7 `-O2` (byte-identical on most residue
  functions, worse where it diverges); the residue ceiling is an unattributed
  scheduling/regalloc residue, and `-mcpu=r2000` is a per-TU discriminator, not
  a global truth.
- [psyq-cpe-tail-and-bss.md](psyq-cpe-tail-and-bss.md): the CPE v1 prefix
  embedded at GAME's initialized-data tail, and why referenced storage inside
  the final page can still be original BSS rather than initialized source data.
- [open-map-render-residue.md](open-map-render-residue.md): RTBL window
  ownership, conflicting light-matrix base forms in map rendering and
  initialization, and the visibility-traversal setup residue.
- [open-floor-item-render-residue.md](open-floor-item-render-residue.md):
  typed floor-item sprite descriptors and the extra facing-byte mask shared
  by the GAME and OPEN retail renderers.
- [open-sprite-render.md](open-sprite-render.md): typed sprite material,
  SDK packet ownership, and the depth/texture source forms behind the exact
  OPEN projected-sprite renderer.
- [open-ft4-descriptor.md](open-ft4-descriptor.md): the byte-identical SDK
  rectangle trial and why texture byte loads do not establish source field width.
- [open-camera-step.md](open-camera-step.md): direct lane publication trial
  and the preserved fixed-point camera-step residue.
- [open-tmd-countdown.md](open-tmd-countdown.md): unconditional primitive
  countdown initialization leaves only two frame instructions unmatched.
- [open-entity-traversal-residue.md](open-entity-traversal-residue.md): visible
  entity/item traversal, the wider floor-item state owner, and the remaining
  entry/material-base instruction differences.
- [notification-state.md](notification-state.md): retail payload/control
  base relationships, the variadic enqueue signature, and the distinction
  between the recovered notification fields and the wider graphics object.
- [render-material.md](render-material.md): supported texture/colour identities,
  unresolved aggregate boundaries, and the retail effect-pool skip behavior.
- [tmd-counts.md](tmd-counts.md): word-sized disk counts, explicit halfword
  narrowing in preparation, and the shared GAME/OPEN TMD interface.
- [open-unlit-triangles.md](open-unlit-triangles.md): the unlit F3/FT3 path,
  packed GPU accesses, and the unresolved graphics-wide shared-base relation.
- [open-graphics-runtime-owner.md](open-graphics-runtime-owner.md): complete
  graphics owner, exact map traversal/VRAM adjustment, and preserved initializer controls.
- [open-map-enqueue.md](open-map-enqueue.md): lit map GT3/GT4 emission,
  the complete colour owner, and byte-identical shared packed-coordinate types.
- [tmd-projection.md](tmd-projection.md): projection/accessor ownership,
  the observed ReadSZ2 call contract, and unattributed register differences.
- [animation-cache.md](animation-cache.md): animation-pool metadata, typed
  cache pointers, complete BSS extent and contiguous binder/lifecycle ownership.
- [mips-parser-instrumentation.md](mips-parser-instrumentation.md): isolated
  parser write guards, load-delay audit, and reduced Unicorn memory-hook
  control-flow defects.
- [game-parser-verification.md](game-parser-verification.md): parser failure
  controls, VAB partial state and cleanup quirks, and seven-function match review.
- [open-format-display-sdk.md](open-format-display-sdk.md): four exact custom
  formatters, the display-adjustment address-base residue, twelve SDK provider
  attributions, and the corrected SquareRoot12 boundary.
- [open-ending-scroll.md](open-ending-scroll.md): complete ending-scroll C,
  initialized panel/camera/color owners, the unreachable epilogue correction,
  and the remaining short-state and branch-join comparison.
- [open-tmd-enqueue.md](open-tmd-enqueue.md): all twelve general TMD polygon
  modes, SDK-preserving packed views, static compiled/retail call-path controls,
  and the remaining shared graphics-base and instruction differences.
- [open-transition-snapshot.md](open-transition-snapshot.md): the directly
  observed transition-position stack copy and the remaining eight-byte frame
  discrepancy, with an exact list of non-exact object differences.
- [open-display-init-pointers.md](open-display-init-pointers.md): observed
  DRAWENV pointer lifetimes and height initialization, with bounded retail/C
  argument/write traces and remaining address-base differences.
- [open-fog-interpolation.md](open-fog-interpolation.md): scaled-difference
  expression recovering the exact OPEN fog interpolation and its preserved start.
- [open-item-cursor.md](open-item-cursor.md): saved stream start, postincrement
  scan and direct count-member accesses closing the OPEN placement loader.
- [game-parser-exact-closure.md](game-parser-exact-closure.md): six remaining
  parser match attempts, retained source improvements and instruction-level residues.
- [game-parser-inline-helpers.md](game-parser-inline-helpers.md): pinned-compiler
  inline support, six parser helper experiments and the retained typed vertex copy.
- [game-world-state-layout.md](game-world-state-layout.md): the complete map
  runtime owner, preserved interior referents and newly exact spinner consumer.
- [tmd-packet-header.md](tmd-packet-header.md): bounded packet-header inline
  experiments, their stack traffic and the item-count reset negative control.
