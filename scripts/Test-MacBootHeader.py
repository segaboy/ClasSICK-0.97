#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Data-only startup metadata on host, modeled Mac and native 286.

Default media are original. --disk explicitly admits an external private DC42
runtime input; its bytes are never decoded as code or exported in diagnostics.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT/'core/storage/mac-boot-header.c'
BLOCK = ROOT/'core/storage/block-read.c'
SHARED = ROOT/'probes/storage/mac-boot-header.c'
PIN = '7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def original_media():
    disk = bytearray((i*37+(i >> 8)+19) & 255 for i in range(800*512))
    struct.pack_into('>H', disk, 0, 0x4c4b)
    struct.pack_into('>HH', disk, 6, 0xa1d3, 0x83e7)
    names = [b'', b'R', b'A\0\xff', b'OwnDesk', bytes(range(0xe0, 0xef)), b'Hi', b'Scrap']
    digest = 0
    for i, name in enumerate(names):
        disk[10+16*i] = len(name)
        disk[11+16*i:11+16*i+len(name)] = name
        for byte in bytes((len(name),))+name+bytes(15-len(name)):
            digest = (((digest << 5) | (digest >> 27)) & 0xffffffff) ^ byte
    struct.pack_into('>HHIII', disk, 122, 0x1234, 0xfedc, 0x10203040, 0x89abcdef, 0xfedcba98)
    expected = [0x4d424844, 0, 2, 0x4c4b, 0xa1d3, 0x83e7, 0x1234, 0xfedc,
                0x10203040, 0x89abcdef, 0xfedcba98, digest, 1, 0, 0, 0]
    return bytes(disk), expected


def refusal(code, atomic=False):
    return [0x4d424844, code, 2]+[0]*9+[1, 1, int(atomic), 0]


