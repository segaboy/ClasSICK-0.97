#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Prepare startup file resource sources through paired native modeled providers.

An explicit --disk is an authorized private DC42 runtime input, never a fixture
or implementation source. Only aggregate metadata and original probe code are inspected.
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
CORE = [ROOT/'core/storage'/name for name in ('block-read.c', 'mfs-read.c', 'mfs-volume.c', 'mac-boot-header.c', 'startup-files.c')]
CORE += [ROOT/'formats/resourcefork'/name for name in ('transport.c', 'mfs-transport.c')]
SHARED = ROOT/'probes/storage/startup-files.c'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', type=Path, required=True)
    parser.add_argument('--tool-root', type=Path)
    parser.add_argument('--m68k-bin', type=Path)
    parser.add_argument('--emulator', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--lane', choices=('pc286', 'mac128k', 'both'), default='both')
    parser.add_argument('--disk', type=Path, help='explicit private DC42 runtime input; never used by default')
    parser.add_argument('--cycles', type=int, default=1200000000)
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
    tools = {'emulator': exe, 'python': Path(sys.executable).resolve(), 'host_cc': args.cc.resolve(strict=True)}
    sources = [*CORE, *(p.with_suffix('.h') for p in CORE), SHARED, SHARED.with_suffix('.h'),
               ROOT/'docs/specifications/SPEC-0115-mfs-volume-reader.md',
               ROOT/'docs/specifications/SPEC-0112-mfs-metadata-reader.md',
               ROOT/'docs/specifications/SPEC-0116-streamed-resource-fork.md',
               ROOT/'docs/specifications/SPEC-0024-resource-fork-reader.md',
               ROOT/'docs/specifications/SPEC-0122-startup-files.md',
               ROOT/'formats/resourcefork/reader.h', ROOT/'scripts/Test-MfsVolumeMedia.py',
               ROOT/'scripts/resource_media_fixture.py', ROOT/'scripts/startup_files_fixture.py',
               ROOT/'tests/startup-files.c', ROOT/'scripts/Test-MacBootHeader.py',
               ROOT/'docs/specifications/SPEC-0119-mac-boot-header.md',
               ROOT/'docs/specifications/SPEC-0111-read-only-logical-blocks.md',
               ROOT/'docs/specifications/SPEC-0113-boothush-pc286-floppy-provider.md',
               ROOT/'docs/specifications/SPEC-0114-boothush-mac-floppy-provider.md', Path(__file__).resolve()]
    native, native_identity, m68k, tool = None, None, {}, None
    if pc:
        tool = args.tool_root.resolve(strict=True)
        lock_path = ROOT/'scripts/pc286-toolchain-lock.json'
        sources += [lock_path, ROOT/'arch/i286/c16.h', ROOT/'scripts/pc286_loader.py',
                    ROOT/'platform/pc286/boothush-floppy-read.c',
                    ROOT/'platform/pc286/boothush-floppy-read.h', ROOT/'probes/pc286/boot-startup-files.c']
    if mac:
        sources += [ROOT/'scripts/m68k_windows_toolchain.py', ROOT/'arch/m68k/mac-reset.S',
                    ROOT/'arch/m68k/mac-rom.ld', ROOT/'platform/mac128k/iwm-read.c',
                    ROOT/'platform/mac128k/iwm-read.h', ROOT/'platform/mac128k/iwm-capture.S',
                    ROOT/'platform/mac128k/startup-floppy.c', ROOT/'platform/mac128k/startup-floppy.h',
                    ROOT/'probes/mac128k/startup-files.c']
    before_sources = {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}
    if pc:
        lock = json.loads(lock_path.read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            path = tool/row['path']
            if path.stat().st_size != row['bytes'] or sha(path) != row['sha256']:
                raise ValueError('pinned C16 tool/header mismatch: '+row['path'])
            tools['pc286:'+row['path']] = path
    if mac:
        native = module('mfs_media_tools', ROOT/'scripts/m68k_windows_toolchain.py')
        m68k, native_identity = native.inspect(args.m68k_bin)
        tools.update({'m68k:'+key: path for key, path in m68k.items()})
    before_tools = {key: sha(path) for key, path in tools.items()}
    media_helper = module('startup_container', ROOT/'scripts/Test-MfsVolumeMedia.py')
    fixture = module('startup_fixture', ROOT/'scripts/startup_files_fixture.py')
    logical, oracle = (media_helper.dc42_data(selected), None) if selected else fixture.original_media()
    expected = oracle['expected'] if oracle else None
    out.mkdir(parents=True, exist_ok=False)
    raw_disk = out/'logical-transfer.raw'
    raw_disk.write_bytes(logical)
    raw_sha = sha(raw_disk)
    env = os.environ.copy()
    for key in ('WCC', 'WATCOM', 'INCLUDE', 'LIB', 'WCL', 'WLINK'):
        env.pop(key, None)
    env['PATH'] = str(args.cc.resolve().parent)+os.pathsep+env.get('PATH', '')
    if mac:
        env['PATH'] = str(m68k['gcc'].parent)+os.pathsep+env.get('PATH', '')
    commands, observations, builds = [], [], {}

    def run(command, label, expect_failure=False):
        command = [str(value) for value in command]
        commands.append(command)
        result = subprocess.run(command, cwd=out, env=env, capture_output=True,
                                timeout=max(40, args.timeout_ms/1000+5),
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (out/(label+'.stdout')).write_bytes(result.stdout)
        (out/(label+'.stderr')).write_bytes(result.stderr)
        if bool(result.returncode) != expect_failure:
            raise ValueError(label+' failed; see retained stdout/stderr')
        if expect_failure and b'startup check failed:' not in result.stderr:
            raise ValueError(label+' did not reach the unchanged test oracle')
        return result.stdout

    cc = args.cc.resolve(strict=True)
    host_disk = out/'original-host.raw'
    original_logical, _ = fixture.original_media()
    host_disk.write_bytes(original_logical)
    hosted, mutants, container_controls = [], [], []
    warning = ['-Wall', '-Wextra', '-Werror', '-Wconversion', '-Wsign-conversion']
    for label, options in [('release', ['-O3', '-DNDEBUG']),
                           ('sanitized', ['-O1', '-fsanitize=address,undefined',
                                          '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'])]:
        binary = out/(label+'.exe')
        run([cc, '-std=c11', *warning, *options, *CORE, SHARED,
             ROOT/'tests/startup-files.c', '-o', binary], label+'-compile')
        message = run([binary, host_disk], label+'-test').decode().strip()
        if not re.fullmatch(r'Startup files: [0-9]+ checks passed', message):
            raise ValueError('host test did not complete')
        hosted.append({'profile': label, 'result': message, 'binary_sha256': sha(binary)})
    coordinator = ROOT/'core/storage/startup-files.c'
    original_code = coordinator.read_text()
    mutations = [
        ('no-rebase', 'out->resources[i].source.context=&out->adapters[i];', '(void)i;'),
        ('early-publication', 'report->stage=CS_STARTUP_OPEN;',
         'report->stage=CS_STARTUP_OPEN;copy_bytes(out,candidate,sizeof *out);'),
        ('wrong-shell', 'candidate->header.names[i]', 'candidate->header.names[0]'),
    ]
    for label, anchor, replacement in mutations:
        if original_code.count(anchor) != 1:
            raise ValueError('mutation anchor changed: '+label)
        source, binary = out/(label+'.c'), out/(label+'.exe')
        source.write_text(original_code.replace(anchor, replacement))
        run([cc, '-std=c11', *warning, '-O2', '-I'+str(coordinator.parent),
             *(source if p == coordinator else p for p in CORE), SHARED,
             ROOT/'tests/startup-files.c', '-o', binary], label+'-compile')
        run([binary, host_disk], label+'-test', expect_failure=True)
        mutants.append({'case': label, 'source_sha256': sha(source), 'rejected': True})
    wrapper_helper = module('startup_dc42_writer', ROOT/'scripts/Test-MacBootHeader.py')
    wrapper = wrapper_helper.original_dc42(original_logical)
    container = out/'original-startup-files.dc42'
    container.write_bytes(wrapper)
    if media_helper.dc42_data(container) != original_logical:
        raise ValueError('generated DC42 round trip differs')
    container_controls.append({'case': 'valid-original', 'accepted': True, 'sha256': sha(container)})
    malformed = [('truncated', wrapper[:-1]), ('trailing', wrapper+b'\0')]
    for label, offset, value in [('name-count', 0, 64), ('encoding', 80, 1),
                                 ('format', 82, 0), ('checksum', 72, wrapper[72]^1)]:
        changed = bytearray(wrapper)
        changed[offset] = value
        malformed.append((label, bytes(changed)))
    for label, invalid in malformed:
        path = out/('invalid-'+label+'.dc42')
        path.write_bytes(invalid)
        try:
            media_helper.dc42_data(path)
        except ValueError:
            container_controls.append({'case': label, 'accepted': False, 'sha256': sha(path)})
        else:
            raise ValueError('invalid wrapper accepted: '+label)

    if pc:
        loader = module('mfs_media_loader', ROOT/'scripts/pc286_loader.py')
        flags = ['-zq', '-2', '-ms', '-zl', '-s', '-os', '-zastd=c99', '-we',
                 '-dCS_C16_REQUIRE_16BIT', '-i='+str(tool/'h')]
        objects = []
        for index, source in enumerate([*CORE, SHARED, ROOT/'platform/pc286/boothush-floppy-read.c',
                                        ROOT/'probes/pc286/boot-startup-files.c']):
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
        report = re.search(r'^\s*([0-9a-fA-F]+)[+*]?\s+_cs_startup_files_report\s*$', text, re.M)
        if not entry or not bss or not report or '.lib' in text.lower():
            raise ValueError('missing 286 entry/BSS/report or runtime library')
        payload = binary.read_bytes()
        bss_start, bss_size = (int(value, 16) for value in bss.groups())
        report_address = 0x10000+int(report.group(1), 16)
        if len(payload)+64 > 16384:
            raise ValueError('full payload plus report exceeds single-run inspection budget')
        image = bytearray(loader.floppy(payload, int(entry.group(1), 16), bss_end=bss_start+bss_size,
                                       poison_bss=True, verify_payload=True))
        image[60*512:860*512] = logical
        media = out/'pc-private-transfer.img'
        media.write_bytes(image)
        spans = [(0x10000+i, payload[i:i+4096]) for i in range(0, len(payload), 4096)]
        builds['pc286'] = {'payload_bytes': len(payload), 'payload_sha256': sha(binary),
                           'bss_start': bss_start, 'bss_bytes': bss_size,
                           'near_segment_end': 65536, 'stack_top': 65534,
                           'linked_bytes_below_stack': bss_start+bss_size,
                           'report_address': report_address, 'runtime_modules': []}

        def pc_command(marker, fault):
            cmd = [exe, '--machine', 'pc286', '--floppy', media, '--cycles', str(args.cycles),
                   '--timeout-ms', str(args.timeout_ms), '--until-post', hex(marker)]
            for address, chunk in spans:
                cmd += ['--inspect', hex(address)+':'+str(len(chunk))]
            cmd += ['--inspect', hex(report_address)+':64']
            if fault:
                cmd += ['--fail-floppy-read', ':'.join(str(v) for v in oracle['pc_fault'])]
            return cmd

        pc_media_sha = sha(media)
        for name, fault in ([('baseline', False), ('late-fault', True), ('fresh-recovery', False)]
                            if expected else [('baseline', False), ('repeat', False)]):
            marker = 0xb8
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
            words = list(struct.unpack('<16I', bytes.fromhex(memories[-1]['bytes'])))
            if fault:
                injection = result.get('floppy_read_failure', {})
                if [injection.get(k) for k in ('injected', 'cylinder', 'head', 'sector')] != [1, *oracle['pc_fault']]:
                    raise ValueError('missing exact injected 286 sector failure')
            validate(words, oracle, fault)
            if sha(media) != pc_media_sha:
                raise ValueError('286 transfer media changed')
            observations.append({'lane': 'pc286', 'case': name, 'report': words, 'result': result,
                                 'full_payload_matches': True, 'media_sha256': pc_media_sha})

    if mac:
        elf, raw, rom = out/'mac.elf', out/'mac.bin', out/'mac.rom'
        objects, frames = [], []
        reset = out/'startup-reset.S'
        reset_text = (ROOT/'arch/m68k/mac-reset.S').read_text()
        if reset_text.count('.long 0x4000') != 1:
            raise ValueError('reset stack anchor changed')
        reset.write_text(reset_text.replace('.long 0x4000', '.long 0x6000'))
        linker = out/'startup-rom.ld'
        link_text = (ROOT/'arch/m68k/mac-rom.ld').read_text()
        if link_text.count('LENGTH = 0x1500') != 1 or link_text.count('__bss_end <= 0x2000') != 1:
            raise ValueError('linker memory anchors changed')
        linker.write_text(link_text.replace('LENGTH = 0x1500', 'LENGTH = 0x3500').replace('__bss_end <= 0x2000', '__bss_end <= 0x4000'))
        mac_sources = [reset, *CORE, SHARED,
                       ROOT/'platform/mac128k/iwm-read.c', ROOT/'platform/mac128k/iwm-capture.S',
                       ROOT/'platform/mac128k/startup-floppy.c', ROOT/'probes/mac128k/startup-files.c']
        for index, source in enumerate(mac_sources):
            obj = out/('mac-'+str(index)+'.o')
            flags = ['-mcpu=68000', '-Os', '-ffreestanding', '-fno-builtin', '-nostdlib']
            if source.suffix == '.c':
                flags += ['-std=c99', '-Wall', '-Wextra', '-Werror', '-Wconversion',
                          '-Wsign-conversion', '-fstack-usage']
            run([m68k['gcc'], *flags, '-c', source, '-o', obj], 'mac-compile-'+str(index))
            objects.append(obj)
            if source.suffix == '.c':
                frames += obj.with_suffix('.su').read_text(encoding='utf-8').splitlines()
        run([m68k['gcc'], '-mcpu=68000', '-nostdlib', '-Wl,--build-id=none',
             '-Wl,-T,'+str(linker), *objects, '-o', elf], 'mac-link')
        if run([m68k['nm'], '-u', elf], 'mac-imports').strip():
            raise ValueError('unexpected Mac runtime dependency')
        symbols = run([m68k['nm'], '-n', elf], 'mac-symbols').decode('utf-8')

        def symbol(name):
            match = re.search(r'^([0-9a-fA-F]+) [BbDd] '+name+r'\s*$', symbols, re.M)
            if match is None:
                raise ValueError('missing Mac diagnostic symbol '+name)
            return int(match.group(1), 16)

        diag_address, report_address = symbol('cs_mac_diag'), symbol('cs_startup_files_report')
        data_start, bss_start, bss_end = symbol('__data_start'), symbol('__bss_start'), symbol('__bss_end')
        if (data_start != 0xb00 or not data_start <= bss_start <= bss_end <= 0x4000 or
                not data_start <= diag_address <= bss_end-28 or
                not data_start <= report_address <= bss_end-64):
            raise ValueError('Mac linked objects exceed the isolated probe memory budget')
        run([m68k['objcopy'], '-O', 'binary', elf, raw], 'mac-binary')
        payload = raw.read_bytes()
        if not 260 <= len(payload) <= 65536:
            raise ValueError('Mac probe outside 64 KiB ROM budget')
        if int.from_bytes(payload[:4], 'big') != 0x6000:
            raise ValueError('Mac reset stack does not match the explicit 0x4000..0x6000 budget')
        rom.write_bytes(payload+bytes([255])*(65536-len(payload)))
        rom_sha = sha(rom)
        builds['mac128k'] = {'rom_sha256': rom_sha, 'payload_bytes': len(payload),
                            'diag_address': diag_address, 'report_address': report_address,
                            'data_start': data_start, 'bss_start': bss_start, 'bss_end': bss_end,
                            'data_bss_bytes': bss_end-data_start, 'data_bss_limit': 0x4000,
                            'stack_reserved_bottom': 0x4000, 'stack_initial_top': 0x6000,
                            'framebuffer_start': 0x1a700, 'local_compiler_frames': frames,
                            'probe_reset_sha256': sha(reset), 'probe_linker_sha256': sha(linker),
                            'runtime_modules': []}
        for name, fault in ([('baseline', False), ('late-fault', True), ('fresh-recovery', False)]
                            if expected else [('baseline', False), ('repeat', False)]):
            marker = 0xb8
            cmd = [exe, '--machine', 'mac128k', '--rom', rom, '--floppy', raw_disk,
                   '--cycles', str(args.cycles), '--timeout-ms', str(args.timeout_ms),
                   '--until', hex(diag_address)+':4:'+hex(marker), '--inspect', hex(diag_address)+':28',
                   '--inspect', hex(report_address)+':64', '--mac-floppy-state', '1']
            if fault:
                cmd += ['--mac-floppy-bad-checksum', ':'.join(str(v) for v in oracle['mac_fault'])]
            result = json.loads(run(cmd, 'mac-'+name))
            memories = result.pop('memory_ranges', [])
            if (result.get('status') != 'condition_met' or len(memories) != 2 or
                    [m.get('address') for m in memories] != [diag_address, report_address]):
                raise ValueError('Mac '+name+': missing stop/diagnostics')
            diag = list(struct.unpack('>7I', bytes.fromhex(memories[0]['bytes'])))
            words = list(struct.unpack('>16I', bytes.fromhex(memories[1]['bytes'])))
            if diag[0] != marker or diag[6]:
                raise ValueError('Mac guest marker or fault differs')
            if fault:
                injection = result.get('mac_floppy_checksum_fault', {})
                if (injection.get('kind') != 'synthetic_gcr_data_checksum' or
                    injection.get('mode') != 'persistent' or
                    [injection.get(k) for k in ('track', 'sector')] != oracle['mac_fault'] or
                    injection.get('altered_nibbles', 0) < 1):
                    raise ValueError('missing exact Mac checksum injection')
            validate(words, oracle, fault)
            if sha(rom) != rom_sha or sha(raw_disk) != raw_sha:
                raise ValueError('Mac input changed')
            observations.append({'lane': 'mac128k', 'case': name, 'report': words, 'diag': diag,
                                 'result': result, 'rom_artifact_unchanged': True, 'media_sha256': raw_sha})

    for lane in ('pc286', 'mac128k'):
        rows = [row for row in observations if row['lane'] == lane and row['case'] != 'late-fault']
        if rows and (rows[0]['result'] != rows[1]['result'] or rows[0]['report'] != rows[1]['report']):
            raise ValueError(lane+': fresh repeat differs')
    successful = [row['report'] for row in observations if row['case'] == 'baseline']
    if any(row != successful[0] for row in successful):
        raise ValueError('native lane aggregate metadata differs')
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
              'commands': commands, 'expected_original': oracle, 'private_runtime_input': bool(selected),
              'selected_container_sha256': selected_sha, 'logical_media_sha256': raw_sha,
              'inputs_unchanged': True, 'hosted': hosted, 'mutants': mutants, 'container_controls': container_controls,
              'startup_sources_ready': all(row['report'][1] == 0 and row['report'][14] == 0xb8
                                                 for row in observations if row['case'] != 'late-fault'),
              'limits': 'Startup file selection and prepared resource sources through native modeled block providers. '
                        'No protected file names, type keys, resource IDs or payload bytes/digests in diagnostics. '
                        'Private logical-sector copies remain in the explicit external output directory. '
                        'No Resource Manager traps/loading, legacy execution, System startup, writes or physical hardware claim. '
                        'The admitted emulator cannot inspect Mac ROM addresses; ROM artifact identity is pinned, '
                        'while loaded-payload byte comparison is performed only for286. Local compiler frames are not cumulative stack proof. '
                        'Optional private media receives no fixed-sector injection or original-fixture count oracle.'}
    if not report['startup_sources_ready']:
        report['status'] = 'observed-refusal'
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'status': report['status'], 'cases': len(observations), 'report': str(out/'report.json')}))
    return 0


def validate(words, oracle, fault):
    if len(words) != 16 or words[0] != 0x53544631 or words[15] != 1:
        raise ValueError('missing original startup report signature/guards')
    if oracle:
        expected = oracle['fault_expected'] if fault else oracle['expected']
        if words != expected:
            raise ValueError('original startup metadata or exact failure differs: '+repr(words))
    elif words[1] == 0:
        if (words[2:8] != [6, 2, 0, 0, 0, 0] or words[8] != 2 or
                words[12:15] != [2, 0, 0xb8]):
            raise ValueError('runtime startup completion is inconsistent')
    elif (words[1] != 2 or words[2] not in (1, 2, 3, 4, 5) or
          words[3] != (2 if words[2] <= 2 else words[8]) or
          words[4] != (1 if words[2] == 1 else 3 if words[2] == 5 else 2) or
          words[5] == 0 or words[8] > 1 or words[12:15] != [0, 1, 0xd1]):
        raise ValueError('runtime startup refusal is inconsistent')


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, struct.error, subprocess.TimeoutExpired) as exc:
        print('Startup file verification failed: '+str(exc), file=sys.stderr)
        sys.exit(1)
