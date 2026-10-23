# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Flat-byte oracle; host model tests, never a 286 target execution claim."""
from pathlib import Path
import hashlib
import importlib.util
import json
import random
import types

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'probes/pc286/segmented_model.py'
ORACLE = ROOT / 'tests/data/pc286-segmented-oracle.json'
# Same frozen JSON content, normalized to Git LF. Private original has SHA 521eaa...58dabf.
FROZEN_SHA = '85f4697fedade09f293b6b88779b761d086ae02837caac518c65b46d6011f18f'


def load():
    spec = importlib.util.spec_from_file_location('segmented', SOURCE)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def frozen(m):
    assert hashlib.sha256(ORACLE.read_bytes().replace(b'\r\n', b'\n')).hexdigest() == FROZEN_SHA
    o = json.loads(ORACLE.read_text()); base = o['region_base']
    sizes = o['bank_sizes']; mp = m.build(m.Reg(), [(base, 7, sizes)])
    pattern = bytes((i * 7 + 11) & 255 for i in range(o['region_size']))
    cursor = 0
    for bank, size in zip(mp.regions[0]['banks'], sizes):
        mp.banks[bank][:] = pattern[cursor:cursor + size]; cursor += size
    count = 0
    for case in o['read_cases']:
        out = [0xDEADBEEF]
        code = m.read(mp, base + case['offset'], case['width'], m.R, out)
        assert code == (m.OK if case['expected_status'] == 'ok' else m.ADDR), case['name']
        assert out[0] == case.get('expected_value', 0xDEADBEEF), case['name']
        count += 2
    code, cap = m.open_cap(mp, base, o['region_size'], m.R); assert code == m.OK
    lengths = []
    for _ in range(65):
        code, chunk = m.chunk(cap)
        assert code == m.OK
        if chunk is None: break
        lengths.append(chunk[2])
    else: raise AssertionError('nonterminating capability')
    assert lengths == o['full_capability_expected_chunks']; count += 1
    def make(): return m.build(m.Reg(), [(0, m.R | m.W, [4])])
    t = make(); t.banks[0] = bytearray(1); assert m.validate(t) == m.MAP; count += 1
    t = make(); t.regions[0]['flags'] = 1 | (1 << 32); assert m.validate(t) == m.MAP; count += 1
    t = make(); assert m.window(t, 0, 1, 1 << 32)[0] == m.ARG; count += 1
    t = make(); t.regions[0]['flags'] = m.R; t.banks[1] = [0] * 4
    assert m.validate(t) == m.MAP; count += 1
    reg = m.Reg(); t = m.build(reg, [(0, 3, [4])]); _, cap = m.open_cap(t, 0, 4, m.R)
    before = (reg.n, reg.cur)
    bad = [dict(base=0, size=6, flags=3, banks=[1])]
    assert m.new_map(reg, bad, {1: bytearray(4)})[0] == m.MAP; count += 1
    assert (reg.n, reg.cur) == before; count += 1
    assert m.chunk(cap)[0] == m.OK; count += 1
    reg = m.Reg(); old = m.build(reg, [(0, 3, [4])]); m.build(reg, [(0, 3, [4])])
    assert m.open_cap(old, 0, 4, m.R)[0] == m.STALE; count += 1
    _, cap = m.open_cap(make(), 0, 4, m.R); assert cap.access == m.R; count += 1
    assert count == 38
    return count


