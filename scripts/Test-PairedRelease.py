#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell
"""Read-only independent auditor for the paired release package."""
import argparse, hashlib, io, json, os, re, stat, sys, zipfile

FIXED = (2026, 10, 19, 0, 0, 0)
errors = []


def err(m):
    errors.append(m)


def sha(b):
    return hashlib.sha256(b).hexdigest()


def lstat_reg(p):
    try:
        st = os.lstat(p)
    except OSError:
        return None
    return st if stat.S_ISREG(st.st_mode) else None


def read_exact(p, size, digest, label):
    st = lstat_reg(p)
    if st is None:
        err(f"{label}: missing or not a regular file")
        return None
    if st.st_size != size:
        err(f"{label}: size {st.st_size} != {size}")
        return None
    h = hashlib.sha256()
    with open(p, "rb") as f:
        data = f.read()
    h.update(data)
    if h.hexdigest() != digest:
        err(f"{label}: sha256 mismatch")
        return None
    return data


def listdir_exact(d, expected, label):
    try:
        with os.scandir(d) as it:
            ents = list(it)
    except OSError:
        err(f"{label}: cannot list")
        return False
    names = sorted(e.name for e in ents)
    if names != sorted(expected):
        err(f"{label}: entries {names} != {sorted(expected)}")
        return False
    return True


def safe_name(n):
    if not n or n.startswith("/") or "\\" in n or "\x00" in n or n.endswith("/"):
        return False
    if re.match(r"^[A-Za-z]:", n):
        return False
    return all(p not in ("", ".", "..") for p in n.split("/"))


def check_zip_names(zf, label, strict):
    infos = zf.infolist()
    names = [i.filename for i in infos]
    if len(set(names)) != len(names):
        err(f"{label}: duplicate ZIP entries")
    if len({n.lower() for n in names}) != len(set(names)):
        err(f"{label}: case-colliding ZIP entries")
    for i in infos:
        n = i.orig_filename
        directory = not strict and i.is_dir()
        if not safe_name(n[:-1] if directory else n):
            err(f"{label}: unsafe member name {n!r}")
        mode = i.external_attr >> 16
        if mode and not (stat.S_ISDIR(mode) if directory else stat.S_ISREG(mode)):
            err(f"{label}: non-regular member {n!r}")
        if i.flag_bits & 0x1:
            err(f"{label}: encrypted member {n!r}")
        if strict:
            if i.date_time != FIXED:
                err(f"{label}: timestamp {i.date_time} on {n}")
            if (mode & 0o7777) != 0o644:
                err(f"{label}: permissions {oct(mode & 0o7777)} on {n}")
            if i.compress_type != zipfile.ZIP_DEFLATED:
                err(f"{label}: not DEFLATE: {n}")
            # ZIP flags do not prove a compression level; inspect builder and compare twins.
            if i.create_system != 3 or i.extra or i.comment:
                err(f"{label}: unexpected extra/comment/system on {n}")
    if strict and names != sorted(names):
        err(f"{label}: members not sorted")
    return infos


def sums_text(items):
    return "".join(f"{sha(b)}  {n}\n" for n, b in sorted(items.items()))


LINK = re.compile(r"\]\(\s*<?([^)\s>]*)|^\s*\[[^\]]+\]:\s*(\S+)|href=[\"']([^\"']*)", re.M | re.I)


def check_links(text, label):
    for m in LINK.finditer(text):
        t = next(g for g in m.groups() if g is not None)
        if not (t.startswith("https://") or t.startswith("#")):
            err(f"{label}: relative or non-https link {t!r}")


def check_abs(text, label):
    if re.search(r"[A-Za-z]:\\|/(?:home|Users|tmp)/", text):
        err(f"{label}: absolute host path")


def within(a, b):
    a, b = os.path.normcase(os.path.realpath(a)), os.path.normcase(os.path.realpath(b))
    return a == b or a.startswith(b + os.sep)


def audit_source(path, cfg_in):
    data = read_exact(path, cfg_in["bytes"], cfg_in["sha256"], "source.zip")
    if data is None:
        return
    try:
        with zipfile.ZipFile(io.BytesIO(data)) as zf:
            check_zip_names(zf, "source.zip", False)
    except zipfile.BadZipFile:
        err("source.zip: bad ZIP")


