# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Verify the exact experimental Mac/286 release package, without extracting it.

Checks identities and internal consistency; this is not a signature, a new
qualification result, or proof of the owner's authorization outside the receipt.
"""
import argparse
import hashlib
import io
import json
import os
from pathlib import Path, PurePosixPath
import re
import stat
import zipfile

VERSIONS = {"rom": "0.4.0-alpha.1", "pc": "0.2.0-alpha.1"}
REVIEW_SHA = "b35eb298494a8a40946cf1fb469d21d8b12911dd0529329bdd4b6f931dea275f"
INVENTORY_SHA = "acf331bfa1a6acd3ad27ded5296967f4adc52f824a8c6e4621be47d7b88437db"
LICENSE_SHA = "3972dc9744f6499f0f9b2dbf76696f2ae7ad8af9b23dde66d6af86c9dfb36986"
IMAGE_NAMES = {"mac/classick.rom", "pc286/hard-disk.img", "pc286/install-floppy.img"}
TOP_LEVEL = {"COPYING", "COPYRIGHT.md", "README.md", "TRYING.md", "TRYING.txt",
             "RELEASE.json", "SHA256SUMS", "source.zip", "build-source.zip", "verify-package.py"}
PREFIXES = {"mac", "pc286", "font", "media", "port-media", "evidence"}
TEXT_EVIDENCE_SUFFIXES = {".json", ".md", ".txt", ".xml", ".csv", ".py", ".bhmouse"}
HISTORICAL_METADATA = {"docs/development/cursor-byte-composition-20261116.md",
                       "releases/paired-native-preview-2-mac286.json"}
HEX = re.compile(r"[0-9a-f]{64}\Z")


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def safe_name(name):
    """Require portable, unambiguous relative file names, including on Windows."""
    if not isinstance(name, str) or not name or "\\" in name or ":" in name:
        raise ValueError("unsafe member name")
    parts = name.split("/")
    reserved = {"CON", "PRN", "AUX", "NUL"} | {f"{p}{n}" for p in ("COM", "LPT") for n in range(1, 10)}
    if (PurePosixPath(name).is_absolute() or
        any(p in ("", ".", "..") or p.endswith((".", " ")) or
            p.split(".")[0].upper() in reserved or
            any(ord(c) < 32 or c in '<>"|?*' for c in p) for p in parts)):
        raise ValueError("unsafe member: " + repr(name))
    return name


def unique_names(names):
    folded = set()
    for name in names:
        safe_name(name)
        key = name.casefold()
        if key in folded:
            raise ValueError("duplicate or case-colliding member: " + name)
        folded.add(key)
    for name in names:
        if any(str(p).casefold() in folded for p in PurePosixPath(name).parents if str(p) != "."):
            raise ValueError("file/directory collision: " + name)


def zip_files(archive, allow_directories=False):
    names, seen = [], set()
    for item in archive.infolist():
        directory = item.is_dir()
        name = item.filename[:-1] if directory else item.filename
        safe_name(name)
        if name.casefold() in seen:
            raise ValueError("duplicate ZIP entry: " + name)
        seen.add(name.casefold())
        kind = stat.S_IFMT(item.external_attr >> 16)
        if item.flag_bits & 1 or kind not in (0, stat.S_IFREG, stat.S_IFDIR):
            raise ValueError("encrypted or nonregular ZIP entry: " + name)
        if directory:
            if not allow_directories or kind == stat.S_IFREG:
                raise ValueError("unexpected ZIP directory: " + name)
        else:
            if kind == stat.S_IFDIR:
                raise ValueError("incorrect ZIP entry type: " + name)
            names.append(name)
    unique_names(names)
    return names


def directory_files(path):
    if path.is_symlink() or getattr(path, "is_junction", lambda: False)() or not path.is_dir():
        raise ValueError("package must be a regular directory or ZIP")
    names = []
    for current, directories, files in os.walk(path, followlinks=False):
        for item in directories + files:
            target = Path(current) / item
            if target.is_symlink() or getattr(target, "is_junction", lambda: False)():
                raise ValueError("linked package member: " + str(target))
        names.extend((Path(current) / item).relative_to(path).as_posix() for item in files)
    unique_names(names)
    return names


def load_json(raw):
    def pairs(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ValueError("duplicate JSON key: " + key)
            result[key] = value
        return result
    return json.loads(raw, object_pairs_hook=pairs)


def inventory(raw):
    rows = {}
    for line in raw.decode("ascii").splitlines():
        digest, name = line.split("  ", 1)
        safe_name(name)
        if name in rows or not HEX.fullmatch(digest):
            raise ValueError("invalid checksum inventory")
        rows[name] = digest
    unique_names(list(rows))
    return rows


def metadata(record):
    if (record.get("schema_version") != 1 or record.get("status") != "experimental-release" or
        record.get("authorization") != "authorized" or record.get("component_versions") != VERSIONS or
        not isinstance(record.get("owner_authorization"), dict) or not record["owner_authorization"] or
        not isinstance(record.get("acceptance"), dict) or not record["acceptance"] or
        not isinstance(record.get("limitations"), list) or not record["limitations"] or
        not isinstance(record.get("scope"), str) or not record["scope"].strip()):
        raise ValueError("invalid experimental release authorization or scope")
    if any(key in record for key in ("ready_for_release", "published", "owner_disposition_required")):
        raise ValueError("ambiguous release status; publication is recorded separately")


def image_inputs(read, reviewed_inputs):
    names = (set(load_json(read("evidence/mac-build.json"))["inputs"]) |
             set(load_json(read("evidence/pc286-session.json"))["source_sha256"]))
    if not names or not names <= set(reviewed_inputs) or names & HISTORICAL_METADATA:
        raise ValueError("invalid reviewed image-producing input set")
    return {name: reviewed_inputs[name] for name in sorted(names)}


def verify_members(names, read):
    rows = inventory(read("SHA256SUMS"))
    if set(names) != set(rows) | {"SHA256SUMS"}:
        raise ValueError("package membership differs from SHA256SUMS")
    for name in names:
        if name not in TOP_LEVEL and name.split("/")[0] not in PREFIXES:
            raise ValueError("unexpected package member: " + name)
    if not TOP_LEVEL <= set(names):
        raise ValueError("required package files missing")
    for name, digest in rows.items():
        if sha(read(name)) != digest:
            raise ValueError("changed file: " + name)
    release = load_json(read("RELEASE.json"))
    metadata(release)
    files = release["files_sha256"]
    if files != {name: digest for name, digest in rows.items() if name != "RELEASE.json"}:
        raise ValueError("release file inventory differs")
    original = read("evidence/history/review.json")
    if sha(original) != REVIEW_SHA or release["review_sha256"] != REVIEW_SHA:
        raise ValueError("wrong frozen review")
    old_sums = read("evidence/history/SHA256SUMS")
    if sha(old_sums) != INVENTORY_SHA:
        raise ValueError("wrong frozen review inventory")
    old = inventory(old_sums)
    retained = release["retained_review_files"]
    if set(retained) != set(old) or retained["source.zip"] != "build-source.zip":
        raise ValueError("incomplete frozen review retention")
    for origin, destination in retained.items():
        safe_name(destination)
        if destination not in rows or rows[destination] != old[origin]:
            raise ValueError("changed reviewed artifact: " + origin)
    generated_files = TOP_LEVEL | {"evidence/history/SHA256SUMS", "evidence/release-authorization.json"}
    for name in set(names) - set(retained.values()) - generated_files:
        if (not name.startswith("evidence/") or name.startswith("evidence/history/") or
            PurePosixPath(name).suffix.lower() not in TEXT_EVIDENCE_SUFFIXES):
            raise ValueError("unadmitted supplemental member: " + name)
        raw = read(name)
        raw.decode("utf-8")
        if b"\0" in raw:
            raise ValueError("binary supplemental evidence: " + name)
    review = load_json(original)
    candidate = load_json(read("evidence/history/candidate.json"))
    if release["images"] != candidate["images"] or set(release["images"]) != IMAGE_NAMES:
        raise ValueError("wrong released image set")
    for name, expected in release["images"].items():
        raw = read(name)
        if len(raw) != expected["bytes"] or sha(raw) != expected["sha256"]:
            raise ValueError("image mismatch: " + name)
    if release["compiled_source_lf_sha256"] != review["compiled_source_lf_sha256"]:
        raise ValueError("compiled input map differs from frozen review")
    current_inputs = image_inputs(read, review["compiled_source_lf_sha256"])
    if release["current_image_input_lf_sha256"] != current_inputs:
        raise ValueError("current image input map differs from build records")
    build_source_raw = read("build-source.zip")
    build_digest = sha(build_source_raw)
    if build_digest != review["source_archive_sha256"] or release["build_source_archive_sha256"] != build_digest:
        raise ValueError("build source archive differs from frozen review")
    if release["build_source_revision"] != review["source_revision"]:
        raise ValueError("build source revision differs from frozen review")
    with zipfile.ZipFile(io.BytesIO(build_source_raw)) as build_source:
        zip_files(build_source, allow_directories=True)
        if build_source.comment.decode("ascii") != release["build_source_revision"]:
            raise ValueError("build source Git revision mismatch")
        for name, digest in review["compiled_source_lf_sha256"].items():
            if sha(build_source.read(name).replace(b"\r\n", b"\n")) != digest:
                raise ValueError("frozen build source mismatch: " + name)
        if build_source.read("LICENSE") != read("COPYING"):
            raise ValueError("build source/license mismatch")
    if sha(read("COPYING")) != LICENSE_SHA:
        raise ValueError("missing or changed complete GPL license")
    if read("TRYING.md") != read("TRYING.txt"):
        raise ValueError("instruction copies differ")
    for name in ("README.md", "TRYING.md"):
        body = read(name).decode("utf-8").casefold()
        if "experimental" not in body or any(marker in body for marker in (
                "release review package", "release approval pending", "owner disposition pending",
                "publication approval pending", "proposed versions remain unpublished")):
            raise ValueError("stale or missing final release instructions: " + name)
    authorization = load_json(read("evidence/release-authorization.json"))
    metadata(authorization)
    if any(release.get(k) != v for k, v in authorization.items()):
        raise ValueError("release differs from supplied authorization")
    source_raw = read("source.zip")
    if sha(source_raw) != release["source_archive_sha256"]:
        raise ValueError("source archive hash mismatch")
    with zipfile.ZipFile(io.BytesIO(source_raw)) as source:
        source_names = zip_files(source, allow_directories=True)
        revision = release["source_revision"]
        if not re.fullmatch(r"[0-9a-f]{40}", revision) or source.comment.decode("ascii") != revision:
            raise ValueError("source revision differs from Git archive")
        source_hashes = release["source_files_sha256"]
        if set(source_names) != set(source_hashes):
            raise ValueError("source archive membership differs")
        for name, digest in source_hashes.items():
            if sha(source.read(name)) != digest:
                raise ValueError("source file mismatch: " + name)
        for name, digest in current_inputs.items():
            if sha(source.read(name).replace(b"\r\n", b"\n")) != digest:
                raise ValueError("compiled source mismatch: " + name)
        historical_changes = {}
        for name, digest in review["compiled_source_lf_sha256"].items():
            current = sha(source.read(name).replace(b"\r\n", b"\n"))
            if current != digest:
                if name not in HISTORICAL_METADATA:
                    raise ValueError("unexplained historical input change: " + name)
                historical_changes[name] = {"build_source_lf_sha256": digest, "current_source_lf_sha256": current}
        if release["historical_metadata_changes"] != historical_changes:
            raise ValueError("historical metadata changes are not accurately declared")
        if source.read("LICENSE") != read("COPYING"):
            raise ValueError("source/license mismatch")
        if source.read("scripts/Verify-PairedExperimentalRelease.py").replace(b"\r\n", b"\n") != read("verify-package.py").replace(b"\r\n", b"\n"):
            raise ValueError("packaged verifier differs from committed source")
    return {"status": "pass", "classification": "experimental-release", "authorization": "authorized",
            "files": len(names), "frozen_inputs": len(review["compiled_source_lf_sha256"]),
            "current_image_inputs": len(current_inputs), "source_revision": revision,
            "build_source_revision": release["build_source_revision"], "component_versions": VERSIONS}


def verify(path):
    path = Path(path)
    if path.is_file():
        with zipfile.ZipFile(path) as archive:
            return verify_members(zip_files(archive), archive.read)
    return verify_members(directory_files(path), lambda name: (path / name).read_bytes())


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().package), indent=2))
