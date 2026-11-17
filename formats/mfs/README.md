# MFS format

The first bounded metadata reader lives in `core/storage/mfs-read.[ch]`.
It reads the volume header through generic logical blocks and decodes one
caller-selected directory entry, using bytewise endian conversion and copied
raw names. See [SPEC-0112](../../docs/specifications/SPEC-0112-mfs-metadata-reader.md)
and the [implementation receipt](../../docs/development/mfs-metadata-20261117.md).

Directory enumeration, allocation-map decoding/validation, fork extraction,
File Manager services and writes remain unimplemented. Header/entry decoding is
not a complete mount or System Disk boot. Fixtures are independently generated;
no reference disk or protected code is included.