def audit_component(key, c, cfg, inputs, pkg, docs):
    d = os.path.join(pkg, key)
    ups = [c["standalone"], c["archive"], c["source_archive"], "SHA256SUMS.txt"]
    if not listdir_exact(d, ups, key):
        return None
    for u in ups:
        if lstat_reg(os.path.join(d, u)) is None:
            err(f"{key}/{u}: not a regular file")
            return None
    raw = {u: open(os.path.join(d, u), "rb").read() for u in ups}
    ci = cfg["inputs"]
    bsrc = c["binary_input"]
    if raw[c["standalone"]] != inputs[bsrc]:
        err(f"{key}: standalone differs from admitted {bsrc}")
    if raw[c["source_archive"]] != inputs["source.zip"]:
        err(f"{key}: source archive differs from admitted source.zip")
    others = {u: raw[u] for u in ups[:3]}
    if raw["SHA256SUMS.txt"].decode("utf-8", "replace") != sums_text(others):
        err(f"{key}: outer SHA256SUMS.txt wrong")
    # expected demo zip members
    exp = {m: inputs[i] for m, i in c["payloads"].items()}
    exp["COPYING"] = inputs["COPYING"]
    for member, doc in c["documents"].items():
        p = os.path.join(docs, *doc.split("/"))
        if lstat_reg(p) is None or not within(p, docs):
            err(f"{key}: doc {doc} missing/unsafe")
            continue
        no_alias(p)
        b = open(p, "rb").read().replace(b"\r\n", b"\n")
        if b.startswith(b"\xef\xbb\xbf"):
            err(f"{key}: doc {doc} has BOM")
        try:
            t = b.decode("utf-8")
        except UnicodeDecodeError:
            err(f"{key}: doc {doc} not UTF-8")
            continue
        if "\r" in t or not t.endswith("\n"):
            err(f"{key}: doc {doc} stray CR or missing newline")
        if c["version"] not in t:
            err(f"{key}: {doc} lacks version {c['version']}")
        exp[member] = b
    exp["SOURCE.txt"] = (
        f"Compiled source: {cfg['compiled_source_revision']}\n"
        f"Qualification evidence: {cfg['evidence_revision']}\n"
        f"Source: https://github.com/segaboy/ClasSICK-0.97/tree/{cfg['compiled_source_revision']}\n"
        f"Evidence: https://github.com/segaboy/ClasSICK-0.97/blob/{cfg['evidence_revision']}/{cfg['evidence_path']}\n"
        f"CI: {cfg['ci_url']}\nLicense: {cfg['license']}\n").encode()
    exp["SHA256SUMS.txt"] = None
    zpath = os.path.join(d, c["archive"])
    try:
        zf = zipfile.ZipFile(zpath)
    except zipfile.BadZipFile:
        err(f"{key}: demo zip bad")
        return raw
    with zf:
        infos = check_zip_names(zf, f"{key} demo zip", True)
        names = [i.filename for i in infos]
        if sorted(names) != sorted(exp):
            err(f"{key}: demo members {sorted(names)} != {sorted(exp)}")
            return raw
        if any(i.is_dir() for i in infos):
            err(f"{key}: directory entries in demo zip")
        # validate sizes against allowlist before reading content
        for i in infos:
            if exp[i.filename] is not None and i.file_size != len(exp[i.filename]):
                err(f"{key}: {i.filename} size {i.file_size} != {len(exp[i.filename])}")
                return raw
            if i.file_size > 80 * 1024 * 1024:
                err(f"{key}: {i.filename} oversized")
                return raw
        got = {}
        for i in infos:
            got[i.filename] = zf.read(i)
    for n, b in exp.items():
        if b is not None and got[n] != b:
            err(f"{key}: member {n} content mismatch")
    inner = {n: b for n, b in got.items() if n != "SHA256SUMS.txt"}
    if got["SHA256SUMS.txt"].decode("utf-8", "replace") != sums_text(inner):
        err(f"{key}: internal SHA256SUMS.txt wrong (duplicates/self/hash)")
    for n in ("README.txt", "RELEASE.txt", "SOURCE.txt"):
        t = got[n].decode("utf-8", "replace")
        check_links(t, f"{key}/{n}")
        check_abs(t, f"{key}/{n}")
    return raw


def no_alias(path):
    path = os.path.abspath(path)
    while True:
        if os.path.islink(path) or getattr(os.path, "isjunction", lambda p: False)(path):
            err("symlink or junction in path")
        parent = os.path.dirname(path)
        if parent == path:
            break
        path = parent


TOP = {"schema_version", "compiled_source_revision", "evidence_revision", "ci_url", "license",
       "inputs", "components", "unqualified"}
V1_PATH = "docs/development/preview-session-evidence.json"
EV_RE = re.compile(r"docs/(?:[A-Za-z0-9][A-Za-z0-9_.-]*/)*[A-Za-z0-9][A-Za-z0-9_.-]*\.json")


def _nodup(pairs):
    keys = [k for k, _ in pairs]
    if len(keys) != len(set(keys)):
        raise ValueError("duplicate JSON key")
    return dict(pairs)


