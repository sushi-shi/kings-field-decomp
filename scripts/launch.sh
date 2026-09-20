# Embedded by writeShellApplication, which supplies game_binary and dependencies.
for argument in "$@"; do
  case "$argument" in
    --help|-h)
      printf '%s\n' \
        "Usage: KF_DISC=/path/to/disc.iso kings-field [--saves DIRECTORY] [--skip-intro]" \
        "The original Japanese ISO or BIN/CUE is extracted locally on first launch." \
        "Verified resources are reused from the XDG cache; saves are stored separately." \
        "Explicit --data or --disc/--extract-to/--extract-only options bypass automatic caching."
      exit 0
      ;;
  esac
done

# Preserve the direct application's explicit resource/extraction workflows.
for argument in "$@"; do
  case "$argument" in
    --data|--disc|--extract-to|--extract-only)
      exec "$game_binary" "$@"
      ;;
  esac
done

if [[ -z "${KF_DISC:-}" ]]; then
  printf '%s\n' "Set KF_DISC to your original Japanese King's Field ISO or BIN/CUE." \
    'Example: KF_DISC="/path/to/disc.iso" nix run github:sushi-shi/kings-field-decomp/port' >&2
  exit 1
fi
if [[ ! -f "$KF_DISC" || ! -r "$KF_DISC" ]]; then
  printf 'Cannot read disc image: %s\n' "$KF_DISC" >&2
  exit 1
fi

# Every accepted disc extracts to the same hash-verified Japanese resource set.
# Only this launcher's successful imports are published at resources-v1.
case "${XDG_CACHE_HOME:-}" in
  /*) cache_base="$XDG_CACHE_HOME/kings-field/SLPS-00017" ;;
  *) cache_base="${HOME:?HOME or an absolute XDG_CACHE_HOME is required}/.cache/kings-field/SLPS-00017" ;;
esac
umask 077
mkdir -p -- "$cache_base"
exec {cache_lock}>"$cache_base/import.lock"
flock -x "$cache_lock"
data_directory="$cache_base/resources-v1"
if [[ -L "$data_directory" || ( -e "$data_directory" && ! -d "$data_directory" ) ]]; then
  printf 'Resource cache is not an ordinary directory: %s\n' "$data_directory" >&2
  exit 1
fi
if [[ ! -d "$data_directory" ]]; then
  import_directory=$(mktemp -d -- "$cache_base/import.XXXXXX")
  printf 'Extracting original resources from %s\n' "$KF_DISC" >&2
  if ! "$game_binary" --disc "$KF_DISC" --extract-to "$import_directory/data" --extract-only; then
    printf 'Import failed. Any partial output remains in %s; no cache was replaced.\n' "$import_directory" >&2
    exit 1
  fi
  # Rename within the cache filesystem publishes only a complete extraction.
  mv -T --no-clobber -- "$import_directory/data" "$data_directory"
  if [[ -e "$import_directory/data" ]]; then
    printf 'Cache destination appeared during import; extracted files remain in %s.\n' "$import_directory" >&2
    exit 1
  fi
  rmdir -- "$import_directory"
fi
flock -u "$cache_lock"
exec {cache_lock}>&-
exec "$game_binary" --data "$data_directory" "$@"
