#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Inspect CODE metadata from prepared Shell sources on both native targets.

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
PINNED_EMULATOR = '3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4'
CORE = [ROOT/'core/storage'/name for name in ('block-read.c', 'mfs-read.c', 'mfs-volume.c', 'mac-boot-header.c', 'startup-files.c')]
CORE += [ROOT/'formats/resourcefork'/name for name in ('transport.c', 'mfs-transport.c')]
CORE += [ROOT/'personality/system1/code-stream.c']
SHARED = ROOT/'probes/storage/code-stream.c'


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
    parser.add_argument('--routine-base', choices=('literal','after-header'), default='literal')
    parser.add_argument('--load-segment', action='store_true', help='materialize the first resolved Shell segment')
    parser.add_argument('--fixture-result', type=int, default=37, help='original load-fixture MOVEQ operand, 0..127')
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
    if selected and selected_sha != '1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647':
        raise ValueError('private disk is not the designated System Disk')
    after_header = args.routine_base == 'after-header'
    if args.load_segment and (not after_header or not 0 <= args.fixture_result <= 127):
        parser.error('segment loads use after-header and fixture-result 0..127')
    load_api=[ROOT/'personality/system1'/name for name in ('code-stream-row.c','code-stream-load.c')]
    load_probe=ROOT/'probes/storage/code-stream-load.c'
    native_shared=load_probe if args.load_segment else SHARED
    native_core=CORE+load_api if args.load_segment else CORE
    guest_defines=['CS_EXECUTE_FIXTURE'] if args.load_segment and not selected else []
    pc_cpu=[ROOT/'core/cpu/m68k.c', ROOT/'core/services/traps.c'] if pc and guest_defines else []
    base_define = 'CS_PROBE_ROUTINE_BASE=CS_CSTREAM_AFTER_HEADER' if after_header else 'CS_PROBE_ROUTINE_BASE=CS_CSTREAM_RESOURCE_START'
    tools = {'emulator': exe, 'python': Path(sys.executable).resolve(), 'host_cc': args.cc.resolve(strict=True)}
    sources = [*CORE, *(p.with_suffix('.h') for p in CORE), SHARED, SHARED.with_suffix('.h'),
               ROOT/'docs/specifications/SPEC-0115-mfs-volume-reader.md',
               ROOT/'docs/specifications/SPEC-0112-mfs-metadata-reader.md',
               ROOT/'docs/specifications/SPEC-0116-streamed-resource-fork.md',
               ROOT/'docs/specifications/SPEC-0024-resource-fork-reader.md',
               ROOT/'docs/specifications/SPEC-0122-startup-files.md',
               ROOT/'docs/specifications/SPEC-0127-streamed-code-inspection.md',
               ROOT/'docs/specifications/SPEC-0032-code-only-launch-preflight.md',
               ROOT/'formats/resourcefork/reader.h', ROOT/'scripts/Test-MfsVolumeMedia.py',
               ROOT/'scripts/resource_media_fixture.py', ROOT/'scripts/code_stream_fixture.py',
               ROOT/'tests/code-stream.c', ROOT/'scripts/Test-MacBootHeader.py',
               ROOT/'docs/specifications/SPEC-0119-mac-boot-header.md',
               ROOT/'docs/specifications/SPEC-0111-read-only-logical-blocks.md',
               ROOT/'docs/specifications/SPEC-0113-boothush-pc286-floppy-provider.md',
               ROOT/'docs/specifications/SPEC-0114-boothush-mac-floppy-provider.md', Path(__file__).resolve()]
    native, native_identity, m68k, tool = None, None, {}, None
    if args.load_segment:
        sources += [*load_api, *(p.with_suffix('.h') for p in load_api),load_probe,
                    ROOT/'docs/specifications/SPEC-0134-streamed-segment-load.md',ROOT/'tests/code-stream-load.c',
                    *pc_cpu, *(p.with_suffix('.h') for p in pc_cpu)]
    if pc:
        tool = args.tool_root.resolve(strict=True)
        lock_path = ROOT/'scripts/pc286-toolchain-lock.json'
        sources += [lock_path, ROOT/'arch/i286/c16.h', ROOT/'scripts/pc286_loader.py',
                    ROOT/'platform/pc286/boothush-floppy-read.c',
                    ROOT/'platform/pc286/boothush-floppy-read.h', ROOT/'probes/pc286/boot-code-stream.c']
    if mac:
        sources += [ROOT/'scripts/m68k_windows_toolchain.py', ROOT/'arch/m68k/mac-reset.S',
                    ROOT/'arch/m68k/mac-rom.ld', ROOT/'platform/mac128k/iwm-read.c',
                    ROOT/'platform/mac128k/iwm-read.h', ROOT/'platform/mac128k/iwm-capture.S',
                    ROOT/'probes/mac128k/code-stream.c']
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
    fixture = module('startup_fixture', ROOT/'scripts/code_stream_fixture.py')
    original_answer=args.fixture_result if args.load_segment else None
    logical, oracle = (media_helper.dc42_data(selected), None) if selected else fixture.original_media(
        after_header=after_header, fixture_result=original_answer)
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
        if expect_failure and b'code stream check failed:' not in result.stderr:
            raise ValueError(label+' did not reach the unchanged test oracle')
        return result.stdout

    cc = args.cc.resolve(strict=True)
    host_disk = out/'original-host.raw'
    original_logical, _ = fixture.original_media()
    host_disk.write_bytes(original_logical)
    host_exact,host_over=out/'original-exact-capacity.raw',out/'original-over-capacity.raw'
    host_exact.write_bytes(fixture.original_media(8192,64)[0])
    host_over.write_bytes(fixture.original_media(65,65)[0])
    hosted, mutants, container_controls = [], [], []
    load_fixture = out/'original-load-host.raw'
    if args.load_segment:
        load_fixture.write_bytes(fixture.original_media(after_header=True,fixture_result=37)[0])
    warning = ['-Wall', '-Wextra', '-Werror', '-Wconversion', '-Wsign-conversion']
    for label, options in [('release', ['-O3', '-DNDEBUG']),
                           ('sanitized', ['-O1', '-fsanitize=address,undefined',
                                          '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'])]:
        binary = out/(label+'.exe')
        run([cc, '-std=c11', *warning, *options, *CORE, SHARED,
             ROOT/'tests/code-stream.c', '-o', binary], label+'-compile')
        message = run([binary, host_disk, host_exact, host_over], label+'-test').decode().strip()
        if not re.fullmatch(r'Code stream: [0-9]+ checks passed', message):
            raise ValueError('host test did not complete')
        hosted.append({'profile': label, 'result': message, 'binary_sha256': sha(binary)})
        if args.load_segment:
            load_binary=out/(label+'-load.exe')
            run([cc,'-std=c11',*warning,*options,*CORE,*load_api,ROOT/'tests/code-stream-load.c',
                 '-o',load_binary],label+'-load-compile')
            checked=run([load_binary,load_fixture],label+'-load-test').decode().strip()
            if not re.fullmatch(r'Code stream load: [0-9]+ checks passed',checked):
                raise ValueError('host load tests did not complete')
            hosted.append({'profile':label+'-load','result':checked,'binary_sha256':sha(load_binary)})
    coordinator = ROOT/'personality/system1/code-stream.c'
    original_code = coordinator.read_text()
    mutations = [
        ('early-publication', 'result=find(w,segment->resource.entry.id,&segment->resource,r);',
         'copy(out,m,sizeof *out);result=find(w,segment->resource.entry.id,&segment->resource,r);'),
        ('wrong-routine-base', 'uint32_t routine=be16(w->bytes+(size_t)(j<<3));',
         'uint32_t routine=be16(w->bytes+(size_t)(j<<3))+4u;'),
        ('skip-group', 'if(segment->first_entry!=w->first[s] || count!=w->seen[s] ||\n            w->last[s]-w->first[s]+1u!=count)return fail(r,CS_CSTREAM_SEGMENT_HEADER);',
         'if(0)return fail(r,CS_CSTREAM_SEGMENT_HEADER);'),
    ]
    for label, anchor, replacement in mutations:
        if original_code.count(anchor) != 1:
            raise ValueError('mutation anchor changed: '+label)
        source, binary = out/(label+'.c'), out/(label+'.exe')
        source.write_text(original_code.replace(anchor, replacement))
        run([cc, '-std=c11', *warning, '-O2', '-I'+str(coordinator.parent),
             *(source if p == coordinator else p for p in CORE), SHARED,
             ROOT/'tests/code-stream.c', '-o', binary], label+'-compile')
        run([binary, host_disk, host_exact, host_over], label+'-test', expect_failure=True)
        mutants.append({'case': label, 'source_sha256': sha(source), 'rejected': True})
    wrapper_helper = module('startup_dc42_writer', ROOT/'scripts/Test-MacBootHeader.py')
    wrapper = wrapper_helper.original_dc42(original_logical)
    container = out/'original-code-stream.dc42'
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
                 '-dCS_C16_REQUIRE_16BIT','-d'+base_define, '-i='+str(tool/'h')]
        objects = []
        flags.extend('-d'+name for name in guest_defines)
        for index, source in enumerate([*native_core,*pc_cpu,native_shared, ROOT/'platform/pc286/boothush-floppy-read.c',
                                        ROOT/'probes/pc286/boot-code-stream.c']):
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
        report = re.search(r'^\s*([0-9a-fA-F]+)[+*]?\s+_cs_code_stream_report\s*$', text, re.M)
        if not entry or not bss or not report or '.lib' in text.lower():
            raise ValueError('missing 286 entry/BSS/report or runtime library')
        payload = binary.read_bytes()
        bss_start, bss_size = (int(value, 16) for value in bss.groups())
        report_address = 0x10000+int(report.group(1), 16)
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

        def pc_command(marker, fault, pieces):
            cmd = [exe, '--machine', 'pc286', '--floppy', media, '--cycles', str(args.cycles),
                   '--timeout-ms', str(args.timeout_ms), '--until-post', hex(marker)]
            for address, chunk in pieces:
                cmd += ['--inspect', hex(address)+':'+str(len(chunk))]
            cmd += ['--inspect', hex(report_address)+':64']
            if fault:
                cmd += ['--fail-floppy-read', ':'.join(str(v) for v in oracle['pc_fault'])]
            return cmd

        pc_media_sha = sha(media)
        for name, fault in ([('baseline', False), ('late-fault', True), ('fresh-recovery', False)]
                            if expected else [('baseline', False), ('repeat', False)]):
            marker = 0xb8
            windows = [spans] if len(payload)+64 <= 16384 else [spans[i:i+3] for i in range(0,len(spans),3)]
            words = None
            for pass_number, pieces in enumerate(windows):
                result = json.loads(run(pc_command(marker, fault, pieces), 'pc-'+name+'-'+str(pass_number)))
                memories = result.pop('memory_ranges', [])
                if (result.get('status') != 'condition_met' or result.get('post') != marker or
                        result.get('cs') != 0x1000 or len(memories) != len(pieces)+1):
                    raise ValueError('286 '+name+': missing stop or payload inspection')
                for memory, (address, chunk) in zip(memories[:-1], pieces):
                    if memory.get('address') != address or bytes.fromhex(memory.get('bytes', '')) != chunk:
                        raise ValueError('286 loaded payload differs')
                if memories[-1].get('address') != report_address:
                    raise ValueError('286 report address differs')
                observed = list(struct.unpack('<16I', bytes.fromhex(memories[-1]['bytes'])))
                if words is not None and words != observed:
                    raise ValueError('286 stop report differs between payload windows')
                words = observed
            if fault:
                injection = result.get('floppy_read_failure', {})
                if [injection.get(k) for k in ('injected', 'cylinder', 'head', 'sector')] != [1, *oracle['pc_fault']]:
                    raise ValueError('missing exact injected 286 sector failure')
            if args.load_segment:
                validate_load(words, oracle, fault, original_answer)
            else:
                validate(words, oracle, fault)
            if sha(media) != pc_media_sha:
                raise ValueError('286 transfer media changed')
            observations.append({'lane': 'pc286', 'case': name, 'report': words, 'result': result,
                                 'full_payload_matches': True, 'media_sha256': pc_media_sha})

    if mac:
        elf, raw, rom = out/'mac.elf', out/'mac.bin', out/'mac.rom'
        objects, frames = [], []
        reset = out/'code-stream-reset.S'
        reset_text = (ROOT/'arch/m68k/mac-reset.S').read_text()
        if reset_text.count('.long 0x4000') != 1:
            raise ValueError('reset stack anchor changed')
        stack_top = 0xc000 if args.load_segment else 0x8000
        bss_limit = stack_top - 0x2000
        reset.write_text(reset_text.replace('.long 0x4000', '.long '+hex(stack_top)))
        linker = out/'code-stream-rom.ld'
        link_text = (ROOT/'arch/m68k/mac-rom.ld').read_text()
        if link_text.count('LENGTH = 0x1500') != 1 or link_text.count('__bss_end <= 0x2000') != 1:
            raise ValueError('linker memory anchors changed')
        linker.write_text(link_text.replace('LENGTH = 0x1500', 'LENGTH = '+hex(bss_limit-0xb00)).replace('__bss_end <= 0x2000', '__bss_end <= '+hex(bss_limit)))
        mac_sources = [reset, *native_core, native_shared,
                       ROOT/'platform/mac128k/iwm-read.c', ROOT/'platform/mac128k/iwm-capture.S',
                       ROOT/'probes/mac128k/code-stream.c']
        for index, source in enumerate(mac_sources):
            obj = out/('mac-'+str(index)+'.o')
            flags = ['-mcpu=68000', '-Os', '-ffreestanding', '-fno-builtin', '-nostdlib']
            if source.suffix == '.c':
                flags.extend('-D'+name for name in guest_defines)
                flags += ['-D'+base_define,'-std=c99', '-Wall', '-Wextra', '-Werror', '-Wconversion',
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

        diag_address, report_address = symbol('cs_mac_diag'), symbol('cs_code_stream_report')
        data_start, bss_start, bss_end = symbol('__data_start'), symbol('__bss_start'), symbol('__bss_end')
        if (data_start != 0xb00 or not data_start <= bss_start <= bss_end <= bss_limit or
                not data_start <= diag_address <= bss_end-28 or
                not data_start <= report_address <= bss_end-64):
            raise ValueError('Mac linked objects exceed the isolated probe memory budget')
        run([m68k['objcopy'], '-O', 'binary', elf, raw], 'mac-binary')
        payload = raw.read_bytes()
        if not 260 <= len(payload) <= 65536:
            raise ValueError('Mac probe outside 64 KiB ROM budget')
        if int.from_bytes(payload[:4], 'big') != stack_top:
            raise ValueError('Mac reset stack does not match probe budget')
        rom.write_bytes(payload+bytes([255])*(65536-len(payload)))
        rom_sha = sha(rom)
        builds['mac128k'] = {'rom_sha256': rom_sha, 'payload_bytes': len(payload),
                            'diag_address': diag_address, 'report_address': report_address,
                            'data_start': data_start, 'bss_start': bss_start, 'bss_end': bss_end,
                            'data_bss_bytes': bss_end-data_start, 'data_bss_limit': bss_limit,
                            'stack_reserved_bottom': bss_limit, 'stack_initial_top': stack_top,
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
            if args.load_segment:
                validate_load(words, oracle, fault, original_answer)
            else:
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
              'routine_base': args.routine_base, 'selected_container_sha256': selected_sha, 'logical_media_sha256': raw_sha,
              'materialize_segment': args.load_segment, 'fixture_execution_enabled': bool(guest_defines),
              'inputs_unchanged': True, 'hosted': hosted, 'mutants': mutants, 'container_controls': container_controls,
              'code_metadata_ready': all(row['report'][2] == 0 and row['report'][14] == 0xb8
                                                 for row in observations if row['case'] != 'late-fault'),
              'limits': 'Streamed CODE metadata from prepared Shell sources through native modeled block providers. '
                        'No protected file names, type keys, resource IDs or payload bytes/digests in diagnostics. '
                        'Private logical-sector copies remain in the explicit external output directory. '
                        'No Resource Manager traps/loading, legacy execution, System startup, writes or physical hardware claim. '
                        'The admitted emulator cannot inspect Mac ROM addresses; ROM artifact identity is pinned, '
                        'while loaded-payload byte comparison is performed only for286. Local compiler frames are not cumulative stack proof. '
                        'Optional private media receives no fixed-sector injection or original-fixture count oracle.'}
    if not report['code_metadata_ready']:
        report['status'] = 'observed-refusal'
    if args.load_segment:
        report['limits'] = ('One resolved Shell segment in isolated scratch memory. Original fixtures alone '
                            'contain an invoked MOVEQ/RTS routine. Private builds only copy the body. '
                            'No Finder launch, patched A5 jump table, installed runtime, or hardware claim.')
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'status': report['status'], 'cases': len(observations), 'report': str(out/'report.json')}))
    return 0


