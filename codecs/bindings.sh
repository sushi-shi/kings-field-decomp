#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

case "${1:-}" in
  ''|--check) ;;
  *) echo "Usage: bash codecs/bindings.sh [--check]" >&2; exit 2 ;;
esac

bindings=$(mktemp)
trap 'rm -f "$bindings"' EXIT
bindgen codecs/ffi.h \
  --use-core --rust-target 1.85 --rust-edition 2021 \
  --allowlist-type 'Kf.*' --allowlist-var 'KF_.*' --generate types,vars \
  --no-prepend-enum-name --no-doc-comments \
  --raw-line '// Regenerate in nix develop: bash codecs/bindings.sh' \
  --raw-line '#![allow(dead_code, non_camel_case_types, non_upper_case_globals)]' \
  --output "$bindings" -- -x c -std=c11 -Wno-pragma-once-outside-header -I include

if [[ "${1:-}" == --check ]]; then
  diff -u codecs/src/ffi/bindings.rs "$bindings"
else
  cp "$bindings" codecs/src/ffi/bindings.rs
fi
