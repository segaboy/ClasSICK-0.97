# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original guest trap/heap standalone linkage and assembler oracle; no execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sys
from importlib.util import spec_from_file_location, module_from_spec

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--tool-root', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    root = Path(args.tool_root).resolve()
    out = Path(args.output).resolve()
    out.mkdir(parents=True, exist_ok=False)
    spec = spec_from_file_location('guest_audit', repo / 'scripts/Verify-GuestM68k.py')
    helper = module_from_spec(spec)
    sys.dont_write_bytecode = True
    spec.loader.exec_module(helper)
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = str(root / 'usr/lib/x86_64-linux-gnu')
    env['PATH'] = str(root / 'usr/bin') + os.pathsep + env.get('PATH', '')
    bins = root / 'usr/bin'
    cc, nm = bins / 'm68k-linux-gnu-gcc-13', bins / 'm68k-linux-gnu-nm'
    modules = dict(helper.MODULES)
    modules.pop('apps/memory-probe/probe.c')
    modules['core/memory/manager.c'] = {'cs_guest_validate', 'cs_guest_window'}
    modules['personality/system1/memory-registers.c'] = {'cs_trap_decode', 'cs_mm_execute'}
    modules['personality/system1/memory-traps.c'] = {
        'cs_trap_get', 'cs_trap_decode', 'cs_trap_call', 'cs_trap_direct',
        'cs_mm_execute', 'cs_mm_register_call', 'cs_m68k_bus_step_options'}
    flags = ['-mcpu=68000', '-Wa,-m68000', '-std=c11', '-ffreestanding', '-fno-builtin',
             '-fno-pic', '-fno-pie', '-fno-common', '-fno-stack-protector',
             '-fno-tree-loop-distribute-patterns', '-fstack-usage', '-Wall', '-Wextra',
             '-Werror', '-Wconversion', '-Wsign-conversion', '--sysroot=' + str(root),
             '-ffile-prefix-map=' + str(repo) + '=.']
    profiles = []
    for opt in ('-O0', '-Os'):
        folder = out / opt[1:]
        folder.mkdir()
        objects, frames = [], []
        for index, (source, allowed) in enumerate(modules.items()):
            obj = folder / (str(index) + '.o')
            objects.append(obj)
            helper.run([cc, *flags, opt, '-ffile-prefix-map=' + str(folder) + '=.', '-c', repo/source, '-o', obj], env)
            helper.audit(obj, nm, allowed, env)
        combined = folder / 'original-memory-traps.o'
        helper.run([bins/'m68k-linux-gnu-ld', '-r', *objects, '-o', combined], env)
        helper.audit(combined, nm, set(), env)
        for usage in sorted(folder.glob('*.su')):
            for line in usage.read_text().splitlines():
                parts = line.split('\t')
                if len(parts) != 3 or parts[2] not in ('static', 'dynamic,bounded') or int(parts[1]) > 1024:
                    raise ValueError('Unbounded/oversized frame: ' + line)
                frames.append({'function': parts[0].split(':')[-1], 'bytes': int(parts[1])})
        profiles.append({'optimization': opt, 'objects': len(objects),
                         'combined_sha256': hashlib.sha256(combined.read_bytes()).hexdigest(), 'stack_frames': frames})
    asm, raw = out/'fixture.o', out/'fixture.bin'
    helper.run([cc, '-mcpu=68000', '-Wa,-m68000', '-c', repo/'apps/guest-heap-probe/program.S', '-o', asm], env)
    helper.run([bins/'m68k-linux-gnu-objcopy', '-O', 'binary', '--only-section=.text', asm, raw], env)
    text = (repo/'apps/guest-heap-probe/program.h').read_text().split('#define CS_GUEST_HEAP_PROGRAM_BYTES', 1)[1].split('#endif', 1)[0]
    expected = bytes(int(token, 0) for token in re.findall(r'0x[0-9A-Fa-f]+|\b[0-9]+\b', text))
    if len(expected) != 82 or raw.read_bytes() != expected:
        raise ValueError('Original assembler/C fixture differs')
    footprint = out/'footprint.c'
    footprint.write_text('#include "personality/system1/memory-traps.h"\nconst uint32_t cs_s1_bytes=sizeof(cs_s1_traps);\n')
    helper.run([cc, *flags, '-Os', '-I'+str(repo), '-c', footprint, '-o', out/'footprint.o'], env)
    helper.run([bins/'m68k-linux-gnu-objcopy', '-O', 'binary', '--only-section=.rodata', out/'footprint.o', out/'footprint.bin'], env)
    runtime_bytes = int.from_bytes((out/'footprint.bin').read_bytes(), 'big')
    inputs = (*modules, 'core/cpu/m68k-bus.h', 'core/cpu/m68k.h', 'core/services/traps.h',
              'core/memory/guest.h', 'core/memory/manager.h', 'personality/system1/memory-registers.h',
              'personality/system1/memory-traps.h', 'apps/guest-heap-probe/program.h', 'apps/guest-heap-probe/program.S',
              'docs/specifications/SPEC-0025-guest-memory-traps.md', 'scripts/Verify-MemoryTrapsM68k.py')
    result = {'schema_version': 1, 'specification': 'SPEC-0025',
              'compiler': helper.run([cc, '--version'], env).splitlines()[0],
              'object_audits': len(modules)*2, 'complete_relocatable_links': 2,
              'assembler_fixture_bytes': 82, 'assembler_fixture_sha256': hashlib.sha256(expected).hexdigest(),
              'binding_metadata_bytes': runtime_bytes, 'profiles': profiles,
              'inputs': {f: hashlib.sha256((repo/f).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for f in inputs},
              'tool_lock_sha256': hashlib.sha256((repo/'scripts/m68k-toolchain-lock.json').read_bytes().replace(b'\r\n', b'\n')).hexdigest(),
              'native_executed': False, 'rom_integration': False}
    (out/'manifest.json').write_text(json.dumps(result, indent=2)+'\n')
    print('SPEC-0025 standalone 68000 binding/encoding PASS; no execution claim.')

if __name__ == '__main__':
    main()