def validate(words, oracle, fault):
    if len(words) != 16 or words[0] != 0x43535431 or words[15] != 1:
        raise ValueError('missing original CODE report signature/guards')
    if oracle:
        expected = oracle['fault_expected'] if fault else oracle['expected']
        if words != expected:
            raise ValueError('original CODE metadata or exact failure differs: '+repr(words))
    elif words[14] == 0xb8:
        if (words[1:5] != [2,0,0,0] or not 1 <= words[6] <= 64 or
                not words[6] <= words[5] <= 8192 or words[12:14] != [1,0]):
            raise ValueError('runtime CODE completion is inconsistent')
    elif (words[14] not in (0xd1,0xd2) or words[1] != words[14]-0xd0 or
          words[2] == 0 or words[12:14] != [0,1]):
        raise ValueError('runtime CODE refusal is inconsistent')
    elif words[1:3] == [2,12] and (words[3] != 7 or words[7] not in (1,2,3,4) or words[8] >= words[5]):
        raise ValueError('runtime routine-range classification is inconsistent')


def validate_load(words, oracle, fault, answer):
    if len(words) != 16 or words[0] != 0x43534c31 or any(words[7:14]+words[15:]):
        raise ValueError('bad segment-load report')
    if fault:
        if words[1:7] != [2,4,0,0,0,0] or words[14] != 0xd2:
            raise ValueError('wrong segment-load injected failure')
    elif oracle:
        if words[1:7] != [4,0,620,2,1,answer] or words[14] != 0xb8:
            raise ValueError('original routine failed: '+repr(words))
    elif words[1:3] != [4,0] or words[14] != 0xb8 or words[5:7] != [0,0] or not (
            2 <= words[3] <= 16384 and words[4] % 2 == 0 and words[4] <= words[3]-2):
        raise ValueError('private copy-only load refused or inconsistent: '+repr(words))


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, struct.error, subprocess.TimeoutExpired) as exc:
        print('Code stream verification failed: '+str(exc), file=sys.stderr)
        sys.exit(1)
