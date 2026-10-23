# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# Reference model only. A Python bytearray is NOT qualified 286 segmented access.
OK, ARG, MAP, WIDTH, ADDR, PERM, STALE, CAP, EXH = range(9)
R, W, X = 1, 2, 4
HI = 0xFFFFFFF8
CHUNK = 32767
TOP = 1 << 24


class Reg:
    def __init__(s, limit=0xFFFFFFFF):
        s.n = 0
        s.cur = 0
        s.limit = limit


class Map:
    def __init__(s, reg, regions, banks, glimit):
        s.reg = reg
        s.life = reg.n
        s.gen = 0
        s.glimit = glimit
        s.regions = regions
        s.banks = banks


def new_map(reg, regions, banks, glimit=0xFFFFFFFF):
    m = Map(reg, regions, banks, glimit)
    c = validate(m)
    if c:
        return c, None
    if reg.n >= reg.limit:
        return EXH, None
    reg.n += 1
    reg.cur = m.life = reg.n
    return OK, m


def mutate(m):
    if m.gen >= m.glimit:
        return EXH
    m.gen += 1
    return OK


def validate(m):
    if m is None:
        return ARG
    rs = m.regions
    if not rs or len(rs) > 8 or len(m.banks) > 64:
        return MAP
    seen = set()
    for i, r in enumerate(rs):
        base, size, fl = r['base'], r['size'], r['flags']
        if (size <= 0 or base < 0 or ((base | size) & 1) or base >= TOP or size > TOP - base
                or type(fl) is not int or fl < 0 or fl > 0xFFFFFFFF
                or fl == 0 or (fl & HI) or not fl & R):
            return MAP
        for p in rs[:i]:
            if not (base + size <= p['base'] or p['base'] + p['size'] <= base):
                return MAP
        total = 0
        if not r['banks']:
            return MAP
        for b in r['banks']:
            if type(b) is not int or b < 1 or b > 64 or b in seen or b not in m.banks:
                return MAP
            seen.add(b)
            buf = m.banks[b]
            if (type(buf) not in (bytes, bytearray) or not 1 <= len(buf) <= 65535
                    or (fl & W and type(buf) is not bytearray)):
                return MAP
            total += len(buf)
        if total != size:
            return MAP
    if set(m.banks) != seen:
        return MAP
    return OK


def segs(m, r, off, n):
    out = []
    for b in r['banks']:
        sz = len(m.banks[b])
        if off >= sz:
            off -= sz
            continue
        k = min(n, sz - off)
        out.append((b, off, k))
        n -= k
        off = 0
        if n == 0:
            break
    return out


def window(m, at, n, access):
    if type(access) is not int or access <= 0 or access > 0xFFFFFFFF or (access & HI):
        return ARG, None
    c = validate(m)
    if c:
        return c, None
    if n <= 0 or at < 0 or at >= TOP or n > TOP - at:
        return ADDR, None
    for r in m.regions:
        if r['base'] <= at and n <= r['size'] and at - r['base'] <= r['size'] - n:
            if (r['flags'] & access) != access:
                return PERM, None
            return OK, (r, at - r['base'])
    return ADDR, None


def operand(m, at, width, access):
    c = validate(m)
    if c:
        return c, None
    if width not in (1, 2, 4):
        return WIDTH, None
    if width != 1 and at & 1:
        return ADDR, None
    return window(m, at, width, access)


def read(m, at, width, access, out):
    if (out is None or type(access) is not int or access < 0 or access > 0xFFFFFFFF
            or not access & R):
        return ARG
    c, v = operand(m, at, width, access)
    if c:
        return c
    val = 0
    for b, o, k in segs(m, v[0], v[1], width):
        for i in range(k):
            val = (val << 8) | m.banks[b][o + i]
    out[0] = val
    return OK


def write(m, at, width, value):
    c, v = operand(m, at, width, W)
    if c:
        return c
    data = [(value >> ((width - 1 - i) * 8)) & 255 for i in range(width)]
    j = 0
    for b, o, k in segs(m, v[0], v[1], width):
        for i in range(k):
            m.banks[b][o + i] = data[j]
            j += 1
    return OK


class Cap:
    pass


def open_cap(m, at, n, access):
    c, v = window(m, at, n, access)
    if c:
        return c, None
    if m.reg.cur != m.life:
        return STALE, None
    k = Cap()
    k.access = access
    k.m, k.life, k.gen = m, m.life, m.gen
    k.r, k.off, k.left, k.done = v[0], v[1], n, False
    return OK, k


