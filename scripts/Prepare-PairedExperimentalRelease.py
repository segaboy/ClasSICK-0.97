# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Package the frozen reviewed Mac/286 images as an authorized experimental release.

Only committed corresponding source is archived. No guest, compiler, network,
Git mutation, tag or publication is performed. Output must be a new external path.
"""
import argparse
from importlib.util import module_from_spec, spec_from_file_location
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]
STAMP = (2026, 11, 17, 0, 0, 0)


def module(name):
    spec = spec_from_file_location(name, ROOT / "scripts" / (name + ".py"))
    result = module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def git(*args):
    executable = os.environ.get("GIT_EXECUTABLE") or shutil.which("git")
    if not executable:
        raise ValueError("Git is required; put it on PATH or set GIT_EXECUTABLE")
    return subprocess.check_output([executable, *args], cwd=ROOT, timeout=120,
                                   creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("review-package", "release-metadata", "instructions", "extra-evidence", "output"):
        parser.add_argument("--" + name, type=Path, required=True)
    args = parser.parse_args()
    v = module("Verify-PairedExperimentalRelease")
    out = args.output.resolve()
    archive_path = out.with_suffix(".zip")
    review_path = args.review_package.resolve()
    if (out == ROOT or ROOT in out.parents or out == review_path or review_path in out.parents or
        out in review_path.parents or out == archive_path or out.exists() or archive_path.exists()):
        raise ValueError("output and its ZIP must be new external paths, separate from the review package")
    names = v.directory_files(review_path)
    module("Verify-PairedPreviewReview").verify(review_path)
    review_raw = (review_path / "review.json").read_bytes()
    if v.sha(review_raw) != v.REVIEW_SHA or v.sha((review_path / "SHA256SUMS").read_bytes()) != v.INVENTORY_SHA:
        raise ValueError("input is not the frozen cursor review package")
    review = v.load_json(review_raw)
    candidate = v.load_json((review_path / "candidate.json").read_bytes())
    supplied_raw = args.release_metadata.read_bytes()
    release = v.load_json(supplied_raw)
    v.metadata(release)
    generated = {"source_revision", "source_archive_sha256", "source_files_sha256", "compiled_source_lf_sha256",
                 "build_source_revision", "build_source_archive_sha256", "current_image_input_lf_sha256", "historical_metadata_changes",
                 "review_sha256", "images", "retained_review_files", "files_sha256"}
    if generated & set(release):
        raise ValueError("supplied metadata must not override generated identities")
    instructions = args.instructions.read_bytes()
    extras = v.load_json(args.extra_evidence.read_bytes())
    if not isinstance(extras, dict):
        raise ValueError("extra evidence must map relative destinations to source paths")
    head = git("rev-parse", "HEAD").decode("ascii").strip()
    source_raw = git("archive", "--format=zip", head)
    current_inputs = v.image_inputs(lambda name: (review_path / name).read_bytes(), review["compiled_source_lf_sha256"])
    historical_changes = {}
    with zipfile.ZipFile(io.BytesIO(source_raw)) as source:
        source_names = v.zip_files(source, allow_directories=True)
        source_hashes = {name: v.sha(source.read(name)) for name in sorted(source_names)}
        for name, digest in current_inputs.items():
            if v.sha(source.read(name).replace(b"\r\n", b"\n")) != digest:
                raise ValueError("committed input differs from reviewed build: " + name)
        for name, digest in review["compiled_source_lf_sha256"].items():
            current = v.sha(source.read(name).replace(b"\r\n", b"\n"))
            if current != digest:
                if name not in v.HISTORICAL_METADATA:
                    raise ValueError("unexplained historical input change: " + name)
                historical_changes[name] = {"build_source_lf_sha256": digest, "current_source_lf_sha256": current}
        for name in ("Prepare-PairedExperimentalRelease.py", "Verify-PairedExperimentalRelease.py"):
            if source.read("scripts/" + name).replace(b"\r\n", b"\n") != (ROOT / "scripts" / name).read_bytes().replace(b"\r\n", b"\n"):
                raise ValueError("release helper must match committed source: " + name)
        copying = source.read("LICENSE")
        verifier = source.read("scripts/Verify-PairedExperimentalRelease.py")
    if v.sha(copying) != v.LICENSE_SHA:
        raise ValueError("committed license differs from complete reviewed GPL text")
    files, retained = {}, {}
    keep_top = {"COPYING", "COPYRIGHT.md"}
    for name in sorted(names):
        destination = ("build-source.zip" if name == "source.zip" else name if name in keep_top or
                       name.split("/")[0] in v.PREFIXES else "evidence/history/" + name)
        files[destination] = (review_path / name).read_bytes()
        if name != "SHA256SUMS":
            retained[name] = destination
    # All supplemental inputs are explicitly named UTF-8 evidence, never tool binaries.
    for destination, origin in extras.items():
        v.safe_name(destination)
        if (not destination.startswith("evidence/") or destination.startswith("evidence/history/") or
            destination == "evidence/release-authorization.json" or destination in files or
            Path(destination).suffix.lower() not in v.TEXT_EVIDENCE_SUFFIXES):
            raise ValueError("unsafe or colliding evidence destination: " + destination)
        raw = Path(origin).read_bytes()
        raw.decode("utf-8")
        if b"\0" in raw:
            raise ValueError("binary supplemental evidence: " + destination)
        files[destination] = raw
    files.update({"COPYING": copying, "source.zip": source_raw, "verify-package.py": verifier,
                  "TRYING.md": instructions, "TRYING.txt": instructions,
                  "evidence/release-authorization.json": supplied_raw})
    files["README.md"] = ("# ClasSICK 0.97 experimental Mac/286 release\n\n"
        "ROM 0.4.0-alpha.1 / PC Preview 0.2.0-alpha.1.\n\n"
        "The owner authorized this bounded experimental release with the limitations\n"
        "recorded in RELEASE.json. Read TRYING.md for setup, controls and scope.\n"
        "This authorization does not establish human, physical or complete historical\n"
        "qualification. System software and the game port remain incomplete.\n\n"
        "The reviewed Mac ROM, 286 disk images, diagnostic font and media retain\n"
        "their exact identities. evidence/history preserves admission-time pending\n"
        "records; RELEASE.json supplies the current experimental release disposition.\n"
        "build-source.zip is the frozen corresponding source used to qualify the\n"
        "images; all 80 reviewed input hashes are checked there. source.zip contains\n"
        "the current committed source, release helpers and later evidence. Its\n"
        "image-producing inputs still match the frozen build records. Explicit\n"
        "old/new hashes preserve the two historically updated receipt/metadata paths.\n"
        "COPYING contains GPL version 3. Project contributions are GPL-3.0-or-later.\n"
        "No compiler, emulator binary or reference firmware is included.\n\n"
        "Verify the directory or ZIP with Python 3:\n\n"
        "    python verify-package.py --package PATH_TO_PACKAGE_OR_ZIP\n\n"
        "SHA256SUMS inventories all other files. Verification checks consistency and\n"
        "frozen artifact identities; it is not a cryptographic signature or a new trial.\n"
        "Public hosting/tag/asset identities belong to the separate publication receipt.\n").encode("utf-8")
    v.unique_names(list(files) + ["RELEASE.json", "SHA256SUMS"])
    release.update({"source_revision": head, "source_archive_sha256": v.sha(source_raw),
                    "build_source_revision": review["source_revision"],
                    "build_source_archive_sha256": review["source_archive_sha256"],
                    "current_image_input_lf_sha256": current_inputs,
                    "historical_metadata_changes": historical_changes,
                    "source_files_sha256": source_hashes,
                    "compiled_source_lf_sha256": review["compiled_source_lf_sha256"],
                    "review_sha256": v.REVIEW_SHA, "images": candidate["images"],
                    "retained_review_files": dict(sorted(retained.items())),
                    "files_sha256": {name: v.sha(raw) for name, raw in sorted(files.items())}})
    files["RELEASE.json"] = (json.dumps(release, indent=2, sort_keys=True) + "\n").encode("utf-8")
    files["SHA256SUMS"] = "".join(v.sha(raw) + "  " + name + "\n" for name, raw in sorted(files.items())).encode("ascii")
    v.verify_members(list(files), files.__getitem__)
    out.mkdir(parents=True, exist_ok=False)
    for name, raw in sorted(files.items()):
        target = out / name
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open("xb") as stream:
            stream.write(raw)
    with zipfile.ZipFile(archive_path, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, raw in sorted(files.items()):
            info = zipfile.ZipInfo(name, STAMP)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.create_system = 3
            info.external_attr = 0o100644 << 16
            archive.writestr(info, raw, compresslevel=9)
    result = v.verify(out)
    v.verify(archive_path)
    result.update({"archive": str(archive_path), "archive_sha256": v.sha(archive_path.read_bytes()),
                   "bytes": archive_path.stat().st_size})
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
