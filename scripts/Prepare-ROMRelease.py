#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Validate release metadata or prepare a local, unpublished own-ROM bundle.

No network, Git mutation, compilation, emulator launch or publication occurs.
Text fingerprints use Git LF bytes; ROM fingerprints use exact binary bytes.
"""

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def lf(path):
    return path.read_bytes().replace(b"\r\n", b"\n")


def local(root, name):
    """Resolve a portable repository-relative file, including symlink checks."""
    require(isinstance(name, str) and name and "\\" not in name,
            "Invalid repository-relative path")
    parts = name.split("/")
    require(not any(p in ("", ".", "..") or ":" in p for p in parts),
            "Unsafe repository-relative path")
    path = (root / name).resolve()
    require(path.is_relative_to(root.resolve()) and path.is_file(),
            f"Missing or out-of-tree file: {name}")
    return path


def encoded(value):
    return (json.dumps(value, indent=2) + "\n").encode("utf-8")


def metadata():
    catalog = json.loads(lf(ROOT / "releases/components.json"))
    require(catalog["schema_version"] == 1, "Unsupported component schema")
    rom = catalog["components"]["rom"]
    system = catalog["components"]["system"]
    require((rom["state"] == "draft" and rom["latest_published_version"] is None)
            or (rom["state"] == "prerelease" and
                rom["latest_published_version"] == rom["version"]),
            "Unsupported ROM release state")
    require(system["state"] == "unreleased" and system["version"] is None
            and system["latest_published_version"] is None,
            "Review the preparer before introducing a System release")
    version = rom["version"]
    require(re.fullmatch(r"(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)"
                         r"(?:-(?:alpha|beta|rc)\.[1-9]\d*)?", version),
            "Invalid component version")
    record = json.loads(lf(local(ROOT, rom["candidate_record"])))
    require(record["schema_version"] == 1 and record["component"] == "rom"
            and record["version"] == version and record["state"] == "draft"
            and record["published"] is False
            and record["tag_target_revision"] is None
            and record["proposed_tag"] == rom["tag_prefix"] + version,
            "Component and draft record disagree")
    require(record["release_notes"] == rom["release_notes"], "Release notes disagree")
    for name in (catalog["version_policy"], rom["release_notes"], rom["changelog"],
                 system["changelog"], "LICENSE", "COPYRIGHT.md"):
        local(ROOT, name)
    artifact = record["artifact"]
    require(artifact["filename"] == f"classick-rom-{version}-mac128k.rom"
            and artifact["bytes"] == 65536, "Unexpected ROM artifact identity")
    for revision in (record["compiled_source_revision"], record["evidence_revision"]):
        require(re.fullmatch(r"[0-9a-f]{40}", revision), "Invalid source revision")
    source_base = "https://github.com/segaboy/ClasSICK-0.97/"
    compiled = record["compiled_source_revision"]
    require(record["source_url"] == source_base + "tree/" + compiled
            and record["source_archive_url"] == source_base + "archive/" + compiled + ".tar.gz"
            and record["license"] == "GPL-3.0-or-later", "Source/license identity mismatch")
    qbytes = lf(local(ROOT, record["qualification"]["path"]))
    require(sha(qbytes) == record["qualification"]["sha256_git_lf"],
            "Qualification checksum mismatch")
    qualification = json.loads(qbytes)
    verification = qualification["verification"]
    require(qualification["compiled_source_revision"] == compiled
            and qualification["specification"] == record["specification"]
            and qualification["current_candidate_boot_observed"] is True
            and verification["rom"]["image_sha256"] == artifact["sha256"]
            and verification["rom"]["image_size"] == artifact["bytes"]
            and verification["rom_manifest"] == record["build_manifest"]
            and verification["ci_status"] == "success"
            and verification["ci_url"] == record["ci_url"]
            and qualification["boot_observation"]["rom_sha256"] == artifact["sha256"],
            "Candidate does not match its qualified image")
    return record, qbytes


def prepare(record, qbytes, args):
    image = args.rom.read_bytes()
    require(len(image) == record["artifact"]["bytes"]
            and sha(image) == record["artifact"]["sha256"], "ROM checksum/size mismatch")
    manifest_bytes = lf(args.build_manifest)
    require(len(manifest_bytes) == record["build_manifest"]["bytes"]
            and sha(manifest_bytes) == record["build_manifest"]["sha256"],
            "Build manifest checksum/size mismatch")
    manifest = json.loads(manifest_bytes)
    require(manifest["profile"] == record["profile"]
            and manifest["specification"] == record["specification"]
            and manifest["twins_equal"] is True
            and all(p["image_sha256"] == record["artifact"]["sha256"]
                    and p["image_size"] == record["artifact"]["bytes"]
                    for p in manifest["profiles"]) and manifest["profiles"],
            "Build profile/image mismatch")
    require(manifest["inputs"], "Missing native source fingerprints")
    for name, digest in manifest["inputs"].items():
        require(sha(lf(local(args.source_root, name))) == digest,
                f"Qualified source fingerprint mismatch: {name}")
    output = args.output.resolve()
    require(not args.output.exists() and not args.output.is_symlink(),
            "Output must be a fresh directory")
    require(not output.is_relative_to(ROOT) and
            not output.is_relative_to(args.source_root.resolve()),
            "Output must be outside the repository and qualified source checkout")
    notes = lf(local(ROOT, record["release_notes"]))
    notes = notes.replace(f"({record['version']}.json)".encode(), b"(RELEASE.json)")
    notes = notes.replace(b"(../../docs/development/loaded-app-redraw-evidence.md)",
                          ("(https://github.com/segaboy/ClasSICK-0.97/blob/"
                           + record["evidence_revision"]
                           + "/docs/development/loaded-app-redraw-evidence.md)").encode())
    for name in ("TRYING-0.1.0-alpha.1.md", "0.1.0-alpha.1-publication.json"):
        notes = notes.replace(f"({name})".encode(),
                              ("(https://github.com/segaboy/ClasSICK-0.97/blob/main/releases/rom/"
                               + name + ")").encode())
    notes = notes.replace(b"(../README.md)",
                          b"(https://github.com/segaboy/ClasSICK-0.97/blob/main/releases/README.md)")
    files = {
        "DRAFT.txt": b"UNPUBLISHED DRAFT. No release tag or publication is asserted.\n"
                     b"Read RELEASE-NOTES.md for scope, limitations and untested boot targets.\n",
        record["artifact"]["filename"]: image,
        "build-manifest.json": manifest_bytes,
        "qualification.json": qbytes,
        "RELEASE-NOTES.md": notes,
        "RELEASE.json": encoded({"candidate": record,
                                 "preparer_sha256_git_lf": sha(lf(Path(__file__))),
                                 "published": False}),
        "LICENSE": lf(local(ROOT, "LICENSE")),
        "COPYRIGHT.md": lf(local(ROOT, "COPYRIGHT.md")),
        "SOURCE.txt": ("ClasSICK ROM corresponding original project source and build scripts:\n"
                       + record["source_url"] + "\n" + record["source_archive_url"]
                       + "\nQualification record:\nhttps://github.com/segaboy/ClasSICK-0.97/blob/"
                       + record["evidence_revision"] + "/" + record["qualification"]["path"]
                       + "\nThis local draft includes source links, not a bundled source archive.\n"
                       "Before binary distribution, review GPL source delivery obligations.\n").encode("utf-8")
    }
    files["SHA256SUMS.txt"] = "".join(
        f"{sha(files[name])}  {name}\n" for name in sorted(files)).encode("utf-8")
    # All refusals above precede directory creation. Retain a marked partial
    # draft on an I/O error; never remove or overwrite the user's existing data.
    output.mkdir(parents=True, exist_ok=False)
    for name, data in files.items():
        with (output / name).open("xb") as stream:
            stream.write(data)
    print(f"Prepared unpublished draft: {output} ({len(files)} files)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Validate metadata only")
    parser.add_argument("--rom", type=Path)
    parser.add_argument("--build-manifest", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--source-root", type=Path, default=ROOT)
    args = parser.parse_args()
    if args.check:
        if any((args.rom, args.build_manifest, args.output)):
            parser.error("--check cannot be combined with bundle arguments")
    elif not all((args.rom, args.build_manifest, args.output)):
        parser.error("Supply --check or all of --rom, --build-manifest and --output")
    try:
        record, qbytes = metadata()
        if args.check:
            print(f"Release metadata valid: ROM {record['version']}; System unreleased")
        else:
            prepare(record, qbytes, args)
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"Release preparation refused: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