def differential(m):
    """Independent flat shadow; no use of the model's span or byte-order helpers."""
    rng = random.Random(0x286097)
    checks = 0
    for sizes in ([1, 3, 65535, 65533], [32767, 32767, 32767, 32767, 4], [65535, 65535, 2]):
        n = sum(sizes); base = 0x20000
        mp = m.build(m.Reg(), [(base, 7, list(sizes))]); shadow = bytearray(n)
        bounds = [sum(sizes[:i]) for i in range(1, len(sizes))]
        offsets = [-2, -1, 0, 1, n - 4, n - 3, n - 2, n - 1, n, n + 1]
        offsets += [b + d for b in bounds for d in range(-4, 5)]
        offsets += [rng.randrange(-4, n + 4) for _ in range(96)]
        for offset in offsets:
            for width in (1, 2, 4, 3):
                value = rng.getrandbits(32)
                status = (m.WIDTH if width not in (1, 2, 4) else
                          m.ADDR if (width != 1 and offset & 1) or offset < 0 or offset + width > n else m.OK)
                before = [bytes(mp.banks[b]) for b in sorted(mp.banks)]
                assert m.write(mp, base + offset, width, value) == status
                if status == m.OK:
                    shadow[offset:offset + width] = (value & ((1 << (width * 8)) - 1)).to_bytes(width, 'big')
                else:
                    assert before == [bytes(mp.banks[b]) for b in sorted(mp.banks)]
                actual = b''.join(bytes(mp.banks[b]) for b in mp.regions[0]['banks'])
                assert actual == shadow
                out = [0xDEADBEEF]
                assert m.read(mp, base + offset, width, m.R, out) == status
                assert out[0] == (int.from_bytes(shadow[offset:offset + width], 'big') if status == m.OK else 0xDEADBEEF)
                checks += 1
        # Descriptor traversal: exact order and coverage, including a smaller consumer limit.
        for limit in (1, 32767):
            code, cap = m.open_cap(mp, base, n, m.R); assert code == m.OK
            collected = bytearray(); total = 0
            for _ in range(n + 1):
                code, chunk = m.chunk(cap, limit); assert code == m.OK
                if chunk is None: break
                bank, at, length = chunk
                assert 0 < length <= limit and at + length <= len(mp.banks[bank])
                collected += mp.banks[bank][at:at + length]; total += length
            else: raise AssertionError('nonterminating capability')
            assert total == n and collected == shadow
            assert m.release(cap) == m.OK and m.chunk(cap)[0] == m.CAP
    # Entry-specific precedence; malformed maps beat width, width beats remaining access bits.
    mp = m.build(m.Reg(), [(0, 3, [4])]); out = [0x1234]
    assert m.read(mp, 0, 3, m.R | 8, out) == m.WIDTH and out == [0x1234]
    mp.regions[0]['flags'] = 0
    assert m.read(mp, 0, 3, m.R | 8, out) == m.MAP and out == [0x1234]
    assert m.window(mp, 0, 1, 8)[0] == m.ARG
    return checks


def controls():
    rejected = []
    for name in ('endian', 'partial-write', 'chunk-bound'):
        m = load()
        if name == 'endian':
            original = m.read
            def wrong(*args):
                result = original(*args)
                if result == m.OK:
                    width, out = args[2], args[4]
                    out[0] = int.from_bytes(out[0].to_bytes(width, 'big'), 'little')
                return result
            m.read = wrong
        elif name == 'partial-write':
            original = m.write
            def wrong(*args):
                result = original(*args)
                if result != m.OK: args[0].banks[1][0] ^= 1
                return result
            m.write = wrong
        else:
            source = SOURCE.read_text().replace('min(k.left, limit)', 'k.left')
            assert source != SOURCE.read_text()
            m = types.ModuleType('bad_chunk'); exec(compile(source, str(SOURCE), 'exec'), m.__dict__)
        try:
            if name == 'partial-write': differential(m)
            elif name == 'endian': frozen(m)
            else:
                mp = m.build(m.Reg(), [(0, 7, [65535, 65535, 2])])
                _, cap = m.open_cap(mp, 0, 131072, m.R)
                assert m.chunk(cap)[1][2] <= 32767
        except AssertionError:
            rejected.append(name)
        else: raise AssertionError('fault control escaped: ' + name)
    assert len(rejected) == 3
    return rejected


if __name__ == '__main__':
    module = load()
    print(json.dumps({'frozen_checks': frozen(module), 'flat_cases': differential(module),
                      'fault_controls_rejected': controls()}))