def load_cfg(raw):
    cfg = json.loads(raw, object_pairs_hook=_nodup)
    if type(cfg) is not dict:
        raise ValueError("not an object")
    v = cfg.get("schema_version")
    if type(v) is not int or v not in (1, 2):
        raise ValueError("schema_version must be 1 or 2")
    if set(cfg) != TOP | ({"evidence_path"} if v == 2 else set()):
        raise ValueError("unexpected top-level keys")
    if v == 2:
        p = cfg["evidence_path"]
        if not (isinstance(p, str) and p.isascii() and EV_RE.fullmatch(p)
                and not any(s.endswith(".") for s in p.split("/")[:-1])):
            raise ValueError("bad evidence_path")
    else:
        cfg["evidence_path"] = V1_PATH
    return cfg


def main():
    ap = argparse.ArgumentParser()
    for a in ("config", "inputs", "docs", "package"):
        ap.add_argument("--" + a, required=True)
    a = ap.parse_args()
    for path in (a.config, a.inputs, a.docs, a.package):
        no_alias(path)
    try:
        rawcfg = open(a.config, "rb").read()
        cfg = load_cfg(rawcfg)
        if set(cfg["components"]) != {"mac", "pc"} \
                or set(cfg["inputs"]) != {"rom.rom", "BOOTX64.EFI", "pc.img", "source.zip", "COPYING"}:
            raise ValueError("config schema/set mismatch")
    except (OSError, ValueError, KeyError) as e:
        print(json.dumps({"result": "fail", "errors": [f"config: {e}"]}))
        return 1
    for x in (a.inputs, a.docs):
        if within(a.package, x) or within(x, a.package):
            err("package overlaps inputs/docs")
    inputs = {}
    for n, m in cfg["inputs"].items():
        if n == "source.zip":
            audit_source(os.path.join(a.inputs, n), m)
            inputs[n] = read_exact(os.path.join(a.inputs, n), m["bytes"], m["sha256"], n)
        else:
            inputs[n] = read_exact(os.path.join(a.inputs, n), m["bytes"], m["sha256"], n)
    pkg = a.package
    if os.path.islink(pkg) or not os.path.isdir(pkg):
        err("package not a real directory")
    elif not errors and listdir_exact(pkg, ["mac", "pc", "review.json", "APPROVAL-PENDING.txt"], "root"):
        for k in ("mac", "pc"):
            if os.path.islink(os.path.join(pkg, k)) or not os.path.isdir(os.path.join(pkg, k)):
                err(f"{k}: not a real directory")
        review = {"schema_version": 1, "state": "approval-pending", "published": False,
                  "compiled_source_revision": cfg["compiled_source_revision"],
                  "evidence_revision": cfg["evidence_revision"], "config_sha256": sha(rawcfg),
                  "components": {}}
        if cfg["schema_version"] == 2:
            review["evidence_path"] = cfg["evidence_path"]
        if not errors:
            for k, c in cfg["components"].items():
                raw = audit_component(k, c, cfg, inputs, pkg, a.docs)
                if raw is None:
                    continue
                ups = [c["standalone"], c["archive"], c["source_archive"], "SHA256SUMS.txt"]
                review["components"][k] = {"tag": c["tag"], "version": c["version"], "assets": {
                    u: {"bytes": len(raw[u]), "sha256": sha(raw[u])} for u in ups}}
            rp = os.path.join(pkg, "review.json")
            ap_ = os.path.join(pkg, "APPROVAL-PENDING.txt")
            if lstat_reg(rp) is None or lstat_reg(ap_) is None:
                err("review.json/APPROVAL-PENDING.txt not regular")
            else:
                try:
                    rj = json.loads(open(rp, "rb").read())
                except ValueError:
                    rj = None
                if rj != review:
                    err("review.json differs from independently reconstructed manifest")
                tb = open(ap_, "rb").read().decode("utf-8", "replace")
                low = tb.lower()
                for w in ("unpublished", "approval pending", "not uploaded"):
                    if w not in low:
                        err(f"APPROVAL-PENDING.txt lacks {w!r}")
                check_abs(tb + open(rp, "rb").read().decode("utf-8", "replace"), "root markers")
    ok = not errors
    print(json.dumps({"result": "pass" if ok else "fail", "errors": errors[:50]}))
    return 0 if ok else 1


if __name__ == "__main__":
    try:
        code = main()
    except (OSError, ValueError, KeyError, TypeError, zipfile.BadZipFile, RuntimeError) as exc:
        print(json.dumps({"result": "fail", "errors": [str(exc)]}))
        code = 1
    sys.exit(code)
