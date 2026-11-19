#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Run a CODE fixture test with freshly generated original MFS media."""
import argparse
from pathlib import Path
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from code_stream_fixture import original_media


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("suite", choices=("tables", "row"))
    parser.add_argument("executable", type=Path)
    args = parser.parse_args()
    sizes = [(180, 20)]
    if args.suite == "tables":
        sizes.extend([(8192, 64), (65, 65)])
    with tempfile.TemporaryDirectory(prefix="classick-code-stream-") as directory:
        media = []
        for rows, segments in sizes:
            path = Path(directory) / f"rows-{rows}-segments-{segments}.raw"
            path.write_bytes(original_media(rows, segments)[0])
            media.append(path)
        return subprocess.run([args.executable.resolve(strict=True), *media], timeout=120).returncode


if __name__ == "__main__":
    sys.exit(main())
