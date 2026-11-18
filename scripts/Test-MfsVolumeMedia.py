#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Read original MFS forks through native modeled floppy providers on both CPUs.

An explicit --disk is an authorized private DC42 runtime input, never a fixture
or implementation source. Only counts/digests and original probe code are inspected.
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
PINNED_EMULATOR = '7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a'
CORE = [ROOT/'core/storage'/name for name in ('block-read.c', 'mfs-read.c', 'mfs-volume.c')]
SHARED = ROOT/'probes/storage/mfs-media.c'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def folded(data):
    value = 0
    for byte in data:
        value = (((value << 5) | (value >> 27)) & 0xffffffff) ^ byte
    return value


def original_media():
    """Independent fixture from SPEC-0112/0115 and the admitted workload contract."""
    disk = bytearray(800*512)
    header = memoryview(disk)[2*512:3*512]
    struct.pack_into('>HIIHHHHHIIHIH', header, 0, 0xd2d7, 1, 0, 0, 2,
                     4, 2, 16, 1024, 1024, 16, 4, 11)
    header[36:43] = b'\x06OwnMFS'
    links = [5, 1, 1, 3, 1] + [0]*11
    for index in range(0, len(links), 2):
        a, b = links[index:index+2]
        header[64+index*3//2:67+index*3//2] = bytes((a >> 4, ((a & 15) << 4) | (b >> 8), b & 255))

    def entry(lba, name, file_id, data_start, data_length, data_physical,
              resource_start, resource_length, resource_physical):
        at = lba*512
        disk[at] = 0x80
        struct.pack_into('>IHIIHIIII', disk, at+18, file_id, data_start, data_length,
                         data_physical, resource_start, resource_length, resource_physical, 0, 0)
        disk[at+50] = len(name)
        disk[at+51:at+51+len(name)] = name

    entry(4, b'Start', 2, 2, 2437, 3072, 4, 17, 1024)
    entry(5, b'Desk', 3, 0, 0, 0, 6, 513, 1024)
    logical = bytearray()
    for chain, length, salt in (([2, 5, 3], 2437, 7), ([4], 17, 91), ([6], 513, 173)):
        payload = bytes((i*29+(i >> 8)+salt) & 255 for i in range(len(chain)*1024))
        for index, allocation in enumerate(chain):
            start = (16+(allocation-2)*2)*512
            disk[start:start+1024] = payload[index*1024:(index+1)*1024]
        logical.extend(payload[:length])
    return bytes(disk), [0x4d465356, 2, 1, 2, 16, 11, 2, 4, 2967, folded(logical), 0, 0]


def dc42_data(path):
    """Validate the container and return only its logical sectors; never decode code."""
    size = path.stat().st_size
    if size not in (84+409600, 84+409600+9600):
        raise ValueError('private DC42 size is outside the 400 KiB profile')
    image = path.read_bytes()
    data_size, tag_size, data_sum, tag_sum = struct.unpack_from('>IIII', image, 64)
    if (image[0] > 63 or data_size != 409600 or tag_size not in (0, 9600) or
            size != 84+data_size+tag_size or image[80:84] != bytes((0, 2, 1, 0))):
        raise ValueError('private DC42 header is outside the admitted profile')

    def checksum(payload):
        value = 0
        for i in range(0, len(payload), 2):
            value = (value+int.from_bytes(payload[i:i+2], 'big')) & 0xffffffff
            value = (value >> 1) | ((value & 1) << 31)
        return value

    data, tags = image[84:84+data_size], image[84+data_size:]
    if checksum(data) != data_sum or checksum(tags[12:]) != tag_sum:
        raise ValueError('private DC42 checksum mismatch')
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tool-root', type=Path)
    parser.add_argument('--m68k-bin', type=Path)
    parser.add_argument('--emulator', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--lane', choices=('pc286', 'mac128k', 'both'), default='both')
    parser.add_argument('--disk', type=Path, help='explicit private DC42 runtime input; never used by default')
    parser.add_argument('--cycles', type=int, default=400000000)
    parser.add_argument('--timeout-ms', type=int, default=30000)
    args = parser.parse_args()
    pc, mac = args.lane in ('pc286', 'both'), args.lane in ('mac128k', 'both')
    if (pc and not args.tool_root) or (mac and not args.m68k_bin):
        parser.error('selected lanes require --tool-root and/or --m68k-bin')
    if not 1 <= args.cycles <= 4000000000 or not 1 <= args.timeout_ms <= 60000:
        parser.error('cycles must be 1..4000000000 and timeout-ms 1..60000')
    exe, out = args.emulator.resolve(strict=True), args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError('outputs and private transfer media must remain outside the repository')
    if sha(exe) != PINNED_EMULATOR:
        raise ValueError('emulator does not match the admitted modeled-device executable')
    selected = args.disk.resolve(strict=True) if args.disk else None
    if selected and (selected == ROOT or ROOT in selected.parents):
        raise ValueError('private runtime disk must remain outside the repository')
    selected_sha = sha(selected) if selected else None
    logical, expected = (dc42_data(selected), None) if selected else original_media()
    tools = {'emulator': exe, 'python': Path(sys.executable).resolve()}
    sources = [*CORE, *(p.with_suffix('.h') for p in CORE), SHARED, SHARED.with_suffix('.h'),
               ROOT/'docs/specifications/SPEC-0115-mfs-volume-reader.md',
               ROOT/'docs/specifications/SPEC-0112-mfs-metadata-reader.md', Path(__file__).resolve()]
    native, native_identity, m68k, tool = None, None, {}, None
    if pc:
        tool = args.tool_root.resolve(strict=True)
        lock_path = ROOT/'scripts/pc286-toolchain-lock.json'
        lock = json.loads(lock_path.read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            path = tool/row['path']
            if path.stat().st_size != row['bytes'] or sha(path) != row['sha256']:
                raise ValueError('pinned C16 tool/header mismatch: '+row['path'])
            tools['pc286:'+row['path']] = path
        sources += [lock_path, ROOT/'arch/i286/c16.h', ROOT/'scripts/pc286_loader.py',
                    ROOT/'platform/pc286/boothush-floppy-read.c',
                    ROOT/'platform/pc286/boothush-floppy-read.h', ROOT/'probes/pc286/boot-mfs-media.c']
    if mac:
        native = module('mfs_media_tools', ROOT/'scripts/m68k_windows_toolchain.py')
        m68k, native_identity = native.inspect(args.m68k_bin)
        tools.update({'m68k:'+key: path for key, path in m68k.items()})
        sources += [ROOT/'scripts/m68k_windows_toolchain.py', ROOT/'arch/m68k/mac-reset.S',
                    ROOT/'arch/m68k/mac-rom.ld', ROOT/'platform/mac128k/iwm-read.c',
                    ROOT/'platform/mac128k/iwm-read.h', ROOT/'platform/mac128k/iwm-capture.S',
                    ROOT/'probes/mac128k/mfs-media.c']
    before_sources = {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}
    before_tools = {key: sha(path) for key, path in tools.items()}
    out.mkdir(parents=True, exist_ok=False)
    raw_disk = out/'logical-transfer.raw'
    raw_disk.write_bytes(logical)
    raw_sha = sha(raw_disk)
    env = os.environ.copy()
    for key in ('WCC', 'WATCOM', 'INCLUDE', 'LIB', 'WCL', 'WLINK'):
        env.pop(key, None)
    if mac:
        env['PATH'] = str(m68k['gcc'].parent)+os.pathsep+env.get('PATH', '')
    commands, observations, builds = [], [], {}

    def run(command, label):
        command = [str(value) for value in command]
        commands.append(command)
        result = subprocess.run(command, cwd=out, env=env, capture_output=True,
                                timeout=max(40, args.timeout_ms/1000+5),
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (out/(label+'.stdout')).write_bytes(result.stdout)
        (out/(label+'.stderr')).write_bytes(result.stderr)
        if result.returncode:
            raise ValueError(label+' failed; see retained stdout/stderr')
        return result.stdout

    if pc:
        loader = module('mfs_media_loader', ROOT/'scripts/pc286_loader.py')
        flags = ['-zq', '-2', '-ms', '-zl', '-s', '-os', '-zastd=c99', '-we',
                 '-dCS_C16_REQUIRE_16BIT', '-i='+str(tool/'h')]
        objects = []
        for index, source in enumerate([*CORE, SHARED, ROOT/'platform/pc286/boothush-floppy-read.c',
                                        ROOT/'probes/pc286/boot-mfs-media.c']):
            obj = out/('pc-'+str(index)+'.obj')
            run([tool/'binnt64/wcc.exe', *flags, '-fo='+str(obj), source], 'pc-compile-'+str(index))
            metadata = run([tool/'binnt64/dmpobj.exe', obj], 'pc-object-'+str(index))
            if b'80286' not in metadata or b'USE16' not in metadata:
                raise ValueError('non-native C16 object')
            objects.append(obj)
        binary, mapping = out/'pc.bin', out/'pc.map'
        run([tool/'binnt64/wlink.exe', 'format', 'raw', 'bin', 'option', 'start=start_',
             'option', 'nodefaultlibs', 'option', 'map='+str(mapping), 'name', binary,
             'file', ','.join(str(p) for p in objects)], 'pc-link')
        text = mapping.read_text(encoding='utf-8', errors='replace')
        entry = re.search(r'Entry point address:\s*([0-9a-fA-F]+)', text)
        bss = re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)', text, re.M)
        report = re.search(r'^\s*([0-9a-fA-F]+)[+*]?\s+_cs_mfs_media_report\s*$', text, re.M)
        if not entry or not bss or not report or '.lib' in text.lower():
            raise ValueError('missing 286 entry/BSS/report or runtime library')
        payload = binary.read_bytes()
        bss_start, bss_size = (int(value, 16) for value in bss.groups())
        report_address = 0x10000+int(report.group(1), 16)
        if len(payload)+48 > 16384:
            raise ValueError('full payload plus report exceeds single-run inspection budget')
        image = bytearray(loader.floppy(payload, int(entry.group(1), 16), bss_end=bss_start+bss_size,
                                       poison_bss=True, verify_payload=True))
        image[60*512:860*512] = logical
        media = out/'pc-private-transfer.img'
        media.write_bytes(image)
        spans = [(0x10000+i, payload[i:i+4096]) for i in range(0, len(payload), 4096)]
        builds['pc286'] = {'payload_bytes': len(payload), 'payload_sha256': sha(binary),
                           'bss_start': bss_start, 'bss_bytes': bss_size,
                           'report_address': report_address, 'runtime_modules': []}

        def pc_command(marker, fault):
            cmd = [exe, '--machine', 'pc286', '--floppy', media, '--cycles', str(args.cycles),
                   '--timeout-ms', str(args.timeout_ms), '--until-post', hex(marker)]
            for address, chunk in spans:
                cmd += ['--inspect', hex(address)+':'+str(len(chunk))]
            cmd += ['--inspect', hex(report_address)+':48']
            if fault:
                cmd += ['--fail-floppy-read', '2:1:8']
            return cmd

        pc_media_sha = sha(media)
        for name, fault in ([('baseline', False), ('late-fault', True), ('fresh-recovery', False)]
                            if expected else [('baseline', False), ('repeat', False)]):
            marker = 0xd4 if fault else 0xb7
            result = json.loads(run(pc_command(marker, fault), 'pc-'+name))
            memories = result.pop('memory_ranges', [])
            if (result.get('status') != 'condition_met' or result.get('post') != marker or
                    result.get('cs') != 0x1000 or len(memories) != len(spans)+1):
                raise ValueError('286 '+name+': missing stop or full payload inspection')
            for memory, (address, chunk) in zip(memories[:-1], spans):
                if memory.get('address') != address or bytes.fromhex(memory.get('bytes', '')) != chunk:
                    raise ValueError('286 loaded payload differs')
            if memories[-1].get('address') != report_address:
                raise ValueError('286 report address differs')
            words = list(struct.unpack('<12I', bytes.fromhex(memories[-1]['bytes'])))
            if fault:
                injection = result.get('floppy_read_failure', {})
                if [injection.get(k) for k in ('injected', 'cylinder', 'head', 'sector')] != [1, 2, 1, 8]:
                    raise ValueError('missing exact injected 286 sector failure')
            validate(words, expected, fault)
            if sha(media) != pc_media_sha:
                raise ValueError('286 transfer media changed')
            observations.append({'lane': 'pc286', 'case': name, 'report': words, 'result': result,
                                 'full_payload_matches': True, 'media_sha256': pc_media_sha})

    if mac:
        elf, raw, rom = out/'mac.elf', out/'mac.bin', out/'mac.rom'
        run([m68k['gcc'], '-mcpu=68000', '-std=c99', '-Os', '-Wall', '-Wextra', '-Werror',
             '-Wconversion', '-Wsign-conversion', '-ffreestanding', '-fno-builtin', '-nostdlib',
             '-Wl,--build-id=none', '-Wl,-T,'+str(ROOT/'arch/m68k/mac-rom.ld'),
             ROOT/'arch/m68k/mac-reset.S', *CORE, SHARED, ROOT/'platform/mac128k/iwm-read.c',
             ROOT/'platform/mac128k/iwm-capture.S', ROOT/'probes/mac128k/mfs-media.c', '-o', elf], 'mac-build')
        if run([m68k['nm'], '-u', elf], 'mac-imports').strip():
            raise ValueError('unexpected Mac runtime dependency')
        symbols = run([m68k['nm'], '-n', elf], 'mac-symbols').decode('utf-8')

        def symbol(name):
            match = re.search(r'^([0-9a-fA-F]+) [BbDd] '+name+r'\s*$', symbols, re.M)
            if match is None:
                raise ValueError('missing Mac diagnostic symbol '+name)
            return int(match.group(1), 16)

        diag_address, report_address = symbol('cs_mac_diag'), symbol('cs_mfs_media_report')
        run([m68k['objcopy'], '-O', 'binary', elf, raw], 'mac-binary')
        payload = raw.read_bytes()
        if not 260 <= len(payload) <= 65536:
            raise ValueError('Mac probe outside 64 KiB ROM budget')
        rom.write_bytes(payload+bytes([255])*(65536-len(payload)))
        rom_sha = sha(rom)
        builds['mac128k'] = {'rom_sha256': rom_sha, 'payload_bytes': len(payload),
                            'diag_address': diag_address, 'report_address': report_address,
                            'runtime_modules': []}
        for name, fault in ([('baseline', False), ('late-fault', True), ('fresh-recovery', False)]
                            if expected else [('baseline', False), ('repeat', False)]):
            marker = 0xd4 if fault else 0xb7
            cmd = [exe, '--machine', 'mac128k', '--rom', rom, '--floppy', raw_disk,
                   '--cycles', str(args.cycles), '--timeout-ms', str(args.timeout_ms),
                   '--until', hex(diag_address)+':4:'+hex(marker), '--inspect', hex(diag_address)+':28',
                   '--inspect', hex(report_address)+':48', '--mac-floppy-state', '1']
            if fault:
                cmd += ['--mac-floppy-bad-checksum', '1:10']
            result = json.loads(run(cmd, 'mac-'+name))
            memories = result.pop('memory_ranges', [])
            if (result.get('status') != 'condition_met' or len(memories) != 2 or
                    [m.get('address') for m in memories] != [diag_address, report_address]):
                raise ValueError('Mac '+name+': missing stop/diagnostics')
            diag = list(struct.unpack('>7I', bytes.fromhex(memories[0]['bytes'])))
            words = list(struct.unpack('>12I', bytes.fromhex(memories[1]['bytes'])))
            if diag[0] != marker or diag[6]:
                raise ValueError('Mac guest marker or fault differs')
            if fault:
                injection = result.get('mac_floppy_checksum_fault', {})
                if (injection.get('kind') != 'synthetic_gcr_data_checksum' or
                    injection.get('mode') != 'persistent' or
                    [injection.get(k) for k in ('track', 'sector')] != [1, 10] or
                    injection.get('altered_nibbles', 0) < 1):
                    raise ValueError('missing exact Mac checksum injection')
            validate(words, expected, fault)
            if sha(rom) != rom_sha or sha(raw_disk) != raw_sha:
                raise ValueError('Mac input changed')
            observations.append({'lane': 'mac128k', 'case': name, 'report': words, 'diag': diag,
                                 'result': result, 'media_sha256': raw_sha})

    for lane in ('pc286', 'mac128k'):
        rows = [row for row in observations if row['lane'] == lane and row['case'] != 'late-fault']
        if rows and (rows[0]['result'] != rows[1]['result'] or rows[0]['report'] != rows[1]['report']):
            raise ValueError(lane+': fresh repeat differs')
    successful = [row['report'] for row in observations if row['case'] == 'baseline']
    if any(row != successful[0] for row in successful):
        raise ValueError('native lane counts/digests differ')
    if before_sources != {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}:
        raise ValueError('declared source changed during verification')
    if before_tools != {key: sha(path) for key, path in tools.items()}:
        raise ValueError('declared tool changed during verification')
    if sha(raw_disk) != raw_sha or (selected and sha(selected) != selected_sha):
        raise ValueError('input media changed during verification')
    if mac and native.inspect(args.m68k_bin)[1] != native_identity:
        raise ValueError('m68k identity changed')
    report = {'status': 'pass', 'source_sha256': before_sources, 'tool_sha256': before_tools,
              'm68k_identity': native_identity, 'builds': builds, 'observed': observations,
              'commands': commands, 'expected_original': expected, 'private_runtime_input': bool(selected),
              'selected_container_sha256': selected_sha, 'logical_media_sha256': raw_sha,
              'inputs_unchanged': True,
              'limits': 'Read-only counts/digests through native modeled block providers; no protected file bytes exposed in diagnostics; '
                        'private logical-sector transfer copies remain in the selected external output directory. No '
                        'legacy execution, File Manager/OS startup, write persistence or physical hardware claim. '
                        'The late-file fault checks one 256-byte request, not a multi-sector request failure. '
                        'Optional private runtime media receives no fixed-sector fault injection or original-fixture oracle.'}
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'status': 'pass', 'cases': len(observations), 'report': str(out/'report.json')}))
    return 0


def validate(words, expected, fault):
    if fault:
        if words != [0x4d465356, 1, 8, 2, 16, 11, 0, 0, 0, 0, 0, 0]:
            raise ValueError('fault did not preserve output or report the expected read failure')
    elif expected:
        if words != expected:
            raise ValueError('original directory/fork counts or independent digest differ')
    elif words[0:3] != [0x4d465356, 2, 1] or words[6] != words[3] or words[7] != 2*words[6]:
        raise ValueError('private runtime scan did not complete its declared directory and forks')


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, struct.error, subprocess.TimeoutExpired) as exc:
        print('MFS media verification failed: '+str(exc), file=sys.stderr)
        sys.exit(1)
