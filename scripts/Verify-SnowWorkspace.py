#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Read-only preflight for an isolated own-ROM Snow Early128K workspace."""
import argparse
import hashlib
import json
import os
import re
import sys


def unique(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError('duplicate workspace key')
        result[key] = value
    return result


def verify(workspace, expected_rom, expected_sha256):
    if not re.fullmatch(r'[0-9a-f]{64}', expected_sha256):
        raise ValueError('expected SHA-256 must be lowercase hexadecimal')
    if not os.path.isabs(expected_rom):
        raise ValueError('expected ROM path must be absolute')
    with open(workspace, encoding='utf-8-sig') as f:
        cfg = json.load(f, object_pairs_hook=unique)
    if not isinstance(cfg, dict) or not isinstance(cfg.get('rom_path'), str):
        raise ValueError('missing workspace ROM path')
    if not os.path.isabs(cfg['rom_path']) or os.path.normcase(os.path.abspath(cfg['rom_path'])) != os.path.normcase(os.path.abspath(expected_rom)):
        raise ValueError('workspace ROM path differs from expected ROM')
    init = cfg.get('init_args')
    if (not isinstance(init, dict) or cfg.get('model') != 'Early128K'
            or init.get('mouse_mode') != 'RelativeHw' or init.get('audio_disabled') is not True):
        raise ValueError('unexpected machine, mouse or audio profile')
    for field in ('shared_dir', 'pram_path', 'extension_rom_path', 'display_card_rom_path'):
        if field not in cfg or cfg[field] is not None:
            raise ValueError('workspace sharing or auxiliary image is not disabled')
    if cfg.get('floppy_images') != [] or cfg.get('scsi_targets') != ['None'] * 7:
        raise ValueError('workspace storage is not empty')
    with open(expected_rom, 'rb') as f:
        image = f.read(65537)
    if len(image) != 65536 or hashlib.sha256(image).hexdigest() != expected_sha256:
        raise ValueError('ROM size or SHA-256 mismatch')
    return {'model': 'Early128K', 'mouse_mode': 'RelativeHw', 'rom_sha256': expected_sha256,
            'workspace_path_matches': True, 'empty_storage': True,
            'scope': 'Workspace and file preflight only; emulator loading is not proven.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--workspace', required=True)
    parser.add_argument('--rom', required=True)
    parser.add_argument('--sha256', required=True)
    args = parser.parse_args()
    try:
        result = verify(args.workspace, args.rom, args.sha256)
    except (OSError, ValueError, TypeError) as exc:
        print(str(exc), file=sys.stderr)
        return 2
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == '__main__':
    sys.exit(main())
