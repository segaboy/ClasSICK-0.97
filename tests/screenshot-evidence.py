# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Adversarial tests for scripts/check_screenshots.py (--root REPO)."""
import argparse, binascii, hashlib, json, os, shutil, struct, subprocess, sys, tempfile, unittest, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
VALIDATOR = os.path.join(os.path.dirname(HERE), "scripts", "check_screenshots.py")
DIR = "docs/evidence/screenshots/"
MAN = DIR + "manifest.json"
PNGP = DIR + "2026-01-02/shot-one.png"
EVID = "docs/evidence/note.md"
SIG = b"\x89PNG\r\n\x1a\n"


def chunk(t, d=b"", crc=None):
    c = binascii.crc32(t + d) & 0xFFFFFFFF if crc is None else crc
    return struct.pack(">I", len(d)) + t + d + struct.pack(">I", c)


def ihdr(w=1, h=1, bd=8, ct=2, cm=0, fl=0, il=0):
    return chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, bd, ct, cm, fl, il))


IDAT = chunk(b"IDAT", zlib.compress(b"\0\0\0\0"))
IEND = chunk(b"IEND")
GOOD = SIG + ihdr() + IDAT + IEND


def image(w=1, h=1, bd=8, ct=2):
    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}[ct]
    data = zlib.compress((b"\0" + bytes((w * channels * bd + 7) // 8)) * h)
    palette = chunk(b"PLTE", b"\0\0\0") if ct == 3 else b""
    return SIG + ihdr(w, h, bd, ct) + palette + chunk(b"IDAT", data) + IEND


def entry(data=GOOD, path=PNGP, w=1, h=1, **kw):
    e = {"id": "shot-one", "path": path, "sha256": hashlib.sha256(data).hexdigest(),
         "bytes": len(data), "width": w, "height": h, "captured_on": "2026-01-02",
         "target": "native-x86_64", "caption": "Tiny synthetic capture", "alt": "One pixel",
         "source_revision": "a" * 40, "compiled_source_revision": "b" * 40, "evidence": EVID,
         "license": "GPL-3.0-or-later", "rights": "Original generated fixture",
         "input": "synthetic", "limitations": []}
    e.update(kw)
    return e


def man(*entries, **kw):
    m = {"schema_version": 1, "screenshots": list(entries)}
    m.update(kw)
    return json.dumps(m).encode()


class Base(unittest.TestCase):
    def repo(self, files, stage=True):
        parent = os.path.realpath(os.environ.get("TMP") or tempfile.gettempdir())
        d = tempfile.mkdtemp(prefix="shot-", dir=parent)
        resolved = os.path.realpath(d)
        if resolved == parent or os.path.commonpath((parent, resolved)) != parent:
            raise RuntimeError("Fixture cleanup target leaves its explicit temp root")
        self.addCleanup(shutil.rmtree, d, True)
        subprocess.run(["git", "init", "-q", d], check=True, capture_output=True)
        self.write(d, files)
        if stage and files:
            subprocess.run(["git", "-C", d, "add", "-A", "--", "."], check=True, capture_output=True)
        return d

    def write(self, d, files):
        for p, b in files.items():
            fp = os.path.join(d, *p.split("/"))
            os.makedirs(os.path.dirname(fp), exist_ok=True)
            with open(fp, "wb") as f:
                f.write(b)

    def run_v(self, d):
        r = subprocess.run([sys.executable, "-B", VALIDATOR, "--root", d],
                           capture_output=True, timeout=120)
        return r.returncode, (r.stdout + r.stderr).decode("utf-8", "replace")[-400:]

    def check(self, files, ok, msg=""):
        d = self.repo(files)
        rc, out = self.run_v(d)
        self.assertEqual(rc == 0, ok, "%s rc=%s out=%s" % (msg, rc, out))

    def base(self, data=GOOD, m=None, **kw):
        f = {PNGP: data, EVID: b"# note\n", MAN: m if m is not None else man(entry(data, **kw))}
        return f


class Admission(Base):
    def test_valid_capture_accepted(self):
        self.check(self.base(), True)
        for bd, ct in ((1, 0), (16, 0), (8, 3), (8, 4), (16, 6), (4, 3)):
            d = image(bd=bd, ct=ct)
            self.check(self.base(d), True, "depth %s type %s" % (bd, ct))
        data = zlib.compress(b"\0\0\0\0")
        d = SIG + ihdr(il=1) + chunk(b"IDAT", data[:4]) + chunk(b"IDAT", data[4:]) + IEND
        self.check(self.base(d), True, "interlaced, consecutive IDAT")
        d = SIG + ihdr() + chunk(b"tEXt", b"k\0v") + IDAT + IEND
        self.check(self.base(d), True, "ancillary chunk")

    def test_legacy_and_empty(self):
        self.check({"README.md": b"x"}, True, "absent manifest no PNG")
        self.check({MAN: man()}, True, "empty manifest")
        self.check({"a.txt": b"x"}, True, "no screenshots")

    def test_registration(self):
        self.check({PNGP: GOOD, EVID: b"x"}, False, "PNG without manifest")
        self.check({PNGP: GOOD, EVID: b"x", MAN: man()}, False, "unlisted PNG")
        f = self.base()
        f[DIR + "2026-01-02/other.png"] = GOOD
        self.check(f, False, "second PNG unlisted")
        f = self.base()
        del f[PNGP]
        self.check(f, False, "listed but not indexed")
        self.check({PNGP: GOOD, EVID: b"x", MAN: man(entry(), entry(id="x", path=DIR + "2026-01-02/x.png"))},
                   False, "second entry missing file")
        self.check({"x/other.PNG": GOOD, MAN: man()}, False, "stray uppercase-extension PNG")

    def test_duplicates(self):
        p2 = DIR + "2026-01-02/shot-two.png"
        e1, e2 = entry(), entry(path=p2)
        f = {PNGP: GOOD, p2: GOOD, EVID: b"x", MAN: man(e1, e2)}
        self.check(f, False, "duplicate id")
        f[MAN] = man(entry(), entry(id="shot-two"))
        self.check(f, False, "duplicate path")
        f[MAN] = man(entry(), entry(id="shot-two", path=p2))
        self.check(f, True, "distinct entries control")

    def test_mismatched_facts(self):
        other = image(w=2, h=3)
        cases = {"hash": entry(sha256="0" * 64), "bytes": entry(bytes=len(GOOD) + 1),
                 "width": entry(w=2), "height": entry(h=2),
                 "png2x3 declared 1x1": entry(other)}
        for name, e in cases.items():
            self.check({PNGP: other if name.startswith("png") else GOOD, EVID: b"x", MAN: man(e)}, False, name)
        self.check({PNGP: other, EVID: b"x", MAN: man(entry(other, w=2, h=3))}, True, "2x3 control")

    def test_malformed_png(self):
        a = SIG + ihdr()
        bad = {
            "empty": b"", "truncated": GOOD[:-5], "signature": b"\x88" + GOOD[1:],
            "crc": SIG + chunk(b"IHDR", struct.pack(">IIBBBBB", 1, 1, 8, 2, 0, 0, 0), crc=1) + IDAT + IEND,
            "idat crc": a + chunk(b"IDAT", b"x", crc=0) + IEND,
            "trailing": GOOD + b"\0", "no iend": a + IDAT, "no idat": a + IEND,
            "iend data": a + IDAT + chunk(b"IEND", b"\0"), "two iend": a + IDAT + IEND + IEND,
            "ihdr not first": SIG + chunk(b"tEXt", b"k\0v") + ihdr()[0:] + IDAT + IEND,
            "idat before ihdr": SIG + IDAT + ihdr() + IEND,
            "short ihdr": SIG + chunk(b"IHDR", b"\0" * 12) + IDAT + IEND,
            "long ihdr": SIG + chunk(b"IHDR", struct.pack(">IIBBBBB", 1, 1, 8, 2, 0, 0, 0) + b"\0") + IDAT + IEND,
            "nonconsecutive idat": a + IDAT + chunk(b"tEXt", b"k\0v") + IDAT + IEND,
            "unknown critical": a + chunk(b"ABCD", b"") + IDAT + IEND,
            "plte after idat": SIG + ihdr(ct=3) + IDAT + chunk(b"PLTE", b"\0\0\0") + IEND,
            "length past end": a + struct.pack(">I", 9999) + b"IDAT" + b"x",
            "bad pair": SIG + ihdr(bd=8, ct=1) + IDAT + IEND, "bad pair2": SIG + ihdr(bd=4, ct=2) + IDAT + IEND,
            "bad depth": SIG + ihdr(bd=3) + IDAT + IEND, "compression": SIG + ihdr(cm=1) + IDAT + IEND,
            "filter": SIG + ihdr(fl=1) + IDAT + IEND, "interlace": SIG + ihdr(il=2) + IDAT + IEND,
            "zero width": SIG + ihdr(w=0) + IDAT + IEND,
            "text only": b"not a png",
        }
        for name, data in bad.items():
            with self.subTest(name):
                self.check(self.base(data, w=1, h=1), False, name)

    def test_paths(self):
        for p in (DIR + "2026-01-02/shot-one.PNG", DIR + "2026-01-02/Shot.png", DIR + "2026-02-30/shot.png",
                  DIR + "2026-13-01/shot.png", DIR + "2026-1-02/shot.png", DIR + "2026-01-02/sh_ot.png",
                  DIR + "2026-01-02/shot--one.png", DIR + "2026-01-02/-shot.png", DIR + "2026-01-02/shot-.png",
                  DIR + "2026-01-02/sub/shot.png", DIR + "2026-01-02/shöt.png", DIR + "shot.png",
                  "docs/screenshots/2026-01-02/shot.png", DIR + "2026-01-02/shot.png.png ".strip(),
                  DIR + "2026-01-02/shot.jpg"):
            with self.subTest(p):
                self.check({p: GOOD, EVID: b"x", MAN: man(entry(path=p))}, False, p)
        for p in (DIR + "2026-01-02/../2026-01-02/shot.png", DIR + "..\\x.png",
                  DIR + "2026-01-02\\shot.png", "/" + PNGP, "C:/x.png", DIR + "2026-01-02/./shot.png"):
            with self.subTest(p):
                f = {PNGP: GOOD, EVID: b"x", MAN: man(entry(path=p))}
                self.check(f, False, p)
        self.check({DIR + "2026-02-29/a.png": GOOD, EVID: b"x",
                    MAN: man(entry(path=DIR + "2026-02-29/a.png", captured_on="2026-02-29"))}, False, "non-leap")
        self.check({DIR + "2028-02-29/a.png": GOOD, EVID: b"x",
                    MAN: man(entry(path=DIR + "2028-02-29/a.png", captured_on="2028-02-29"))}, True, "leap control")

    def test_metadata_rules(self):
        mut = {
            "license": {"license": "MIT"}, "license case": {"license": "gpl-3.0-or-later"},
            "id upper": {"id": "Shot"}, "id underscore": {"id": "a_b"}, "id empty": {"id": ""},
            "bool bytes": {"bytes": True}, "float bytes": {"bytes": float(len(GOOD))},
            "str bytes": {"bytes": str(len(GOOD))}, "zero bytes": {"bytes": 0},
            "huge bytes": {"bytes": 1048577}, "bool width": {"width": True}, "float width": {"width": 1.0},
            "hash upper": {"sha256": hashlib.sha256(GOOD).hexdigest().upper()},
            "hash short": {"sha256": "a" * 63}, "target": {"target": "native-arm64"},
            "target case": {"target": "Native-x86_64"}, "date fmt": {"captured_on": "2026-1-2"},
            "date bad": {"captured_on": "2026-02-30"}, "date mismatch": {"captured_on": "2026-01-03"},
            "rev short": {"source_revision": "a" * 39}, "rev upper": {"compiled_source_revision": "A" * 40},
            "rev long": {"source_revision": "a" * 41}, "rev nonhex": {"source_revision": "g" * 40},
            "input": {"input": "automatic"}, "caption empty": {"caption": ""}, "alt blank": {"alt": ""},
            "rights empty": {"rights": ""}, "caption type": {"caption": 5},
            "lim type": {"limitations": "none"}, "lim empty item": {"limitations": [""]},
            "lim nonstr": {"limitations": [1]}, "ev absolute": {"evidence": "/docs/evidence/note.md"},
            "ev traversal": {"evidence": "docs/../docs/evidence/note.md"},
            "ev backslash": {"evidence": "docs\\evidence\\note.md"}, "ev not md": {"evidence": "docs/evidence/note.txt"},
            "ev missing": {"evidence": "docs/evidence/absent.md"},
            "dimension product": {"width": 4097},
        }
        for name, ch in mut.items():
            with self.subTest(name):
                f = {PNGP: GOOD, EVID: b"x", "docs/evidence/note.txt": b"x", MAN: man(entry(**ch))}
                self.check(f, False, name)
        for k in entry():
            with self.subTest("missing " + k):
                e = entry()
                del e[k]
                self.check({PNGP: GOOD, EVID: b"x", MAN: man(e)}, False, "missing " + k)
        for name, ch in {"input none": {"input": "none"}, "human": {"input": "human-confirmed"},
                         "mac": {"target": "native-mac68000"}, "lims": {"limitations": ["a", "b"]}}.items():
            self.check({PNGP: GOOD, EVID: b"x", MAN: man(entry(**ch))}, True, name)
        for name, m in {"schema bool": man(entry(), schema_version=True), "schema 2": man(entry(), schema_version=2),
                        "schema float": man(entry(), schema_version=1.5),
                        "not array": b'{"schema_version":1,"screenshots":{}}',
                        "no schema": b'{"screenshots":[]}', "no list": b'{"schema_version":1}',
                        "top array": b"[]", "entry not object": b'{"schema_version":1,"screenshots":[1]}'}.items():
            with self.subTest(name):
                self.check({PNGP: GOOD, EVID: b"x", MAN: m}, False, name)

    def test_json_strictness(self):
        e = json.dumps(entry())
        dupe = '{"schema_version":1,"schema_version":1,"screenshots":[%s]}' % e
        dup_entry = '{"schema_version":1,"screenshots":[%s]}' % e.replace('"alt"', '"caption": "x", "alt"', 1)
        for name, m in {"dup top key": dupe.encode(), "dup entry key": dup_entry.encode(),
                        "invalid": b"{", "empty": b"", "garbage": b"\xff\xfe"}.items():
            with self.subTest(name):
                self.check({PNGP: GOOD, EVID: b"x", MAN: m}, False, name)

    def test_index_is_authoritative(self):
        d = self.repo(self.base())
        self.write(d, {PNGP: b"corrupt"})
        self.assertEqual(self.run_v(d)[0], 0, "worktree corruption must be ignored")
        d = self.repo(self.base(b"corrupt", m=man(entry())))
        self.write(d, {PNGP: GOOD})
        self.assertNotEqual(self.run_v(d)[0], 0, "staged corruption must fail")
        d = self.repo(self.base())
        self.write(d, {PNGP: GOOD, DIR + "2026-01-02/extra.png": GOOD})
        self.assertEqual(self.run_v(d)[0], 0, "untracked worktree PNG ignored")
        d = self.repo(self.base())
        os.remove(os.path.join(d, *PNGP.split("/")))
        self.assertEqual(self.run_v(d)[0], 0, "worktree deletion ignored")
        d = self.repo(self.base(), stage=True)
        self.write(d, {EVID: b""})
        os.remove(os.path.join(d, *EVID.split("/")))
        self.assertEqual(self.run_v(d)[0], 0, "staged evidence doc suffices")

    def test_manifest_staged_vs_worktree(self):
        d = self.repo(self.base())
        self.write(d, {MAN: b"{"})
        self.assertEqual(self.run_v(d)[0], 0, "worktree manifest corruption ignored")
        d = self.repo(self.base(m=man()))
        self.write(d, {MAN: man(entry())})
        self.assertNotEqual(self.run_v(d)[0], 0, "staged manifest omits PNG; worktree self-registration ignored")
        d = self.repo({PNGP: GOOD, EVID: b"x"})
        self.write(d, {MAN: man(entry())})
        self.assertNotEqual(self.run_v(d)[0], 0, "unstaged manifest cannot register")
        d = self.repo(self.base(m=b"{"))
        self.write(d, {MAN: man(entry())})
        self.assertNotEqual(self.run_v(d)[0], 0, "staged invalid manifest fails")

    def test_no_mutation(self):
        d = self.repo(self.base())
        before = subprocess.run(["git", "-C", d, "status", "--porcelain"], capture_output=True).stdout
        self.assertEqual(self.run_v(d)[0], 0)
        after = subprocess.run(["git", "-C", d, "status", "--porcelain"], capture_output=True).stdout
        self.assertEqual(before, after)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--validator")
    a, rest = ap.parse_known_args()
    if a.validator:
        VALIDATOR = os.path.abspath(a.validator)
    unittest.main(argv=[sys.argv[0]] + rest)
