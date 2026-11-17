# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Adversarial package verification checks using an already prepared release.

Mutations are isolated in memory and a temporary directory. Several controls
recompute both inventories, so they exercise frozen identities and source checks
rather than merely detecting stale checksums. Does not run either guest.
"""
import argparse
from importlib.util import module_from_spec, spec_from_file_location
import io
import json
from pathlib import Path
import tempfile
import warnings
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", type=Path, required=True)
    args = parser.parse_args()
    spec = spec_from_file_location("verify", Path(__file__).with_name("Verify-PairedExperimentalRelease.py"))
    v = module_from_spec(spec)
    spec.loader.exec_module(v)
    baseline = v.verify(args.package)
    if args.package.is_file():
        with zipfile.ZipFile(args.package) as archive:
            original = {name: archive.read(name) for name in v.zip_files(archive)}
    else:
        original = {name: (args.package / name).read_bytes() for name in v.directory_files(args.package)}
    passed = []

    def reject(label, action):
        try:
            action()
        except (ValueError, KeyError, UnicodeError, zipfile.BadZipFile):
            passed.append(label)
        else:
            raise AssertionError("negative control accepted: " + label)

    def encoded(value):
        return (json.dumps(value, sort_keys=True) + "\n").encode("utf-8")

    def reseal(files, mutate_release=None):
        release = v.load_json(files["RELEASE.json"])
        if mutate_release:
            mutate_release(release)
        release["files_sha256"] = {name: v.sha(raw) for name, raw in files.items()
                                    if name not in ("SHA256SUMS", "RELEASE.json")}
        files["RELEASE.json"] = encoded(release)
        files["SHA256SUMS"] = "".join(v.sha(raw) + "  " + name + "\n"
            for name, raw in sorted(files.items()) if name != "SHA256SUMS").encode("ascii")

    def check(files):
        v.verify_members(list(files), files.__getitem__)

    for label, member, body in (
        ("changed image with fresh inventory", "mac/classick.rom", b"wrong image"),
        ("changed reviewed evidence with fresh inventory", "evidence/mac-build.json", b"{}\n"),
        ("truncated GPL with fresh inventory", "COPYING", b"GNU GENERAL PUBLIC LICENSE\n"),
        ("pending instructions with fresh inventory", "README.md", b"experimental release approval pending\n"),
        ("unadmitted compiler binary", "mac/compiler.exe", b"MZ\0\0"),
        ("binary supplemental evidence", "evidence/new.txt", b"binary\0content"),
        ("changed frozen corresponding source", "build-source.zip", b"wrong archive"),
    ):
        files = dict(original)
        files[member] = body
        reseal(files)
        reject(label, lambda: check(files))
    for field, value in (("status", "release-review"), ("authorization", "pending")):
        files = dict(original)
        reseal(files, lambda release: release.update({field: value}))
        reject("invalid " + field, lambda: check(files))
    files = dict(original)
    reseal(files, lambda release: release.update({"historical_metadata_changes": {"fake": {}}}))
    reject("undeclared historical metadata changes", lambda: check(files))
    files = dict(original)
    reseal(files, lambda release: release.update({"current_image_input_lf_sha256": {}}))
    reject("omitted current image input map", lambda: check(files))
    files = dict(original)
    del files["font/font.json"]
    reseal(files)
    reject("missing reviewed member", lambda: check(files))
    files = dict(original)
    files["evidence/unlisted.txt"] = b"unlisted\n"
    reject("extra unlisted member", lambda: check(files))

    def changed_source(mutate, duplicate=None):
        files = dict(original)
        with zipfile.ZipFile(io.BytesIO(files["source.zip"])) as source:
            entries = {name: source.read(name) for name in v.zip_files(source, allow_directories=True)}
            comment = source.comment
        mutate(entries)
        target = io.BytesIO()
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            with zipfile.ZipFile(target, "w", compression=zipfile.ZIP_DEFLATED) as source:
                source.comment = comment
                for name, raw in entries.items():
                    source.writestr(name, raw)
                if duplicate:
                    source.writestr(duplicate, entries[duplicate])
        files["source.zip"] = target.getvalue()
        def update(release):
            release["source_archive_sha256"] = v.sha(files["source.zip"])
            release["source_files_sha256"] = {name: v.sha(raw) for name, raw in entries.items()}
        reseal(files, update)
        return files

    release = v.load_json(original["RELEASE.json"])
    compiled = next(iter(release["current_image_input_lf_sha256"]))
    for label, mutation, duplicate in (
        ("changed compiled source with fresh inventories", lambda entries: entries.update({compiled: b"changed\n"}), None),
        ("unsafe source ZIP member", lambda entries: entries.update({"../escape": b"bad"}), None),
        ("source license removed", lambda entries: entries.pop("LICENSE"), None),
        ("duplicate source ZIP member", lambda entries: None, "LICENSE"),
        ("source verifier changed", lambda entries: entries.update({"scripts/Verify-PairedExperimentalRelease.py": b"bad\n"}), None),
    ):
        files = changed_source(mutation, duplicate)
        reject(label, lambda: check(files))

    # Outer ZIP and directory entry points must independently check membership.
    with tempfile.TemporaryDirectory(prefix="classick-release-negative-") as temporary:
        path = Path(temporary) / "package.zip"
        for label, extra in (("unsafe outer ZIP member", "../escape"),
                             ("duplicate outer ZIP member", "COPYING"),
                             ("case-colliding outer ZIP member", "copying")):
            with warnings.catch_warnings():
                warnings.simplefilter("ignore", UserWarning)
                with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
                    for name, raw in original.items():
                        archive.writestr(name, raw)
                    archive.writestr(extra, b"bad")
            reject(label, lambda: v.verify(path))
        directory = Path(temporary) / "package"
        directory.mkdir()
        for name, raw in original.items():
            target = directory / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(raw)
        assert v.verify(directory)["source_revision"] == baseline["source_revision"]
        (directory / "COPYING").write_bytes(b"changed")
        reject("directory changed file", lambda: v.verify(directory))
    for unsafe in ("/absolute", "C:/drive", "a\\b", "a/../b", "a//b", "a/./b", "NUL.txt", "a. "):
        reject("unsafe portable name " + repr(unsafe), lambda: v.safe_name(unsafe))
    print(json.dumps({"status": "pass", "positive_package": baseline,
                      "negative_controls": len(passed), "rejected": passed}, indent=2))


if __name__ == "__main__":
    main()
