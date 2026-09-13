# Counting resource field values

`kf resources census` counts an integer field in a caller-specified region of
fixed-size binary records. Use an independently verified codec, structure
layout or loader to establish the region, stride, field offset and encoding.
The tool reports byte evidence; it does not decode a guessed schema, name an
operation, associate a record with a mesh, or infer that an unused value has
no runtime meaning.

Run in `nix develop`, after validating the retail files with `kf init`:

```sh
kf resources census /path/to/disc/KF/COM/COM.DAT \
  --offset 0x1610 --length 1128 --stride 8 \
  --field-offset 0 --encoding u8 --value 3 --json \
  > build/com-map-object-operation-census.json
```

This example selects the 141 stored rows of COM.DAT chunk 5. The existing
[`CommonData` codec](../tools/kf-codec/src/game_data.rs) identifies that chunk;
[`MapObjectDefinition`](../tools/kf-codec/src/records.rs) decodes its
8-byte records and first-byte behavior field. The census retains the raw
integer rather than substituting a semantic enum name. `--value 3` includes
a zero-count row when that value does not occur; it does not filter out other
observed values.

JSON includes the full resource and selected region hashes, the complete
layout specification, sorted value counts, zero-based row indices, absolute
resource byte offsets, raw field bytes and raw record bytes. Without `--json`,
the command emits a compact TSV of values, counts, indices and field offsets.
Generated reports belong under `build/`, not in version control.

`--count N` can replace `--length N`, with the selected byte length computed
as count times stride. Supported encodings include unsigned/signed bytes and
16-, 32- and 64-bit integers with explicit little/big endian order. Offsets and
extents accept decimal or hexadecimal. Invalid bounds, a field extending past
its record, or a region with trailing partial-record bytes abort with status 2
and no partial output. Bytes outside the selected region are deliberately not
interpreted.

Region choice is part of the evidence. GAME's map-object loader actually
copies 160 records: the 141-row payload plus the following growth chunk's
length and first 148 payload bytes. The codec exposes this separately as
`map_object_definitions_source`. To inspect exactly the loader's source range,
use `--count 160` instead of `--length 1128`; do not describe that larger range
as 160 independently stored map-object rows. A zero count in the selected
payload cannot by itself prove global absence from code, other resources or
runtime mutations.