def original_dc42(logical):
    checksum = 0
    for i in range(0, len(logical), 2):
        checksum = (checksum+int.from_bytes(logical[i:i+2], 'big')) & 0xffffffff
        checksum = (checksum >> 1) | ((checksum & 1) << 31)
    header = bytearray(84)
    header[:12] = b'\x0bOwnBootData'
    struct.pack_into('>IIII', header, 64, len(logical), 0, checksum, 0)
    header[80:84] = bytes((0, 2, 1, 0))
    return bytes(header)+logical


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('cc', 'm68k-bin', 'tool-root', 'emulator', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    parser.add_argument('--disk', type=Path,
                        help='explicit external DC42 runtime input; no original-header oracle or fault injection')
    args = parser.parse_args()
    out, tool = args.output.resolve(), args.tool_root.resolve(strict=True)
    cc, exe = args.cc.resolve(strict=True), args.emulator.resolve(strict=True)
    if out == ROOT or ROOT in out.parents:
        raise ValueError('output must be a fresh external directory')
    lock_path = ROOT/'scripts/pc286-toolchain-lock.json'
    paths = [CORE, CORE.with_suffix('.h'), BLOCK, BLOCK.with_suffix('.h'), SHARED,
             SHARED.with_suffix('.h'), ROOT/'tests/mac-boot-header.c',
             ROOT/'docs/specifications/SPEC-0119-mac-boot-header.md',
             ROOT/'docs/specifications/SPEC-0111-read-only-logical-blocks.md',
             ROOT/'docs/specifications/SPEC-0113-boothush-pc286-floppy-provider.md',
             ROOT/'docs/specifications/SPEC-0114-boothush-mac-floppy-provider.md', Path(__file__).resolve(),
             ROOT/'probes/mac128k/boot-header.c', ROOT/'probes/pc286/boot-header.c',
             ROOT/'platform/mac128k/iwm-read.c', ROOT/'platform/mac128k/iwm-read.h',
             ROOT/'platform/mac128k/iwm-capture.S', ROOT/'platform/pc286/boothush-floppy-read.c',
             ROOT/'platform/pc286/boothush-floppy-read.h', ROOT/'arch/i286/c16.h',
             ROOT/'arch/m68k/mac-reset.S', ROOT/'arch/m68k/mac-rom.ld',
             ROOT/'scripts/m68k_windows_toolchain.py', ROOT/'scripts/pc286_loader.py',
             ROOT/'scripts/Test-MfsVolumeMedia.py', lock_path]
    source_identity = {p.relative_to(ROOT).as_posix(): sha(p) for p in paths}
    media_reader = module('boot_header_dc42', ROOT/'scripts/Test-MfsVolumeMedia.py')
    selected = args.disk.resolve(strict=True) if args.disk else None
    if selected and (selected == ROOT or ROOT in selected.parents):
        raise ValueError('runtime disk must remain outside the repository')
    selected_sha = sha(selected) if selected else None
    base, expected = (media_reader.dc42_data(selected), None) if selected else original_media()
    lock = json.loads(lock_path.read_text(encoding='utf-8'))
    tool_paths = {'host_cc': cc, 'python': Path(sys.executable).resolve(), 'emulator': exe}
    for row in lock['selected_members']:
        path = tool/row['path']
        if path.stat().st_size != row['bytes'] or sha(path) != row['sha256']:
            raise ValueError('pinned C16 member differs: '+row['path'])
        tool_paths['pc286:'+row['path']] = path
    if sha(exe) != PIN:
        raise ValueError('unadmitted modeled-device executable')
    native = module('boot_header_m68k', ROOT/'scripts/m68k_windows_toolchain.py')
    m68k, identity = native.inspect(args.m68k_bin)
    tool_paths.update({'m68k:'+k: v for k, v in m68k.items()})
    tool_identity = {k: sha(v) for k, v in tool_paths.items()}
    loader = module('boot_header_loader', ROOT/'scripts/pc286_loader.py')
    out.mkdir(parents=True, exist_ok=False)
    container_controls = []
    if not selected:
        wrapper = original_dc42(base)
        container = out/'original-startup.dc42'
        container.write_bytes(wrapper)
        if media_reader.dc42_data(container) != base:
            raise ValueError('original DC42 round trip differs')
        container_controls.append({'case': 'valid-original', 'sha256': sha(container), 'accepted': True})
        malformed = [('short', wrapper[:-1])]
        for label, offset, value in [('name-count', 0, 64), ('encoding', 80, 1),
                                     ('format', 82, 0), ('data-size', 67, 1),
                                     ('data-checksum', 72, wrapper[72] ^ 1)]:
            changed = bytearray(wrapper)
            changed[offset] = value
            malformed.append((label, bytes(changed)))
        for label, invalid in malformed:
            path = out/('invalid-'+label+'.dc42')
            path.write_bytes(invalid)
            try:
                media_reader.dc42_data(path)
            except ValueError:
                container_controls.append({'case': label, 'sha256': sha(path), 'accepted': False})
            else:
                raise ValueError('malformed DC42 control accepted: '+label)
    env = os.environ.copy()
    env['PATH'] = str(cc.parent)+os.pathsep+str(m68k['gcc'].parent)+os.pathsep+env.get('PATH', '')
    for key in ('WCC', 'WATCOM', 'INCLUDE', 'LIB', 'WCL', 'WLINK'):
        env.pop(key, None)
    commands, hosted, builds, observations = [], [], [], []

    def run(command, label, fail=False):
        command = [str(x) for x in command]
        commands.append(command)
        result = subprocess.run(command, cwd=out, env=env, capture_output=True, timeout=40,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (out/(label+'.stdout')).write_bytes(result.stdout)
        (out/(label+'.stderr')).write_bytes(result.stderr)
        if bool(result.returncode) != fail:
            raise ValueError(label+': unexpected process status; see retained logs')
        return result.stdout

    warning = ['-Wall', '-Wextra', '-Werror', '-Wconversion', '-Wsign-conversion']
    for label, options in [('release', ['-O3', '-DNDEBUG']),
                           ('sanitized', ['-O1', '-fsanitize=address,undefined',
                                          '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'])]:
        binary = out/(label+'.exe')
        run([cc, '-std=c11', *warning, *options, BLOCK, CORE, ROOT/'tests/mac-boot-header.c',
             '-o', binary], label+'-compile')
        result = run([binary], label+'-test').decode().strip()
        if not re.fullmatch(r'Mac boot header: [0-9]+ checks passed', result):
            raise ValueError('host test did not report completion')
        hosted.append({'profile': label, 'result': result, 'binary_sha256': sha(binary)})

    variants = [('baseline', CORE)]
    media_cases = [('baseline', base, expected, False), ('fresh-recovery', base, expected, False)]
    if not selected:
        text = CORE.read_text(encoding='utf-8')
        anchor = 'if(area[10u+16u*i]>15u)return CS_MAC_BOOT_NAME;'
        if text.count(anchor) != 1:
            raise ValueError('name-bound mutation anchor changed')
        mutation = out/'name-bound-defect.c'
        mutation.write_text(text.replace(anchor, 'if(0)return CS_MAC_BOOT_NAME;'), encoding='utf-8')
        variants.append(('name-bound-defect', mutation))
        bad_signature, bad_name = bytearray(base), bytearray(base)
        bad_signature[0] = 0
        bad_name[106] = 16
        media_cases[1:1] = [('bad-signature', bytes(bad_signature), refusal(3), False),
                            ('bad-name', bytes(bad_name), refusal(4), False),
                            ('second-sector-fault', base, refusal(6, True), True)]
    c16_flags = ['-zq', '-2', '-ms', '-zl', '-s', '-os', '-zastd=c99', '-we',
                 '-dCS_C16_REQUIRE_16BIT', '-i='+str(tool/'h'), '-i='+str(CORE.parent)]

    for lane in ('pc286', 'mac128k'):
        for build_name, core in variants:
            label = lane+'-'+build_name
            inputs = [BLOCK, core, SHARED]
            if lane == 'pc286':
                inputs += [ROOT/'platform/pc286/boothush-floppy-read.c', ROOT/'probes/pc286/boot-header.c']
                objects = []
                for i, source in enumerate(inputs):
                    obj = out/(label+'-'+str(i)+'.obj')
                    run([tool/'binnt64/wcc.exe', *c16_flags, '-fo='+str(obj), source], label+'-compile-'+str(i))
                    metadata = run([tool/'binnt64/dmpobj.exe', obj], label+'-object-'+str(i))
                    if b'80286' not in metadata or b'USE16' not in metadata:
                        raise ValueError('non-native C16 object')
                    objects.append(obj)
                binary, mapping = out/(label+'.bin'), out/(label+'.map')
                run([tool/'binnt64/wlink.exe', 'format', 'raw', 'bin', 'option', 'start=start_',
                     'option', 'nodefaultlibs', 'option', 'map='+str(mapping), 'name', binary,
                     'file', ','.join(str(p) for p in objects)], label+'-link')
                link_text = mapping.read_text(encoding='utf-8', errors='replace')
                entry = re.search(r'Entry point address:\s*([0-9a-fA-F]+)', link_text)
                bss = re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)', link_text, re.M)
                report = re.search(r'^\s*([0-9a-fA-F]+)[+*]?\s+_cs_boot_header_report\s*$', link_text, re.M)
                if not entry or not bss or not report or '.lib' in link_text.lower():
                    raise ValueError('missing C16 symbols or unwanted runtime')
                bss_start, bss_size = (int(x, 16) for x in bss.groups())
                report_address = 0x10000+int(report.group(1), 16)
                payload = binary.read_bytes()
                spans = [(0x10000+i, payload[i:i+4096]) for i in range(0, len(payload), 4096)]
                if len(payload)+64 > 16384:
                    raise ValueError('payload exceeds full verification inspection capacity')
                image = loader.floppy(payload, int(entry.group(1), 16), bss_end=bss_start+bss_size,
                                      poison_bss=True, verify_payload=True)
                builds.append({'lane': lane, 'case': build_name, 'payload_bytes': len(payload),
                               'payload_sha256': sha(binary), 'bss_bytes': bss_size, 'runtime_modules': []})
            else:
                elf, binary, rom = (out/(label+suffix) for suffix in ('.elf', '.bin', '.rom'))
                run([m68k['gcc'], '-mcpu=68000', '-std=c99', '-Os', *warning, '-ffreestanding',
                     '-fno-builtin', '-nostdlib', '-I', CORE.parent, '-Wl,--build-id=none',
                     '-Wl,-T,'+str(ROOT/'arch/m68k/mac-rom.ld'), ROOT/'arch/m68k/mac-reset.S',
                     *inputs, ROOT/'platform/mac128k/iwm-read.c', ROOT/'platform/mac128k/iwm-capture.S',
                     ROOT/'probes/mac128k/boot-header.c', '-o', elf], label+'-build')
                if run([m68k['nm'], '-u', elf], label+'-imports').strip():
                    raise ValueError('unexpected native Mac runtime import')
                symbols = run([m68k['nm'], '-n', elf], label+'-symbols').decode()
                def address(name):
                    match = re.search(r'^([0-9a-fA-F]+) [BbDd] '+name+r'\s*$', symbols, re.M)
                    if not match:
                        raise ValueError('missing Mac report symbol')
                    return int(match.group(1), 16)
                diag_address, report_address = address('cs_mac_diag'), address('cs_boot_header_report')
                run([m68k['objcopy'], '-O', 'binary', elf, binary], label+'-binary')
                payload = binary.read_bytes()
                if not 260 <= len(payload) <= 65536:
                    raise ValueError('Mac probe exceeds ROM capacity')
                rom.write_bytes(payload+bytes([255])*(65536-len(payload)))
                builds.append({'lane': lane, 'case': build_name, 'payload_bytes': len(payload),
                               'payload_sha256': sha(binary), 'runtime_modules': []})
            cases = media_cases if build_name == 'baseline' else [('name-bound-defect', bytes(bad_name), refusal(4), False)]
            for name, logical, oracle, fault in cases:
                case_label = lane+'-'+name
                media = out/(case_label+'.img')
                if lane == 'pc286':
                    disk = bytearray(image)
                    disk[60*512:860*512] = logical
                    media.write_bytes(disk)
                    command = [exe, '--machine', lane, '--floppy', media, '--cycles', '400000000',
                               '--timeout-ms', '30000', '--until-post', '0xb7']
                    for at, chunk in spans:
                        command += ['--inspect', hex(at)+':'+str(len(chunk))]
                    command += ['--inspect', hex(report_address)+':64']
                    if fault:
                        command += ['--fail-floppy-read', '2:0:2']
                    run_inputs = [media]
                else:
                    media.write_bytes(logical)
                    command = [exe, '--machine', lane, '--rom', rom, '--floppy', media,
                               '--cycles', '400000000', '--timeout-ms', '30000',
                               '--until', hex(diag_address)+':4:0xb7', '--inspect', hex(diag_address)+':28',
                               '--inspect', hex(report_address)+':64', '--mac-floppy-state', '1']
                    if fault:
                        command += ['--mac-floppy-bad-checksum', '0:1']
                    run_inputs = [media, rom]
                before = {str(p): sha(p) for p in run_inputs}
                result = json.loads(run(command, case_label+'-guest'))
                memories = result.pop('memory_ranges', [])
                if result.get('status') != 'condition_met':
                    raise ValueError(case_label+': guest did not complete')
                if lane == 'pc286':
                    if result.get('post') != 0xb7 or result.get('cs') != 0x1000 or len(memories) != len(spans)+1:
                        raise ValueError(case_label+': wrong native PC stop')
                    for memory, (at, chunk) in zip(memories[:-1], spans):
                        if memory.get('address') != at or bytes.fromhex(memory.get('bytes', '')) != chunk:
                            raise ValueError('loaded PC payload differs')
                    if fault:
                        injection = result.get('floppy_read_failure', {})
                        if [injection.get(k) for k in ('injected', 'cylinder', 'head', 'sector')] != [1, 2, 0, 2]:
                            raise ValueError('missing exact second-sector PC fault')
                else:
                    if len(memories) != 2 or memories[0].get('address') != diag_address:
                        raise ValueError('missing Mac diagnostic')
                    diag = list(struct.unpack('>7I', bytes.fromhex(memories[0]['bytes'])))
                    if diag[0] != 0xb7 or diag[6]:
                        raise ValueError('Mac stop or exception differs')
                    if fault:
                        injection = result.get('mac_floppy_checksum_fault', {})
                        if (injection.get('kind') != 'synthetic_gcr_data_checksum' or
                            injection.get('mode') != 'persistent' or injection.get('track') != 0 or
                            injection.get('sector') != 1 or injection.get('altered_nibbles', 0) < 1):
                            raise ValueError('missing exact second-sector Mac fault')
                if memories[-1].get('address') != report_address:
                    raise ValueError('report address differs')
                words = list(struct.unpack(('<' if lane == 'pc286' else '>')+'16I', bytes.fromhex(memories[-1]['bytes'])))
                rejected = (words != oracle) if oracle is not None else (
                    words[:4] != [0x4d424844, 0, 2, 0x4c4b] or words[12:] != [1, 0, 0, 0])
                if build_name == 'baseline' and rejected:
                    raise ValueError(case_label+': numeric metadata or preservation oracle differs: '+repr(words))
                if build_name != 'baseline' and (not rejected or words[1] != 0):
                    raise ValueError('name-bound defect was not detected by unchanged oracle')
                if before != {str(p): sha(p) for p in run_inputs}:
                    raise ValueError('native run input changed')
                observations.append({'lane': lane, 'case': name, 'report': words, 'result': result,
                                     'inputs': before, 'defect_rejected': rejected,
                                     'full_payload_matches': lane == 'pc286'})
    for lane in ('pc286', 'mac128k'):
        rows = [r for r in observations if r['lane'] == lane and r['case'] in ('baseline', 'fresh-recovery')]
        if rows[0]['report'] != rows[1]['report'] or rows[0]['result'] != rows[1]['result']:
            raise ValueError('fresh healthy recovery differs: '+lane)
    if source_identity != {p.relative_to(ROOT).as_posix(): sha(p) for p in paths}:
        raise ValueError('source changed during verification')
    if tool_identity != {k: sha(v) for k, v in tool_paths.items()} or native.inspect(args.m68k_bin)[1] != identity:
        raise ValueError('tool identity changed during verification')
    if selected and sha(selected) != selected_sha:
        raise ValueError('selected runtime container changed')
    baselines = [r['report'] for r in observations if r['case'] == 'baseline']
    if baselines[0] != baselines[1]:
        raise ValueError('native metadata reports differ')
    report = {'status': 'pass', 'source_sha256': source_identity, 'tool_sha256': tool_identity,
              'm68k_identity': identity, 'hosted': hosted, 'builds': builds, 'observed': observations,
              'commands': commands, 'expected_original': expected, 'inputs_unchanged': True,
              'private_runtime_input': bool(selected), 'selected_container_sha256': selected_sha,
              'logical_media_sha256': hashlib.sha256(base).hexdigest(), 'container_controls': container_controls,
              'limits': 'Data-only startup metadata; only numeric fields and a copied-name digest are reported. '
                        'Explicit external runtime inputs have no original-fixture oracle, mutation or fault injection. '
                        'Transfer copies remain outside the repository. No boot entry ABI, legacy execution, '
                        'OS startup, System Disk boot or physical hardware claim.'}
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'status': 'pass', 'cases': len(observations), 'report': str(out/'report.json')}))
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, struct.error, subprocess.TimeoutExpired) as exc:
        print('Boot header verification failed: '+str(exc), file=sys.stderr)
        sys.exit(1)
