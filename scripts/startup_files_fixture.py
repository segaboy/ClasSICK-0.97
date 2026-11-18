#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original SPEC-0122 composition; never reads a runtime or reference disk."""
import hashlib
import struct
from resource_media_fixture import original_media as resource_media


def original_media():
    logical, resource = resource_media()
    disk = bytearray(logical)
    struct.pack_into('>H4sHH', disk, 0, 0x4c4b, b'\xff\x80\x00\x7f', 18, 0)
    names = [b'Own\x00\x80', b'\xffLong\x00', b'DbgA', b'DbgB', b'Screen', b'Startup', b'Scrap']
    for index, name in enumerate(names):
        offset = 10 + 16 * index
        disk[offset] = len(name)
        disk[offset+1:offset+1+len(name)] = name
    struct.pack_into('>HHIII', disk, 122, 10, 20, 17152, 32768, 49152)
    logical = bytes(disk)
    expected = [0x53544631, 0, 6, 2, 0, 0, 0, 0, 2, 6, 8, 127, 2, 0, 0xb8, 1]
    failed = [0x53544631, 2, 5, 1, 3, 12, 3, 66048, 1, 2, 3, 30, 0, 1, 0xd1, 1]
    return logical, {'expected': expected, 'fault_expected': failed,
                     'fault_lba': resource['fault_lba'], 'pc_fault': resource['pc_fault'],
                     'mac_fault': resource['mac_fault'],
                     'logical_sha256': hashlib.sha256(logical).hexdigest(),
                     'resource_fixture_sha256': resource['logical_sha256'],
                     'scope': 'First and second original resource files selected by binary header names; '
                              'second map beyond64KiB; complete source rebasing and output preservation.'}


if __name__ == '__main__':
    import json
    print(json.dumps(original_media()[1], indent=2))
