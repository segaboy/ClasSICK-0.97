#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Run the File Manager test on a fresh, project-authored fragmented volume."""
from pathlib import Path
from runpy import run_path
import subprocess
import sys
from tempfile import TemporaryDirectory


def main():
    executable = Path(sys.argv[1]).resolve(strict=True)
    generator = Path(__file__).resolve().parents[1] / "scripts" / "Test-MfsVolumeMedia.py"
    original, _ = run_path(str(generator))["original_media"]()
    with TemporaryDirectory(prefix="classick-file-io-") as directory:
        disk = Path(directory) / "generated.raw"
        disk.write_bytes(original)
        result = subprocess.run([executable, disk], timeout=60)
        if disk.read_bytes() != original:
            raise RuntimeError("file service modified its host input")
        return result.returncode


if __name__ == "__main__":
    sys.exit(main())
