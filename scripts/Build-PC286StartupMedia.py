# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Build the installed native 286 startup reader and its SPEC-0110 installer."""
import argparse
from importlib import import_module
import json
import os
from pathlib import Path
import re
import subprocess

media = import_module('Build-PC286PreviewMedia')
ROOT = media.ROOT
SOURCES = [
    'core/storage/block-read.c', 'core/storage/mac-boot-header.c',
    'core/storage/mfs-read.c', 'core/storage/mfs-volume.c', 'core/storage/startup-files.c',
    'formats/resourcefork/transport.c', 'formats/resourcefork/mfs-transport.c',
    'platform/pc286/boothush-floppy-read.c', 'platform/pc286/installed-startup.c',
]


def build(tool, assembler, out):
    lock = json.loads((ROOT/'scripts/pc286-toolchain-lock.json').read_text())
    for member in lock['selected_members']:
        if media.sha(tool/member['path']) != member['sha256']:
            raise ValueError('tool or header identity differs: ' + member['path'])
    if out == ROOT or ROOT in out.parents:
        raise ValueError('choose a new build directory outside the repository')
    out.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    for name in ('WCC', 'WCL', 'WLINK', 'WATCOM', 'INCLUDE', 'LIB'):
        env.pop(name, None)
    flags = ['-zq', '-2', '-ms', '-zl', '-s', '-os', '-zastd=c99', '-we',
             '-dCS_C16_REQUIRE_16BIT', '-i=' + str(tool/'h')]
    linker = tool/'binnt64/wlink.exe'
    objects = []
    for src in SOURCES:
        obj = out/(Path(src).stem + '.obj')
        subprocess.run([tool/'binnt64/wcc.exe', *flags, '-fo=' + str(obj), ROOT/src],
                       cwd=out, env=env, check=True, timeout=30)
        objects.append(str(obj))
    binary, mapfile = out/'startup.bin', out/'startup.map'
    subprocess.run([linker, 'format', 'raw', 'bin', 'option', 'start=start_', 'option',
                    'nodefaultlibs', 'option', 'map='+str(mapfile), 'name', binary,
                    'file', ','.join(objects)], cwd=out, env=env, check=True, timeout=30)
    mapping = mapfile.read_text()
    entry = re.search(r'Entry point address:\s*([0-9a-fA-F]+)', mapping)
    bss = re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)', mapping, re.M)
    state = re.search(r'^\s*([0-9a-fA-F]+)[+*]?\s+_cs_installed_startup_state\s*$', mapping, re.M)
    if not (entry and bss and state) or '.lib' in mapping.lower():
        raise ValueError('invalid native map or linked runtime library')
    base, size = (int(x, 16) for x in bss.groups())
    offset = int(state[1], 16)
    payload = binary.read_bytes()
    if not len(payload) <= base <= offset <= base+size-32:
        raise ValueError('startup state outside linked BSS')
    manifest = media.build_install_media(payload, int(entry[1], 16), base+size,
                                         assembler, linker, out/'media')
    dependencies = [*SOURCES, *(str(Path(s).with_suffix('.h')).replace('\\','/')
                                 for s in SOURCES[:-1]), 'formats/resourcefork/reader.h', 'arch/i286/c16.h',
                    'scripts/Build-PC286StartupMedia.py']
    manifest['source_sha256'].update({s: media.sha(ROOT/s) for s in dependencies})
    manifest.update(profile='installed-startup', state_address=0x10000+offset,
                    bss_start=base, bss_bytes=size)
    (out/'media.json').write_text(json.dumps(manifest, indent=2)+'\n')
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('tool-root', 'assembler', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    result = build(args.tool_root.resolve(strict=True), args.assembler.resolve(strict=True),
                   args.output.resolve())
    print(result['payload_sha256'])
