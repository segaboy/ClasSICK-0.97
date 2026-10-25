# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 Dean Howell. License: GPL-3.0-or-later.
"""One-shot bounded framebuffer observation phase. Observes and pauses only."""
import argparse, datetime, hashlib, json, os, re, subprocess, sys, time

VBOX_SHA256 = "7fda8e54157eb94a5ebd9cea079a9fb8c23ade92ae71e13399dc3431c021a228"
UUID_RE = re.compile(r"^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$")
MAX_IMAGES, INTERVAL = 140, 0.35


class PhaseError(Exception):
    pass


def _link(p):
    isj = getattr(os.path, "isjunction", lambda _p: False)
    return os.path.islink(p) or isj(p)


def _clean_chain(path):
    p = os.path.abspath(path)
    while True:
        if os.path.lexists(p) and _link(p):
            return False
        parent = os.path.dirname(p)
        if parent == p:
            return True
        p = parent


def _inside(path, root):
    try:
        return os.path.commonpath([os.path.realpath(path), os.path.realpath(root)]) == os.path.realpath(root)
    except ValueError:
        return False


def validate(record, output, seconds):
    if not isinstance(record, dict):
        raise PhaseError("record must be an object")
    uuid, name, root = (record.get(k) for k in ("uuid", "name", "trial_root"))
    if not (isinstance(uuid, str) and UUID_RE.fullmatch(uuid)):
        raise PhaseError("bad uuid")
    if not (isinstance(name, str) and name.startswith("ClasSICK-097-")):
        raise PhaseError("bad name")
    if not (isinstance(root, str) and os.path.isabs(root) and os.path.isdir(root) and _clean_chain(root)):
        raise PhaseError("bad trial_root")
    if type(seconds) is not int or not 5 <= seconds <= 55:
        raise PhaseError("seconds must be int 5..55")
    if not (isinstance(output, str) and os.path.isabs(output)):
        raise PhaseError("output must be absolute")
    parent = os.path.dirname(output)
    if os.path.lexists(output) or not os.path.isdir(parent) or not _clean_chain(parent):
        raise PhaseError("bad output or parent")
    if not _inside(output, root):
        raise PhaseError("output outside trial_root")


def _ident(info, record):
    return info.get("uuid") == record["uuid"] and info.get("name") == record["name"]


def _cfg(info, root):
    c = info.get("cfg_file")
    ok = (isinstance(c, str) and c.lower().endswith(".vbox") and os.path.isfile(c)
          and _clean_chain(c) and _inside(c, root))
    return os.path.realpath(c) if ok else None


def _utc():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def _cleanup(backend, record, pinned):
    try:
        info = backend.info(record["uuid"])
        if not _ident(info, record) or _cfg(info, record["trial_root"]) != pinned:
            return "failed: identity changed"
        state = info.get("state")
        if state == "running":
            backend.pause(record["uuid"])
            after = backend.info(record["uuid"])
            if not _ident(after, record) or _cfg(after, record["trial_root"]) != pinned:
                return "failed: identity changed after pause"
            return {"paused": "paused", "poweroff": "poweroff_after_pause"}.get(
                after.get("state"), "failed: pause not observed")
        return {"paused": "already_paused", "poweroff": "poweroff_no_action"}.get(state, "failed: unknown state")
    except Exception as e:
        return "failed: %s" % e


def run_phase(record, output, seconds, backend, clock=time.monotonic, sleep=time.sleep):
    validate(record, output, seconds)
    info = backend.info(record["uuid"])
    if not _ident(info, record):
        raise PhaseError("identity mismatch")
    pinned = _cfg(info, record["trial_root"])
    if pinned is None:
        raise PhaseError("bad cfg_file")
    if info.get("state") != "running":
        raise PhaseError("VM is not running")
    made_output = False
    m = {"uuid": record["uuid"], "name": record["name"], "planned_cap_seconds": seconds,
         "started_utc": _utc(), "captures": [], "phase_status": "ok", "error": None}
    start = None
    try:
        os.mkdir(output)
        made_output = True
        start = clock()
        deadline = start + seconds
        while len(m["captures"]) < MAX_IMAGES and clock() < deadline:
            path = os.path.join(output, "frame_%03d.png" % len(m["captures"]))
            backend.snapshot(record["uuid"], path)
            m["captures"].append({"file": os.path.basename(path), "utc": _utc(), "host_elapsed": clock() - start})
            sleep(INTERVAL)
    except Exception as e:
        m["phase_status"], m["error"] = "capture_failed", repr(e)
    finally:
        m["cleanup"] = _cleanup(backend, record, pinned)
        m["ended_utc"] = _utc()
        m["host_elapsed_total"] = clock() - start if start is not None else None
        m["capture_count"] = len(m["captures"])
        m["exit_code"] = 0 if m["phase_status"] == "ok" and m["capture_count"] and not m["cleanup"].startswith("failed") else 1
        if made_output:
            try:
                with open(os.path.join(output, "manifest.json"), "x", encoding="utf-8") as f:
                    json.dump(m, f, indent=2)
            except OSError as e:
                m["manifest_error"], m["exit_code"] = repr(e), 1
    return m


class VBox:
    def __init__(self):
        base = os.environ.get("ProgramFiles", r"C:\Program Files")
        self.exe = os.path.join(base, "Oracle", "VirtualBox", "VBoxManage.exe")

    def _run(self, args):
        with open(self.exe, "rb") as f:
            if hashlib.sha256(f.read()).hexdigest() != VBOX_SHA256:
                raise PhaseError("VBoxManage hash mismatch")
        r = subprocess.run([self.exe] + args, capture_output=True, text=True, timeout=5, shell=False)
        if r.returncode:
            raise PhaseError("VBoxManage failed: %s" % r.stderr.strip()[:200])
        return r.stdout

    def info(self, uuid):
        keys = {"name": "name", "UUID": "uuid", "VMState": "state", "CfgFile": "cfg_file"}
        out = {}
        for line in self._run(["showvminfo", uuid, "--machinereadable"]).splitlines():
            k, _, v = line.partition("=")
            if k in keys and v.startswith('"'):
                out[keys[k]] = json.loads(v)
        return out

    def snapshot(self, uuid, path):
        self._run(["controlvm", uuid, "screenshotpng", path])

    def pause(self, uuid):
        self._run(["controlvm", uuid, "pause"])


def main(argv=None, backend_factory=VBox):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--record", required=True)
    ap.add_argument("--output", required=True)
    ap.add_argument("--seconds", type=int, default=55)
    ap.add_argument("--execute", action="store_true")
    a = ap.parse_args(argv)
    try:
        with open(a.record, encoding="utf-8-sig") as f:
            record = json.load(f)
        validate(record, a.output, a.seconds)
        if not a.execute:
            print(json.dumps({"dry_run": True, "vm": record["name"], "output": a.output, "seconds": a.seconds}))
            return 0
        m = run_phase(record, a.output, a.seconds, backend_factory())
    except (PhaseError, OSError, ValueError) as e:
        print("refused: %s" % e, file=sys.stderr)
        return 2
    print(json.dumps({k: m[k] for k in ("phase_status", "error", "cleanup", "capture_count", "host_elapsed_total")}))
    return m["exit_code"]


if __name__ == "__main__":
    sys.exit(main())