def chunk(k, limit=CHUNK):
    if k is None or limit < 1 or limit > CHUNK:
        return ARG, None
    if k.done:
        return CAP, None
    if k.m.reg.cur != k.life or k.m.life != k.life or k.m.gen != k.gen:
        return STALE, None
    if k.left == 0:
        return OK, None
    s = segs(k.m, k.r, k.off, min(k.left, limit))[0]
    k.off += s[2]
    k.left -= s[2]
    return OK, s


def rights(k):
    return k.access


def release(k):
    if k is None:
        return ARG
    k.done = True
    return OK


def build(reg, layout, **kw):
    regions, banks, i = [], {}, 1
    for base, fl, sizes in layout:
        ids = []
        for z in sizes:
            banks[i] = bytearray(z)
            ids.append(i)
            i += 1
        regions.append(dict(base=base, size=sum(sizes), flags=fl, banks=ids))
    return new_map(reg, regions, banks, **kw)[1]


def image(m, ri=0):
    return b''.join(bytes(m.banks[b]) for b in m.regions[ri]['banks'])


def snap(m):
    return [bytes(b) for b in m.banks.values()]


def drain(k):
    got = []
    while True:
        c, s = chunk(k)
        assert c == OK
        if s is None:
            return got
        got.append(s)


def main():
    S = [30001, 32767, 32768, 17535, 17001, 1000]
    m = build(Reg(), [(0, 7, S)])
    assert validate(m) == OK and sum(S) == 131072
    sh = bytearray(131072)
    bounds, t = [], 0
    for z in S[:-1]:
        t += z
        bounds.append(t)
    n = 0
    for bd in bounds:
        for at in range(bd - 4, bd + 3):
            for w in (1, 2, 4):
                n += 1
                val = (0x9E3779B1 * n) & 0xFFFFFFFF
                if w > 1 and at & 1:
                    assert write(m, at, w, val) == ADDR and image(m) == bytes(sh)
                    continue
                assert write(m, at, w, val) == OK
                sh[at:at + w] = (val & ((1 << (8 * w)) - 1)).to_bytes(w, 'big')
                o = [None]
                assert read(m, at, w, R, o) == OK
                assert o[0] == int.from_bytes(sh[at:at + w], 'big')
                assert image(m) == bytes(sh)
    assert write(m, 29998, 4, 0x01020304) == OK
    assert bytes(m.banks[1][-3:]) == b'\x01\x02\x03' and m.banks[2][0] == 4
    # chunk lengths over a 3-bank 131072-byte region
    m = build(Reg(), [(0, 7, [65535, 65535, 2])])
    for ln in (32767, 32768, 65535, 65536, 131072):
        c, k = open_cap(m, 0, ln, R)
        assert c == OK
        ch = drain(k)
        assert sum(x[2] for x in ch) == ln and all(0 < x[2] <= CHUNK for x in ch)
        assert release(k) == OK and chunk(k)[0] == CAP
    assert open_cap(m, 1, 131072, R)[0] == ADDR
    # invalid later bank: no partial effect, output unchanged
    m = build(Reg(), [(0, 7, [3, 5])])
    m.regions[0]['banks'][1] = 99
    before, o = snap(m), [0xDEAD]
    assert write(m, 2, 4, 0xAABBCCDD) == MAP and snap(m) == before
    assert read(m, 2, 4, R, o) == MAP and o == [0xDEAD]
    m.regions[0]['banks'][1] = 1
    assert validate(m) == MAP
    m.regions[0]['banks'] = [1, 2]
    m.regions[0]['size'] = 9
    assert validate(m) == MAP
    m = build(Reg(), [(0, 7, [4]), (8, 3, [4])])
    m.banks[3] = m.banks.pop(2)
    m.regions[1]['banks'] = [3]
    assert validate(m) == OK
    m.banks[0] = bytearray(1)
    assert validate(m) == MAP
    del m.banks[0]
    m.banks[2] = bytearray(0)
    m.regions[1]['banks'] = [2]
    assert validate(m) == MAP
    m = build(Reg(), [(0, 7, [4]), (8, 3, [4])])
    m.regions[1]['banks'] = [1]
    assert validate(m) == MAP
    # 24-bit limits
    m = build(Reg(), [(0xFFFFFE, 7, [1, 1])])
    o = [0]
    assert write(m, 0xFFFFFE, 2, 0x1234) == OK and read(m, 0xFFFFFE, 2, R, o) == OK and o[0] == 0x1234
    assert read(m, 0xFFFFFF, 1, R, o) == OK and o[0] == 0x34
    assert read(m, 0xFFFFFF, 2, R, o) == ADDR and read(m, TOP, 1, R, o) == ADDR
    assert window(m, 0xFFFFFF, 2, R)[0] == ADDR
    assert build(Reg(), [(0xFFFFFE, 7, [4])]) is None
    # adjacent distinct regions
    m = build(Reg(), [(0, 7, [4]), (4, 7, [4])])
    assert validate(m) == OK and window(m, 2, 4, R)[0] == ADDR and open_cap(m, 2, 4, R)[0] == ADDR
    # flags, permissions, alignment, precedence
    for fl in (1 | 8, 1 | (1 << 31), 1 | (1 << 32), 0, 2, 4):
        assert build(Reg(), [(0, fl, [4])]) is None
    m = build(Reg(), [(0, 1, [4]), (4, 7, [4])])
    before, o = snap(m), [7]
    assert write(m, 0, 1, 1) == PERM and snap(m) == before
    assert read(m, 0, 1, R | X, o) == PERM and o == [7]
    assert read(m, 0, 1, X, o) == ARG and read(m, 0, 1, R, None) == ARG
    assert read(m, 1, 2, R, o) == ADDR and read(m, 0, 3, R, o) == WIDTH and o == [7]
    assert window(m, 0, 1, 0)[0] == ARG and window(m, 0, 1, 1 << 32)[0] == ARG
    assert read(None, 0, 3, R, o) == ARG
    m.regions[0]['flags'] = 0
    assert read(m, 0, 3, R, o) == MAP
    # stale epoch, replaced lifetime, exhaustion
    reg = Reg(2)
    m = build(reg, [(0, 7, [8])])
    c, k = open_cap(m, 0, 8, R)
    assert c == OK and mutate(m) == OK and chunk(k)[0] == STALE
    c, k = open_cap(m, 0, 8, R)
    m2 = build(reg, [(0, 7, [8])])
    assert chunk(k)[0] == STALE and new_map(reg, m.regions, m.banks) == (EXH, None)
    m3 = build(Reg(), [(0, 7, [8])], glimit=1)
    assert mutate(m3) == OK and mutate(m3) == EXH and m3.gen == 1
    # full-span preflight precedes first chunk
    m = build(Reg(), [(0, 7, [4, 4])])
    m.banks[2] = bytearray(2)
    assert open_cap(m, 0, 4, R) == (MAP, None)
    # malformed candidate maps: no registry effect, no map; old cap stays valid
    reg = Reg()
    m = build(reg, [(0, 7, [8])])
    c, k = open_cap(m, 0, 8, R | W)
    assert c == OK and rights(k) == R | W
    bad = {1: bytearray(4), 2: bytearray(4)}
    st = (reg.n, reg.cur)
    assert new_map(reg, [dict(base=0, size=8, flags=7, banks=[1, 99])], bad) == (MAP, None)
    assert new_map(reg, [dict(base=0, size=8, flags=7, banks=[1, 2])],
                   {**bad, 3: bytearray(1)}) == (MAP, None)
    assert new_map(reg, [dict(base=0, size=8, flags=1 | (1 << 32), banks=[1, 2])], bad) == (MAP, None)
    assert new_map(reg, [dict(base=0, size=8, flags=1, banks=[1, 2])],
                   {1: bytearray(4), 2: memoryview(bytearray(4))}) == (MAP, None)
    assert (reg.n, reg.cur) == st and m.life == reg.cur
    assert chunk(k)[0] == OK and open_cap(m, 0, 8, R)[0] == OK
    # exact backing keys; bytes only for read-only regions
    m = build(Reg(), [(0, 1, [4])])
    m.banks[1] = bytes(4)
    assert validate(m) == OK
    m.regions[0]['flags'] = 3
    assert validate(m) == MAP
    m = build(Reg(), [(0, 7, [4])])
    m.banks[2] = bytearray(1)
    assert validate(m) == MAP
    assert window(build(Reg(), [(0, 7, [4])]), 0, 1, R | (1 << 32))[0] == ARG
    # obsolete lifetime cannot open; stored rights
    reg = Reg()
    m = build(reg, [(0, 7, [8])])
    c, k = open_cap(m, 0, 4, R)
    assert c == OK and rights(k) == R
    build(reg, [(0, 7, [8])])
    assert open_cap(m, 0, 4, R) == (STALE, None) and chunk(k)[0] == STALE
    print('ok')


if __name__ == '__main__':
    main()
