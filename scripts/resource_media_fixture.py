#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original synthetic SPEC-0115/0116/0121 media and declarative report oracle.

No media file, reference software, implementation bytes or external fixture is
read. This is a generator for a bounded metadata workload, not an OS disk image.
Running this module checks the generated layout and prints its aggregate oracle;
it does not write the generated 400 KiB payload to the source tree.
"""
import hashlib
import json
import struct


BLOCK_BYTES = 512
DISK_BLOCKS = 800
ALLOCATION_FIRST = 16
ALLOCATION_BYTES = 1024
ALLOCATION_COUNT = 392
PC_TRANSFER_FIRST = 60
FAULT_CACHE_POSITION = 66048


def _require(condition, message):
    if not condition:
        raise ValueError('original resource fixture: ' + message)


def _u16(target, offset, value):
    struct.pack_into('>H', target, offset, value)


def _u32(target, offset, value):
    struct.pack_into('>I', target, offset, value)


def _pattern(length, salt):
    return bytes((index * 29 + (index >> 8) + salt) & 255
                 for index in range(length))


def _item(resource_id, payload, name=None, attributes=0, specification=0):
    _require(-32768 <= resource_id <= 32767, 'resource ID outside signed16')
    _require(name is None or len(name) <= 255, 'resource name too long')
    _require(bool(attributes & 128) == (payload is None), 'external/local mismatch')
    return {'id': resource_id, 'payload': payload, 'name': name,
            'attributes': attributes, 'specification': specification}


def _resource_fork(groups, type_padding=0, name_padding=0, reserved=False):
    """Encode declared resources and compute expectations from their attributes.

    The encoder deliberately permits signed IDs, raw names, an empty per-type
    list, ordinary opaque handles and bit7 references. It does not use either C
    reader or infer its expected warning bits from their output.
    """
    data = bytearray()
    names = bytearray()
    records = []
    warnings = (1 if type_padding or name_padding else 0) | (2 if reserved else 0)
    for type_key, items in groups:
        _require(0 <= type_key <= 0xffffffff, 'type key outside uint32')
        if not items:
            warnings |= 32
        for item in items:
            record = bytearray(12)
            _u16(record, 0, item['id'] & 65535)
            name = item['name']
            _u16(record, 2, 65535 if name is None else len(names))
            if name is not None:
                names.append(len(name))
                names.extend(name)
            attributes = item['attributes']
            record[4] = attributes
            if attributes & 3:
                warnings |= 8
            if attributes & 128:
                record[5:8] = b'\xff\xff\xff'
                warnings |= 16
            else:
                _require(len(data) <= 0xffffff, '24-bit local prefix offset')
                record[5:8] = len(data).to_bytes(3, 'big')
                payload = item['payload']
                data.extend(struct.pack('>I', len(payload)))
                data.extend(payload)
                if len(payload) > 32768:
                    warnings |= 64
                if item['specification']:
                    warnings |= 4
            _u32(record, 8, item['specification'])
            records.append(record)

    type_relative = 28 + type_padding
    references_relative = type_relative + 2 + 8 * len(groups)
    names_relative = references_relative + 12 * len(records) + name_padding
    map_bytes = bytearray(names_relative + len(names))
    if reserved:
        map_bytes[9] = 0x5a
    _u16(map_bytes, 24, type_relative)
    _u16(map_bytes, 26, names_relative)
    _u16(map_bytes, type_relative, len(groups) - 1 if groups else 65535)
    reference_cursor = references_relative
    for index, (type_key, items) in enumerate(groups):
        at = type_relative + 2 + 8 * index
        _u32(map_bytes, at, type_key)
        _u16(map_bytes, at + 4, len(items) - 1 if items else 65535)
        _u16(map_bytes, at + 6, reference_cursor - type_relative)
        reference_cursor += 12 * len(items)
    for index, record in enumerate(records):
        start = references_relative + 12 * index
        map_bytes[start:start + 12] = record
    map_bytes[names_relative:] = names
    map_offset = 256 + len(data)
    fork = bytearray(map_offset + len(map_bytes))
    struct.pack_into('>IIII', fork, 0, 256, map_offset, len(data), len(map_bytes))
    fork[256:map_offset] = data
    fork[map_offset:] = map_bytes
    return bytes(fork), {'types': len(groups), 'references': len(records),
                         'warnings': warnings, 'map_offset': map_offset,
                         'map_length': len(map_bytes), 'name_bytes': len(names)}


def _declarations():
    small_groups = [
        (0x4f574e31, [
            _item(-32768, _pattern(4, 11), b'\x00N\xff'),
            _item(32767, _pattern(900, 37), b'', 3, 0x10203040),
        ]),
        (0x80ff0041, [_item(0, None, None, 128, 0x8000ffff)]),
    ]
    large_groups = [
        (0x4c4f4e47, [
            _item(-1, _pattern(65001, 71), b'Long\x00\xff'),
            _item(1, _pattern(256, 103), None, 2),
        ]),
        (0x00ff0080, [
            _item(-32768, b'', b''),
            _item(32767, _pattern(513, 149), bytes(range(255)), 0, 0x01020304),
        ]),
        (0x52454621, [_item(42, None, b'raw\x80', 128, 0xff002244)]),
        (0x454d5054, []),
    ]
    small, small_counts = _resource_fork(small_groups, reserved=True)
    large, large_counts = _resource_fork(large_groups, type_padding=2,
                                          name_padding=3, reserved=True)
    # The high resource offset maps to low physical block7. Its preceding page
    # uses that block's first sector; its second cache page uses the second.
    large_chain = [3, 8, 4, 9, 5] + list(range(10, 69)) + [7]
    return [
        {'name': b'Own\x00\x80', 'id': 10, 'fork': small, 'chain': [2, 6],
         'directory': (4, 0), 'counts': small_counts},
        {'name': b'\xffLong\x00', 'id': 11, 'fork': large, 'chain': large_chain,
         'directory': (4, 56), 'counts': large_counts},
        {'name': b'', 'id': 12, 'fork': b'', 'chain': [],
         'directory': (5, 0), 'counts': None},
        {'name': b'Zero\x00\xff', 'id': 13, 'fork': b'', 'chain': [69],
         'directory': (6, 0), 'counts': None},
    ]


def _disk_sector(chain, fork_position):
    allocation_index, within = divmod(fork_position, ALLOCATION_BYTES)
    allocation = chain[allocation_index]
    return (ALLOCATION_FIRST + (allocation - 2) * 2 + within // BLOCK_BYTES)


def _verify(logical, files, links):
    """Check serialization and round trips independently of the packing helpers."""
    _require(len(logical) == 409600, 'wrong disk length')
    header = 2 * BLOCK_BYTES
    _require(struct.unpack_from('>H', logical, header)[0] == 0xd2d7, 'signature')
    _require(struct.unpack_from('>H', logical, header + 12)[0] == len(files), 'file count')
    decoded = []
    for index in range(0, ALLOCATION_COUNT, 2):
        at = header + 64 + index * 3 // 2
        pair = int.from_bytes(logical[at:at + 3], 'big')
        decoded.extend([pair >> 12, pair & 4095])
    _require(decoded == links, 'packed allocation links')
    _require(decoded.count(0) == struct.unpack_from('>H', logical, header + 34)[0],
             'free allocation count')
    occupied = set()
    directory_end = {}
    for item in files:
        chain = item['chain']
        data = bytearray()
        current = chain[0] if chain else 0
        actual_chain = []
        while current:
            _require(2 <= current <= ALLOCATION_COUNT + 1, 'allocation range')
            _require(current not in occupied, 'cycle or cross-linked fixture')
            occupied.add(current)
            actual_chain.append(current)
            start = (ALLOCATION_FIRST + (current - 2) * 2) * BLOCK_BYTES
            data.extend(logical[start:start + ALLOCATION_BYTES])
            next_block = decoded[current - 2]
            _require(next_block == 1 or 2 <= next_block <= ALLOCATION_COUNT + 1,
                     'free/reserved link in fixture fork')
            current = 0 if next_block == 1 else next_block
        _require(actual_chain == chain, 'reconstructed fragmented chain')
        _require(data[:len(item['fork'])] == item['fork'], 'logical fork round trip')
        lba, offset = item['directory']
        at = lba * BLOCK_BYTES + offset
        _require(logical[at] == 128, 'inactive directory record')
        _require(struct.unpack_from('>I', logical, at + 18)[0] == item['id'], 'file ID')
        _require(struct.unpack_from('>HII', logical, at + 22) == (0, 0, 0), 'data fork')
        actual = struct.unpack_from('>HII', logical, at + 32)
        _require(actual == (chain[0] if chain else 0, len(item['fork']),
                            len(chain) * ALLOCATION_BYTES), 'resource fork metadata')
        name_length = logical[at + 50]
        _require(logical[at + 51:at + 51 + name_length] == item['name'], 'raw file name')
        extent = (51 + name_length + 1) & ~1
        _require(offset >= directory_end.get(lba, 0), 'directory overlap')
        directory_end[lba] = offset + extent
    for lba in range(4, 8):
        tail = logical[lba * BLOCK_BYTES + directory_end.get(lba, 0):(lba + 1) * BLOCK_BYTES]
        _require(not any(tail), 'nonzero directory tail')
    _require(len(occupied) == 68 and decoded.count(0) == 324, 'declared allocation totals')


def original_media():
    """Return (400KiB logical bytes, original aggregate/fault oracle dictionary)."""
    files = _declarations()
    disk = bytearray(DISK_BLOCKS * BLOCK_BYTES)
    header = 2 * BLOCK_BYTES
    for offset, value in ((0, 0xd2d7), (10, 0), (12, len(files)), (14, 4),
                          (16, 4), (18, ALLOCATION_COUNT), (28, ALLOCATION_FIRST)):
        _u16(disk, header + offset, value)
    for offset, value in ((2, 1), (6, 0), (20, ALLOCATION_BYTES),
                          (24, ALLOCATION_BYTES), (30, 14)):
        _u32(disk, header + offset, value)
    volume_name = b'OwnRsrc\x00\x80'
    disk[header + 36] = len(volume_name)
    disk[header + 37:header + 37 + len(volume_name)] = volume_name

    links = [0] * ALLOCATION_COUNT
    for item in files:
        chain, fork = item['chain'], item['fork']
        _require(len(fork) <= len(chain) * ALLOCATION_BYTES, 'fork exceeds allocation')
        for index, allocation in enumerate(chain):
            _require(2 <= allocation <= ALLOCATION_COUNT + 1, 'declared allocation range')
            _require(links[allocation - 2] == 0, 'duplicate allocation assignment')
            links[allocation - 2] = chain[index + 1] if index + 1 < len(chain) else 1
            start = (ALLOCATION_FIRST + (allocation - 2) * 2) * BLOCK_BYTES
            part = fork[index * ALLOCATION_BYTES:(index + 1) * ALLOCATION_BYTES]
            disk[start:start + len(part)] = part
        lba, offset = item['directory']
        at = lba * BLOCK_BYTES + offset
        disk[at] = 128
        _u32(disk, at + 18, item['id'])
        _u16(disk, at + 32, chain[0] if chain else 0)
        _u32(disk, at + 34, len(fork))
        _u32(disk, at + 38, len(chain) * ALLOCATION_BYTES)
        disk[at + 50] = len(item['name'])
        disk[at + 51:at + 51 + len(item['name'])] = item['name']
    _u16(disk, header + 34, links.count(0))
    for index in range(0, ALLOCATION_COUNT, 2):
        a, b = links[index:index + 2]
        at = header + 64 + index * 3 // 2
        disk[at:at + 3] = bytes((a >> 4, ((a & 15) << 4) | (b >> 8), b & 255))

    completed = [item['counts'] for item in files if item['fork']]
    warnings = 0
    for counts in completed:
        warnings |= counts['warnings']
    expected = [0x52465031, 5, len(files), len(completed), len(completed),
                sum(row['types'] for row in completed),
                sum(row['references'] for row in completed), warnings,
                0, 0, 0, 0, 0, len(files), 0xb8, 0]
    _require(expected == [0x52465031, 5, 4, 2, 2, 6, 8, 127,
                          0, 0, 0, 0, 0, 4, 0xb8, 0], 'baseline declaration')
    first, failing = files[0]['counts'], files[1]
    _require(first['warnings'] == 30 and len(failing['fork']) == 66438,
             'fixed first/large fork properties')
    _require(failing['counts']['map_offset'] == 66042, 'late map-header boundary')
    _require(FAULT_CACHE_POSITION == ((66042 >> 8) + 1) << 8, 'second map cache page')
    fault_lba = _disk_sector(failing['chain'], FAULT_CACHE_POSITION)
    _require(fault_lba == 27 and _disk_sector(failing['chain'], 65792) == 26,
             'distinct physical sectors for the two map cache pages')
    fault_expected = [0x52465031, 4, 2, 2, 1, first['types'], first['references'],
                      first['warnings'], 2, 2, 12, 3, FAULT_CACHE_POSITION,
                      len(files), 0xd4, 0]
    pc_lba = PC_TRANSFER_FIRST + fault_lba
    cylinder, remainder = divmod(pc_lba, 30)
    head, sector_index = divmod(remainder, 15)
    _require(fault_lba < 16 * 12, 'Mac fault outside first 12-sector zone')
    mac_track, mac_sector = divmod(fault_lba, 12)
    logical = bytes(disk)
    _verify(logical, files, links)
    oracle = {
        'expected': expected, 'fault_expected': fault_expected,
        'fault_lba': fault_lba, 'fault_cache_position': FAULT_CACHE_POSITION,
        'pc_fault': [cylinder, head, sector_index + 1],
        'mac_fault': [mac_track, mac_sector],
        'logical_bytes': len(logical), 'logical_sha256': hashlib.sha256(logical).hexdigest(),
        'allocation_bytes': ALLOCATION_BYTES, 'allocation_count': ALLOCATION_COUNT,
        'used_allocations': 68, 'free_allocations': 324,
        'files': [{'ordinal': index + 1, 'logical_resource_bytes': len(item['fork']),
                   'physical_resource_bytes': len(item['chain']) * ALLOCATION_BYTES,
                   'allocation_chain': item['chain'], 'directory_lba': item['directory'][0],
                   'directory_offset': item['directory'][1], 'resource_metadata': item['counts']}
                  for index, item in enumerate(files)],
        'fault_scope': 'Second fork map fixed-header fetch crosses cache pages65792/66048; '
                       'the latter maps to logical diskLBA27. Only the first completed fork '
                       'contributes to opened/type/reference/warning aggregates.',
    }
    return logical, oracle


if __name__ == '__main__':
    _, result = original_media()
    print(json.dumps(result, indent=2))
