#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

case "${1:-}" in
  ''|--check) ;;
  *) echo "Usage: bash codecs/bindings.sh [--check]" >&2; exit 2 ;;
esac

bindings=$(mktemp codecs/src/ffi/bindings.rs.XXXXXX)
trap 'rm -f "$bindings"' EXIT
"${BINDGEN:-bindgen}" include/kf/audio/codec.h \
  --use-core --rust-target 1.85 --rust-edition 2021 \
  --allowlist-type 'Kf.*' --allowlist-var 'KF_.*' --generate types,vars \
  --no-prepend-enum-name --no-doc-comments --no-layout-tests \
  --raw-line '// Generated from the C codec headers by CMake. Do not edit.' \
  --raw-line '#![expect(dead_code, non_camel_case_types)]' \
  --output "$bindings" -- -x c -std=c11 -I include

if [[ "${1:-}" == --check ]]; then
  diff -u codecs/src/ffi/bindings.rs "$bindings"
elif ! cmp -s "$bindings" codecs/src/ffi/bindings.rs; then
  chmod 644 "$bindings"
  mv "$bindings" codecs/src/ffi/bindings.rs
fi
