#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Run the startup-file host contract with its generated original fixture."""
import argparse
from pathlib import Path
import subprocess
import sys
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from startup_files_fixture import original_media

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('executable',type=Path)
args=parser.parse_args()
with tempfile.TemporaryDirectory(prefix='classick-startup-files-') as directory:
    path=Path(directory)/'original.dsk'
    path.write_bytes(original_media()[0])
    result=subprocess.run([str(args.executable.resolve(strict=True)),str(path)],timeout=60)
    sys.exit(result.returncode)
